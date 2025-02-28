#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
//#include <linux/if.h>
//#include <linux/if_packet.h>
//#include <linux/if_ether.h>
#include "common.h"
#include "security_engine.h"
#include "list.h"
#include "debug.h"
#include "crypto/aes.h"
#include "crypto/aes_siv.h"
#include "crypto/sha256.h"
#include "byteorder.h"
#include "p1905_managerd.h"

#define MIN_TLVS_LENGTH    38   /*46 - 8(cmdu header size) */
#define CMDU_HLEN          8
#define END_OF_TLV_TYPE 	0

struct dl_list sec_hash_entry[HASH_TABLE_SIZE];
struct dl_list gik_pool_list;
unsigned char _1905_multicast_address[6]
                    = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x13 };
unsigned char zero_counter[COUNTER_LENGTH]
					= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char security_config = 0x00;

unsigned char *encr_text;
unsigned int encr_text_len;

int sec_debug_level = DEBUG_ERROR;

#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]

char *code2string(unsigned char code)
{
	switch (code) {
		case NO_KEY_FOUND:
			return "NO_KEY_FOUND";
		case INVALID_CMDU:
			return "INVALID_CMDU";
		case HAMC_FAIL:
			return "HAMC_FAIL";
		case HMAC_NOT_EQUAL:
			return "HMAC_NOT_EQUAL";
		case GIK_NOT_FOUND:
			return "GIK_NOT_FOUND";
		case UNKNOWN_KEY_TYPE:
			return "UNKNOWN_KEY_TYPE";
		case AES_SIV_ENC_FAIL:
			return "AES_SIV_ENC_FAIL";
		case AES_SIV_DECRYPT_FAIL:
			return "AES_SIV_DECRYPT_FAIL";
		case NOT_ENCRYPTED:
			return "NOT_ENCRYPTED";
		case INVALID_RX_COUNTER:
			return "INVALID_RX_COUNTER";
		case HMAC_NOT_EQUAL_RETRY:
			return "HMAC_NOT_EQUAL_RETRY";
	}

	return "UNKNOWN_ERROR";
}

char *key_type2string(enum KEY_TYPE type)
{
	switch (type) {
		case KEY_UNKNOWN:
			return "UNKNOWN";
		case KEY_MULTICAST:
			return "MULTICAST";
		case KEY_UNICAST:
			return "UNICAST";
	}

	return "UNKNOWN_ERROR";
}


void counter_increase(unsigned char *counter)
{
	int i = COUNTER_LENGTH - 1;

	while(++(counter[i--]) == 0x00) {
		if (i < 0)
			break;
	}
}

/*
void counter_increase(unsigned char *counter)
{
	int i = 0;

	while(++(counter[i++]) == 0x00) {
		if (i == COUNTER_LENGTH)
			break;
	}
}
*/

struct gik_pool_entry *gik_pool_entry_alloc(unsigned char *key, size_t key_len, unsigned char key_identifier)
{
	struct gik_pool_entry *entry = NULL;

	entry = (struct gik_pool_entry *)os_malloc(sizeof(struct gik_pool_entry));
	if (entry == NULL)
		return NULL;

	if (key_len > WPA_TK_MAX_LEN) {
		os_free(entry);
		return NULL;
	}
	os_memcpy(entry->key, key, key_len);
	entry->key_len = key_len;
	entry->key_identifier = key_identifier;

	return entry;
}

int gik_pool_entry_insert(struct gik_pool_entry *entry)
{
	dl_list_add_tail(&gik_pool_list, &entry->entry);
	return 0;
}

struct gik_pool_entry *gik_pool_entry_lookup(unsigned char key_identifier)
{
	struct gik_pool_entry *entry = NULL;

	dl_list_for_each(entry, &gik_pool_list, struct gik_pool_entry, entry) {
		if (entry->key_identifier == key_identifier)
			return entry;
	}

	return NULL;
}

int gik_pool_entry_delete(unsigned char key_identifier)
{
	struct gik_pool_entry *entry = NULL, *entry_next = NULL;

	dl_list_for_each_safe(entry, entry_next, &gik_pool_list, struct gik_pool_entry, entry) {
		if (key_identifier == entry->key_identifier) {
			dl_list_del(&entry->entry);
			os_free(entry);
			break;
		}
	}

	return 0;
}

int gik_pool_entry_clear()
{
	struct gik_pool_entry *entry = NULL, *entry_next = NULL;

	dl_list_for_each_safe(entry, entry_next, &gik_pool_list, struct gik_pool_entry, entry) {
		dl_list_del(&entry->entry);
		os_free(entry);
	}

	return 0;
}

struct gik_peer_entry *gik_peer_entry_alloc(unsigned char *mac)
{
	struct gik_peer_entry *entry = NULL;

	entry = (struct gik_peer_entry *)os_malloc(sizeof(struct gik_peer_entry));
	if (entry == NULL)
		return NULL;

	os_memset(entry, 0, sizeof(struct gik_peer_entry));
	os_memcpy(entry->peer_mac, mac, ETH_ALEN);

	return entry;
}

struct gik_peer_entry *gik_peer_entry_lookup(struct security_entry *entry, unsigned char *mac)
{
	unsigned int hash_idx = 0;
	struct gik_peer_entry *peer_entry = NULL;

	hash_idx = MAC_ADDR_HASH_INDEX(mac);

	dl_list_for_each(peer_entry, &entry->gik_peer_entry[hash_idx], struct gik_peer_entry, entry) {
		if (!os_memcmp(peer_entry->peer_mac, mac, ETH_ALEN)) {
			return peer_entry;
		}
	}

	return NULL;
}

int gik_peer_entry_insert(struct security_entry *entry, struct gik_peer_entry *peer_entry)
{
	unsigned int hash_idx = 0;

	hash_idx = MAC_ADDR_HASH_INDEX(peer_entry->peer_mac);

	dl_list_add_tail(&entry->gik_peer_entry[hash_idx], &peer_entry->entry);
	return 0;
}

int gik_peer_entry_clear(struct security_entry *entry)
{
	int i = 0;
	struct gik_peer_entry *peer_entry = NULL, *peer_entry_next = NULL;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_for_each_safe(peer_entry, peer_entry_next,
			&entry->gik_peer_entry[i], struct gik_peer_entry, entry) {
			dl_list_del(&peer_entry->entry);
			os_free(peer_entry);
		}
	}
	return 0;
}

int gik_peer_entry_reset_rx_counter(struct security_entry *entry)
{
	int i = 0;
	struct gik_peer_entry *peer_entry = NULL, *peer_entry_next = NULL;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_for_each_safe(peer_entry, peer_entry_next,
			&entry->gik_peer_entry[i], struct gik_peer_entry, entry) {
			os_memset(peer_entry->multi_rx_counter, 0, COUNTER_LENGTH);
		}
	}
	return 0;
}

struct security_entry *sec_entry_alloc(unsigned char *almac)
{
	int i = 0;
	struct security_entry *entry = NULL;

	entry = (struct security_entry*)os_malloc(sizeof(struct security_entry));

	if (entry == NULL)
		return NULL;

	os_memset(entry, 0, sizeof(struct security_entry));
	os_memcpy(entry->almac, almac, ETH_ALEN);

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_init(&entry->gik_peer_entry[i]);
	}
	/*default define invalid key identifier to 0xff*/
	entry->key_identifier = 0xff;
	return entry;
}

struct security_entry *sec_entry_lookup(unsigned char *almac)
{
	unsigned int hash_idx = 0;
	struct security_entry *entry = NULL;

	hash_idx = MAC_ADDR_HASH_INDEX(almac);

	dl_list_for_each(entry, &sec_hash_entry[hash_idx], struct security_entry, entry) {
		if (!os_memcmp(entry->almac, almac, ETH_ALEN)) {
			return entry;
		}
	}

	return NULL;
}

int sec_entry_insert(struct security_entry *entry)
{
	unsigned int hash_idx = 0;

	hash_idx = MAC_ADDR_HASH_INDEX(entry->almac);

	dl_list_add_tail(&sec_hash_entry[hash_idx], &entry->entry);
	return 0;
}

int sec_entry_clear()
{
	int i = 0;
	struct security_entry *entry = NULL, *entry_next = NULL;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_for_each_safe(entry, entry_next, &sec_hash_entry[i], struct security_entry, entry) {
			gik_peer_entry_clear(entry);
			dl_list_del(&entry->entry);
			os_free(entry);
		}
	}

	return 0;
}

int sec_entry_clear_by_mac(unsigned char *almac)
{
	int i = 0;
	struct security_entry *entry = NULL, *entry_next = NULL;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_for_each_safe(entry, entry_next, &sec_hash_entry[i], struct security_entry, entry) {
			if (!os_memcmp(almac, entry->almac, ETH_ALEN)) {
				debug(DEBUG_ERROR, "clear sec info for "MACSTR"\n", MAC2STR(almac));
				dl_list_del(&entry->entry);
				gik_peer_entry_clear(entry);
				os_free(entry);
			}
		}
	}

	return 0;
}

int sec_entry_get_by_mac(unsigned char *almac)
{
	int i = 0;
	struct security_entry *entry = NULL, *entry_next = NULL;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_for_each_safe(entry, entry_next, &sec_hash_entry[i], struct security_entry, entry) {
			if (!os_memcmp(almac, entry->almac, ETH_ALEN)) {
				debug(DEBUG_ERROR, "got PTK for "MACSTR"\n", MAC2STR(almac));
				return 0;
			}
		}
	}

	return -1;
}

void sec_update_buf(unsigned char *buf, unsigned int len)
{
	encr_text = buf;
	encr_text_len = len;
}


int security_engine_init(unsigned char *buf, unsigned int len)
{
	int i = 0;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		dl_list_init(&sec_hash_entry[i]);
	}
	dl_list_init(&gik_pool_list);
    security_config = 0x03;

	sec_update_buf(buf, len);

	return 1;
}

int security_engine_deinit()
{
	sec_entry_clear();
	gik_pool_entry_clear();
	encr_text = NULL;

	return 1;
}

int sec_set_key(unsigned char *almac, unsigned char *key, size_t key_len,
	unsigned char key_identifier)
{
	struct security_entry *entry = NULL;
	struct gik_pool_entry *gik_entry = NULL;
	entry = sec_entry_lookup(almac);

	if (entry == NULL) {
		entry = sec_entry_alloc(almac);
		if (entry == NULL) {
			debug(DEBUG_ERROR, "allocate security entry fail\n");
			return -1;
		}
		sec_entry_insert(entry);
	}

	if (key_len > WPA_TK_MAX_LEN) {
		debug(DEBUG_ERROR, "invalide key length %d\n", (int)key_len);
		return -1;
	}

	if (!os_memcmp(_1905_multicast_address, almac, ETH_ALEN)) {
		entry->key_type = KEY_MULTICAST;
		/* set tx counter as 1 and rx counter as 0 whenever setting group key */
		gik_peer_entry_reset_rx_counter(entry);
		os_memset(entry->encry_tx_counter, 0, COUNTER_LENGTH);
		counter_increase(entry->encry_tx_counter);
		entry->decry_fail_counter = 0;

		if (key_identifier == entry->key_identifier) {
			if (!os_memcmp(entry->key, key, key_len)) {
				debug(DEBUG_ERROR, "same key already installed\n");
				return 0;
			} else {
				debug(DEBUG_ERROR, "same key id, diff key content, store this new key(len=%d)\n",
					(int)key_len);
			}
		}
		/*if no gik exist, use this key as current gik*/
		if (entry->key_len == 0) {
			entry->key_identifier = key_identifier;
			goto install_key;
		} else {
		/*else store this key in gik pool*/
			gik_entry = gik_pool_entry_lookup(key_identifier);
			if (gik_entry == NULL) {
				gik_entry = gik_pool_entry_alloc(key, key_len, key_identifier);
				if (gik_entry == NULL) {
					debug(DEBUG_ERROR, "allocate gik entry fail\n");
					return -1;
				}
				gik_pool_entry_insert(gik_entry);
			} else {
				debug(DEBUG_ERROR, "gik entry already exist in gik pool\n");
				return -1;
			}
			debug(DEBUG_ERROR, "store new key(id=%02x len=%d)\n", key_identifier, (int)key_len);
			return 0;
		}
	} else {
		entry->key_type = KEY_UNICAST;

        /*  set
            the Encryption Transmission Counter to one
            the Encryption Reception Counter to zero
        */
		os_memset(entry->encry_tx_counter, 0, COUNTER_LENGTH);
		counter_increase(entry->encry_tx_counter);
		os_memset(entry->decry_rx_counter, 0, COUNTER_LENGTH);
	}

install_key:
	os_memcpy(entry->key, key, key_len);
	entry->key_len = key_len;
	entry->decry_fail_counter = 0;
	return 0;
}

int add_mic_tlv(unsigned char *clear_text, unsigned short *len,  unsigned char* hmac,
	unsigned char hmac_length, unsigned char key_id, unsigned char* tx_counter,
	unsigned char fragment, unsigned char *src_mac)
{
	unsigned char *p = NULL;
	unsigned short length = 0;

	p = fragment == 1 ? (clear_text + *len) : (clear_text + *len - 3);

    /* MIC TLV: 0xAB */
	*p++ = MIC_TLV_TYPE;
	*((unsigned short*)p) = 15 + hmac_length;

	*((unsigned short*)p) = host_to_be16(*((unsigned short*)p));
	p += sizeof(unsigned short);

    /* bits 7-6: 1905 GTK Key Id
    ** bits 5-4: MIC Version
    **  0x00: Version 1
    **  0x00 - x03: Reserved
    ** bits 3-0: Reserved
    */
	*p = 0;
	*p = (key_id << 6) & 0xC0;
    p++;

	/* Integrity Transmission Counter */
	os_memcpy(p, tx_counter, COUNTER_LENGTH);
	p += COUNTER_LENGTH;

    /* Source AL MAC */
	os_memcpy(p, src_mac, ETH_ALEN);
	p += ETH_ALEN;

	*((unsigned short*)p) = host_to_be16(hmac_length);
	p += 2;

	os_memcpy(p, hmac, hmac_length);
	p += hmac_length;

	/*for the last fragment or not fragment frame, copy End of Message tlv to the end of the message*/
	if (fragment == 0) {
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = 0x00;

		/*to check whether it is need padding octets*/
		length = (unsigned short)(p - clear_text) - ETH_HLEN - CMDU_HLEN;

		if (length  < MIN_TLVS_LENGTH) {
			os_memset(p, 0, (MIN_TLVS_LENGTH - length));
			p += (MIN_TLVS_LENGTH - length);
		}
	}

	*len = (unsigned short)(p - clear_text);

	return 0;
}

int add_encrypted_tlv(unsigned char *tlv, unsigned short *tlv_len,  unsigned char* cipher_text,
	unsigned char *tx_counter, unsigned char fragment, unsigned char *src_mac, unsigned char *dst_mac)
{
	unsigned char *p = NULL, *p_old = NULL;
	unsigned short cipher_len = *tlv_len + AES_BLOCK_SIZE;
	unsigned short len = 0;

	if (fragment == 0)
		cipher_len -= 3;

	p = tlv;

	p_old = p;
    /* Encrypted TLV: 0xAC */
	*p++ = ENCRYPTED_TLV_TYPE;

	*((unsigned short*)p) = 20 + cipher_len;

	*((unsigned short*)p) = host_to_be16(*((unsigned short*)p));
	p += sizeof(unsigned short);

    /* Encryption Transmission Counter */
	os_memcpy(p, tx_counter, COUNTER_LENGTH);
	p += COUNTER_LENGTH;

    /* Source 1905 AL MAC Address */
	os_memcpy(p, src_mac, ETH_ALEN);
	p += ETH_ALEN;

    /* Destination 1905 AL MAC Address */
	os_memcpy(p, dst_mac, ETH_ALEN);
	p += ETH_ALEN;

    /* Length of the AES-SIV Encryption Output field */
	*((unsigned short*)p) = host_to_be16(cipher_len);
	p += sizeof(unsigned short);

    /* AES-SIV Encryption Output field
    (SIV concatenated with all the encrypted TLVs)  */
	os_memcpy(p, cipher_text, cipher_len);
	p += cipher_len;

	/*for the last fragment or not fragment frame, copy End of Message tlv to the end of the message*/
	if(fragment == 0) {
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = 0x00;

		/*to check whether it is need padding octets*/
		len = (unsigned short)(p - tlv);

		if (len  < MIN_TLVS_LENGTH) {
			os_memset(p, 0, (MIN_TLVS_LENGTH - len));
			p += (MIN_TLVS_LENGTH - len);
		}
	}

	*tlv_len = (unsigned short)(p - p_old);
	return 0;
}

unsigned char *get_tlv_pos(unsigned char *tlvs, unsigned char tlv_type, unsigned short tlv_len)
{
	unsigned char *tlv = tlvs;
	unsigned short len = 0;

	while(1) {
		if (*tlv == tlv_type)
			return tlv;
		if (*tlv == END_OF_TLV_TYPE)
			return NULL;
		len = *((unsigned short*)(tlv + 1));
		len = be_to_host16(len);
		tlv += len + 3;

		if((tlv - tlvs) >= tlv_len)
			return NULL;
	}
	return NULL;
}

int calculate_mic(IN unsigned char *key, IN size_t key_len, IN unsigned char *counter,
	IN unsigned char gik_identifier, IN unsigned char *payload, IN size_t payload_len,
	IN unsigned char *src_mac, OUT unsigned char *hash)
{
	unsigned char  *p = encr_text, *p_old = encr_text;

    /*
    "	The first 6 octets of the 1905 CMDU
    "	The first 13 octets of
            the MIC TLV Value (
                1905 GTK Key ID field
                MIC Version field
                reserved bits
            Integrity Transmission Counter field
            Source 1905 AL Mac Address field)
    "	All the TLVs included in the message (including the End of Message TLV) but not the MIC TLV.
    */

    os_memcpy(p, payload, 6);
    p += 6;

    /* bits 7-6: 1905 GTK Key Id
    ** bits 5-4: MIC Version
    **  0x00: Version 1
    **  0x00 - x03: Reserved
    ** bits 3-0: Reserved
    */
    *p = 0;
    *p = (gik_identifier << 6) & 0xC0;
    p++;

	os_memcpy(p, counter, COUNTER_LENGTH);
	p += COUNTER_LENGTH;

	os_memcpy(p, src_mac, ETH_ALEN);
	p += ETH_ALEN;

	os_memcpy(p, payload + CMDU_HLEN, payload_len - CMDU_HLEN);
	p += payload_len - CMDU_HLEN;

//		hex_dump_all("de key", key, key_len);
//		hex_dump_all("de clear_text", p_old, p - p_old);

	if (hmac_sha256(key, key_len, p_old, p - p_old, hash)) {
		return HAMC_FAIL;
	}

	return SUC;
}


int ptk_peer_reset_encrypt_txrx_counter(IN unsigned char *dst_mac)
{
	struct security_entry *entry = NULL;
	struct gik_peer_entry *peer_entry = NULL;
	if(!is_security_on(dst_mac))
		return SUC;

	entry = sec_entry_lookup(dst_mac);
	if (entry == NULL) {
		debug(DEBUG_ERROR, "ptk not found for "MACSTR"\n", MAC2STR(dst_mac));
		return NO_KEY_FOUND;
	}

	os_memset(entry->encry_tx_counter, 0, sizeof(entry->encry_tx_counter));
	os_memset(entry->decry_rx_counter, 0, sizeof(entry->decry_rx_counter));
	peer_entry = gik_peer_entry_lookup(entry, dst_mac);
	if (peer_entry) {
		os_memset(peer_entry->multi_rx_counter, 0, sizeof(peer_entry->multi_rx_counter));
		debug(DEBUG_ERROR, "reset gtk rx counter for "MACSTR"\n", MAC2STR(dst_mac));
	}
	counter_increase(entry->encry_tx_counter);
	debug(DEBUG_ERROR, "reset tx counter:"MACSTR", rx counter "MACSTR"\n",
		MAC2STR(entry->encry_tx_counter), MAC2STR(entry->decry_rx_counter));

	return SUC;
}

int sec_encrypt(IN unsigned char *dst_mac, IN unsigned char *src_mac,
	INOUT unsigned char *msg, INOUT unsigned short *len, IN unsigned char fragment)
{
	struct security_entry *entry = NULL;
	unsigned char *tlv = NULL, *cmdu = NULL, *p = encr_text, *p_old = NULL, *tmp = NULL;;
	size_t length = 0;
	unsigned short _1905_len = 0, tlv_len = 0, padding_len = 0;

	if(!is_security_on(dst_mac))
		return SUC;

	entry = sec_entry_lookup(dst_mac);
	if (entry == NULL)
		return SUC;

	cmdu = msg + ETH_HLEN;
	tlv = cmdu + CMDU_HLEN;
	os_memset(p, 0, encr_text_len);

	/*for not fragment cmdu, it may have some padding octets, now assumes that the padding octets will
	not be encrypted*/
	if (fragment == 0) {
		tmp = get_tlv_pos(tlv, END_OF_TLV_TYPE, *len - ETH_HLEN - CMDU_HLEN);
		if (tmp == NULL)
			return INVALID_CMDU;
		padding_len = *len - (tmp + 3 -  msg);
	}

	_1905_len = *len - ETH_HLEN - padding_len;
	tlv_len = _1905_len - CMDU_HLEN;
	if (tlv_len > encr_text_len)
		return INVALID_CMDU;
	p_old = p;

	if (entry->key_type == KEY_MULTICAST) {
        /*
        "	The first 6 octets of the 1905 CMDU
        "	The first 13 octets of
                the MIC TLV Value (
                    1905 GTK Key ID field
                    MIC Version field
                    reserved bits
                Integrity Transmission Counter field
                Source 1905 AL Mac Address field)
        "	All the TLVs included in the message (including the End of Message TLV) but not the MIC TLV.
        */

		unsigned char hash[SHA256_MAC_LEN];
		os_memcpy(p, cmdu, 6);
        p += 6;

        /* bits 7-6: 1905 GTK Key Id
        ** bits 5-4: MIC Version
        **  0x00: Version 1
        **  0x00 - x03: Reserved
        ** bits 3-0: Reserved
        */
        *p = 0;
        *p = (entry->key_identifier << 6) & 0xC0;
        p++;

		/* Integrity Transmission Counter */
		os_memcpy(p, entry->encry_tx_counter, COUNTER_LENGTH);
		p += COUNTER_LENGTH;

        /* Source 1905 AL Mac Address */
		os_memcpy(p, src_mac, ETH_ALEN);
		p += ETH_ALEN;

		os_memcpy(p, cmdu+CMDU_HLEN, tlv_len);
		p += tlv_len;

		length = p - p_old;

//		hex_dump_all("en key", entry->key, entry->key_len);
//		hex_dump_all("en clear_text", p_old, length);
        /* 256-bit MIC field using HMAC-SHA256 (see [14]) with the 1905 GTK */
		if (hmac_sha256(entry->key, entry->key_len, p_old, length, hash)) {
			return HAMC_FAIL;
		}

//		hex_dump_all("en hash", hash, SHA256_MAC_LEN);

		add_mic_tlv(cmdu, &_1905_len, hash, SHA256_MAC_LEN, entry->key_identifier,
			entry->encry_tx_counter, fragment, src_mac);

		*len = _1905_len + ETH_HLEN;
	} else if(entry->key_type == KEY_UNICAST) {
		/*
		"	K = 1905 TK (256-bit) corresponding to the receiver
		"	P = all of the TLVs in the message concatenated (except the End of Message TLV)
        "   AD1 = The first six octets of the 1905 CMDU.
        "   AD2 = The Encryption Transmission Counter value at the sender from the field in Encrypted Paylod TLV.
        "   AD3 = Source 1905 AL MAC Address from the field in Encrypted Paylod TLV.
        "   AD4 = Destination 1905 AL MAC Address from the field in Encrypted Paylod TLV.
		*/
		const unsigned char *addr[4];
		size_t len_a[4];

		addr[0] = cmdu;
		len_a[0] = 6;

		addr[1] = entry->encry_tx_counter;
		len_a[1] = COUNTER_LENGTH;

		addr[2] = src_mac;
		len_a[2] = ETH_ALEN;

		addr[3] = dst_mac;
		len_a[3] = ETH_ALEN;

		if (aes_siv_encrypt(entry->key, entry->key_len, tlv,
				fragment == 1 ? tlv_len : (tlv_len - 3),
			    4, addr, len_a, p) < 0) {
			return AES_SIV_ENC_FAIL;
		}

		add_encrypted_tlv(tlv, &tlv_len, p, entry->encry_tx_counter, fragment, src_mac, dst_mac);

		*len = tlv_len + CMDU_HLEN + ETH_HLEN;
	} else
		return UNKNOWN_KEY_TYPE;

    /* Increment the Integrity Transmission Counter by one */
	counter_increase(entry->encry_tx_counter);

	return SUC;
}

struct mic_tlv *get_mic_struct_from_mic_tlv(unsigned char *tlv)
{
	struct mic_tlv *mic = (struct mic_tlv *)tlv;

	mic->length = be_to_host16(mic->length);
	mic->mic_len = be_to_host16(mic->mic_len);
    mic->gik_identifier = (mic->gik_identifier >> 6) & 0x03;
	return mic;
}

struct encryped_tlv *get_encrypted_struct_from_mic_tlv(unsigned char *tlv)
{
	struct encryped_tlv *mic = (struct encryped_tlv *)tlv;

	mic->length = be_to_host16(mic->length);
	mic->aes_siv_len = be_to_host16(mic->aes_siv_len);

	return mic;
}


/*Fn to check the counter, 0-suc otherwise-fail*/

char check_counter(unsigned char *receiving_counter, unsigned char *received_counter)
{
	unsigned char i = 0;

	for (i = 0; i < COUNTER_LENGTH; i++) {
		if (receiving_counter[i] > received_counter[i])
			return 0;
		else if (receiving_counter[i] < received_counter[i])
			return -1;
	}

	//if equal counter, should check whether it is all zero, if yes, should regard it is check success
	if (!os_memcmp(zero_counter, receiving_counter, COUNTER_LENGTH))
		return 0;

	return -1;
}
/*
char check_counter(unsigned char *receiving_counter, unsigned char *received_counter)
{
	unsigned char i = 0;

	for (i = COUNTER_LENGTH - 1; i >= 0; i--) {
		if (receiving_counter[i] > received_counter[i])
			return 0;
		else if (receiving_counter[i] < received_counter[i])
			return -1;
	}

	//if equal counter, should check whether it is all zero, if yes, should regard it is check success
	if (!os_memcmp(zero_counter, receiving_counter, COUNTER_LENGTH))
		return 0;

	return -1;
}
*/

int sec_decrypt(IN unsigned char *dst_mac, IN unsigned char *src_mac,
	INOUT unsigned char *msg, INOUT unsigned short *len)
{
	struct security_entry *entry = NULL;
	struct gik_pool_entry *pool_gik_entry = NULL;
	struct gik_peer_entry *peer_entry = NULL;
	unsigned char *tlv = NULL, *mic_tlvs = NULL, *cmdu = NULL, *p = NULL, *p_old = encr_text, *tmp = NULL;
	unsigned short _1905_len = 0, tlv_len = 0, padding_len = 0, cipher_len = 0;
	unsigned char fragment = 0, flag_new_gik = 0;
	struct mic_tlv *mic = NULL;
	struct encryped_tlv *aes_siv = NULL;
	int ret = 0;

	if(!is_security_on(dst_mac))
		return SUC;

	if (!os_memcmp(_1905_multicast_address, dst_mac, ETH_ALEN)) {
		entry = sec_entry_lookup(dst_mac);
	} else {
		entry = sec_entry_lookup(src_mac);
	}

	if (entry == NULL) {
		ret = NO_KEY_FOUND;
		debug(DEBUG_ERROR, "[%d]NO_KEY_FOUND!\n", __LINE__);
		goto fail;
	}

	cmdu = msg;
	tlv = cmdu + CMDU_HLEN;

	tmp = get_tlv_pos(tlv, END_OF_TLV_TYPE, *len - CMDU_HLEN);
	if (tmp == NULL)
		fragment = 1;
	else
		padding_len = *len - (tmp + 3 -  msg);

	_1905_len = *len - padding_len;
	tlv_len = _1905_len - CMDU_HLEN;

	if (tlv_len > encr_text_len) {
		ret = INVALID_CMDU;
		debug(DEBUG_ERROR, "[%d]INVALID_CMDU!\n", __LINE__);
		goto fail;
	}

	if (entry->key_type == KEY_MULTICAST) {
		unsigned char *key = NULL;
		size_t key_len = 0;
		unsigned char hash[SHA256_MAC_LEN];

		mic_tlvs = get_tlv_pos(tlv, MIC_TLV_TYPE, tlv_len);
		if (mic_tlvs == NULL) {
			ret = NOT_ENCRYPTED;
			debug(DEBUG_ERROR, "[%d]NOT_ENCRYPTED!\n", __LINE__);
			goto fail;
		}

		mic = get_mic_struct_from_mic_tlv(mic_tlvs);
//		hex_dump_all("receive mic tlv", (unsigned char*)mic, sizeof(struct mic_tlv) + SHA256_MAC_LEN);

		peer_entry = gik_peer_entry_lookup(entry, src_mac);
		/*fisrt time receiving the multicast frame from peer device*/
		if (peer_entry == NULL) {
			peer_entry = (struct gik_peer_entry*)gik_peer_entry_alloc(src_mac);
			if (peer_entry == NULL) {
				ret = GIK_NOT_FOUND;
				debug(DEBUG_ERROR, "[%d]NO GKEY_FOUND!\n", __LINE__);
				goto fail;
			}
			gik_peer_entry_insert(entry, peer_entry);
		}

		/*check whether peer device use the new gik*/
		if (entry->key_identifier != mic->gik_identifier) {
			/*to temprally use the new gik to check*/
			pool_gik_entry = gik_pool_entry_lookup(mic->gik_identifier);
			if (pool_gik_entry == NULL) {
				ret = GIK_NOT_FOUND;
				debug(DEBUG_ERROR, "[%d]NO GKEY_FOUND!\n", __LINE__);
				goto fail;
			}
			key = pool_gik_entry->key;
			key_len = pool_gik_entry->key_len;
			flag_new_gik = 1;
		} else {
			key = entry->key;
			key_len = entry->key_len;
		}

		cipher_len = mic_tlvs - cmdu;

		/*if it is not fragment, the last ie is the end of message tlv*/
		if (fragment == 0) {
			os_memset(mic_tlvs, 0, 3);
			cipher_len += 3;
		}

		if (calculate_mic(key, key_len, mic->tx_counter, mic->gik_identifier,
			cmdu, cipher_len, src_mac, hash)) {
			ret = HAMC_FAIL;
			debug(DEBUG_ERROR, "[%d]HAMC_FAIL!\n", __LINE__);
			goto fail;
		}

		if (mic->mic_len <= SHA256_MAC_LEN && os_memcmp(hash, mic->mic, mic->mic_len)) {
			/*try to use new gik in gik pool with same key id*/
			if (flag_new_gik == 0) {
				pool_gik_entry = gik_pool_entry_lookup(mic->gik_identifier);
				if (pool_gik_entry == NULL) {
					ret = HMAC_NOT_EQUAL_RETRY;
					debug(DEBUG_ERROR, "[%d]HMAC_NOT_EQUAL_RETRY!\n", __LINE__);
					goto fail;
				}
				key = pool_gik_entry->key;
				key_len = pool_gik_entry->key_len;
				flag_new_gik = 1;

				/*try*/
				os_memset(hash, 0, SHA256_MAC_LEN);
				if (calculate_mic(key, key_len, mic->tx_counter, mic->gik_identifier,
					cmdu, cipher_len, src_mac, hash)) {
					ret = HAMC_FAIL;
					debug(DEBUG_ERROR, "[%d]HAMC_FAIL!\n", __LINE__);
					goto fail;
				}

				if (os_memcmp(hash, mic->mic, mic->mic_len)) {
					ret = HMAC_NOT_EQUAL_RETRY;
					debug(DEBUG_ERROR, "[%d]HMAC_NOT_EQUAL_RETRY!\n", __LINE__);
					goto fail;
				}
			} else {
				ret = HMAC_NOT_EQUAL;
				debug(DEBUG_ERROR, "[%d]HMAC_NOT_EQUAL!\n", __LINE__);
				goto fail;
			}
		}

		/*if peer send the multicast cmdu with new gik encryped, update the new gik to security entry*/
		if (flag_new_gik == 1) {
			os_memcpy(entry->key, key, key_len);
			entry->key_len = key_len;
			entry->key_identifier = mic->gik_identifier;
			gik_peer_entry_reset_rx_counter(entry);
			entry->decry_fail_counter  = 0;
			os_memset(entry->encry_tx_counter, 0, COUNTER_LENGTH);
			counter_increase(entry->encry_tx_counter);
			//os_memset(peer_entry->multi_rx_counter, 0, COUNTER_LENGTH);

			gik_pool_entry_delete(mic->gik_identifier);
		} else {
			if (check_counter(mic->tx_counter, peer_entry->multi_rx_counter)) {
                debug(DEBUG_ERROR, "mc tx counter:"MACSTR", rx counter "MACSTR"\n",
                    MAC2STR(mic->tx_counter), MAC2STR(peer_entry->multi_rx_counter));
				return INVALID_RX_COUNTER;
			}
			os_memcpy(peer_entry->multi_rx_counter, mic->tx_counter, COUNTER_LENGTH);
		}
		*len = cipher_len + ETH_HLEN;
	} else if (entry->key_type == KEY_UNICAST) {
		/*
        "   K = 1905 TK (256-bit) corresponding to the receiver
        "   P = all of the TLVs in the message concatenated (except the End of Message TLV)
        "   AD1 = The first six octets of the 1905 CMDU.
        "   AD2 = The Encryption Transmission Counter value at the sender from the field in Encrypted Paylod TLV.
        "   AD3 = Source 1905 AL MAC Address from the field in Encrypted Paylod TLV.
        "   AD4 = Destination 1905 AL MAC Address from the field in Encrypted Paylod TLV.
		*/
		const unsigned char *addr[4];
		size_t len_a[4];

		mic_tlvs = get_tlv_pos(tlv, ENCRYPTED_TLV_TYPE, tlv_len);
		if (mic_tlvs == NULL) {
			ret = NOT_ENCRYPTED;
			debug(DEBUG_ERROR, "[%d]NOT_ENCRYPTED!\n", __LINE__);
			goto fail;
		}
		aes_siv = get_encrypted_struct_from_mic_tlv(mic_tlvs);
		if (aes_siv->aes_siv_len > tlv_len + AES_BLOCK_SIZE) {
			debug(DEBUG_ERROR, "aes_siv_len %d bigger than %d\n",
				aes_siv->aes_siv_len, tlv_len + AES_BLOCK_SIZE);
			ret = INVALID_CMDU;
			goto fail;
		}

		if (check_counter(aes_siv->tx_counter, entry->decry_rx_counter)) {
			debug(DEBUG_ERROR, "uc tx counter:"MACSTR", rx counter "MACSTR"\n",
				MAC2STR(aes_siv->tx_counter), MAC2STR(entry->decry_rx_counter));
			ret = INVALID_RX_COUNTER;
			goto fail;
		}

		addr[0] = cmdu;
		len_a[0] = 6;

		addr[1] = aes_siv->tx_counter;
		len_a[1] = COUNTER_LENGTH;

		addr[2] = aes_siv->src_mac;
		len_a[2] = ETH_ALEN;

		addr[3] = aes_siv->dst_mac;
		len_a[3] = ETH_ALEN;

		if (aes_siv_decrypt(entry->key, entry->key_len,
					aes_siv->aes, aes_siv->aes_siv_len,
					4, addr, len_a, p_old) < 0) {
			ret = AES_SIV_DECRYPT_FAIL;
			debug(DEBUG_ERROR, "[%d]AES_SIV_DECRYPT_FAIL!\n", __LINE__);
			goto fail;
		}
		cipher_len = aes_siv->aes_siv_len - AES_BLOCK_SIZE;
		os_memcpy(entry->decry_rx_counter, aes_siv->tx_counter, COUNTER_LENGTH);

		p = tlv;
		os_memcpy(p, p_old, cipher_len);
		p += cipher_len;

		/*if not fragment, add the end of message tlv */
		if (fragment == 0) {
			os_memset(p, 0, 3);
			p += 3;
		}

		*len = (unsigned short)(p - msg);

	}

	return SUC;
fail:
	if (entry)
		entry->decry_fail_counter++;
	return ret;
}

int dump_security_info(char *reply, int reply_size)
{
	int i = 0, j = 0, k = 0;
	struct security_entry *sec_entry = NULL;
	struct gik_pool_entry *pool_entry = NULL;
	struct gik_peer_entry *peer_entry = NULL;
	char *pos = reply;
	int len = 0, total_len = 0;

	if (!reply || reply_size == 0)
		return -1;

	len = os_snprintf(pos + total_len, reply_size - total_len, "###############dump security info##############\n");
	if (os_snprintf_error(reply_size - total_len, len)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return -1;
	}
	total_len += len;

	len = os_snprintf(pos + total_len, reply_size - total_len,
						"sucurity config %s%s%s\n",
						(security_config & 0x01) ? "[MULTICAST] " : "",
						(security_config & 0x02) ? "[UNICAST] " : "",
						security_config == 0 ? "DISABLED" : "");
	if (os_snprintf_error(reply_size - total_len, len)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return -1;
	}
	total_len += len;

	len = os_snprintf(pos + total_len, reply_size - total_len, "1.security entry info\n");
	if (os_snprintf_error(reply_size - total_len, len)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return -1;
	}
	total_len += len;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		if (dl_list_empty(&sec_hash_entry[i]))
			continue;
		dl_list_for_each(sec_entry, &sec_hash_entry[i], struct security_entry, entry) {
			len = os_snprintf(pos + total_len, reply_size - total_len,
							">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;

			len = os_snprintf(pos + total_len, reply_size - total_len,
							"\t[AL MAC]          %02x:%02x:%02x:%02x:%02x:%02x\n",
							PRINT_MAC(sec_entry->almac));
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;

			len = os_snprintf(pos + total_len, reply_size - total_len, "\t[key]\t%d bytes", (int)sec_entry->key_len);
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;

			for (j = 0; j < WPA_TK_MAX_LEN; j++) {
				if (j  % 16 == 0) {
					len = os_snprintf(pos + total_len, reply_size - total_len, "\n\t\t");
					if (os_snprintf_error(reply_size - total_len, len)) {
						debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
						return -1;
					}
					total_len += len;
				}
				len = os_snprintf(pos + total_len, reply_size - total_len, "%02x", sec_entry->key[j]);
				if (os_snprintf_error(reply_size - total_len, len)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					return -1;
				}
				total_len += len;
			}
			len = os_snprintf(pos + total_len, reply_size - total_len,
							"\n\t[key type]         %s\n",
							key_type2string(sec_entry->key_type));
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;

			len = os_snprintf(pos + total_len, reply_size - total_len,
							"\t[encry_tx_counter] %02x%02x%02x%02x%02x%02x\n",
							PRINT_MAC(sec_entry->encry_tx_counter));
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;

			len = os_snprintf(pos + total_len, reply_size - total_len,
							"\t[decry_rx_counter] %02x%02x%02x%02x%02x%02x\n",
							PRINT_MAC(sec_entry->decry_rx_counter));
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;

			len = os_snprintf(pos + total_len, reply_size - total_len,
							"\t[decry_fail_count] %d\n",
							sec_entry->decry_fail_counter);
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;

			if (sec_entry->key_type == KEY_MULTICAST) {
				len = os_snprintf(pos + total_len, reply_size - total_len,
								"\t[key_identifier]   %d\n",
								sec_entry->key_identifier);
				if (os_snprintf_error(reply_size - total_len, len)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					return -1;
				}
				total_len += len;

				len = os_snprintf(pos + total_len, reply_size - total_len, "dump gik peer info\n");
				if (os_snprintf_error(reply_size - total_len, len)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					return -1;
				}
				total_len += len;
				for (k = 0; k < HASH_TABLE_SIZE; k++) {
					dl_list_for_each(peer_entry, &sec_entry->gik_peer_entry[k], struct gik_peer_entry, entry) {
						len = os_snprintf(pos + total_len, reply_size - total_len,
									"\t\t[AL MAC]			%02x:%02x:%02x:%02x:%02x:%02x\n",
									PRINT_MAC(peer_entry->peer_mac));
						if (os_snprintf_error(reply_size - total_len, len)) {
							debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
							return -1;
						}
						total_len += len;

						len = os_snprintf(pos + total_len, reply_size - total_len,
										"\t\t[multi_rx_counter]  %02x%02x%02x%02x%02x%02x\n",
										PRINT_MAC(peer_entry->multi_rx_counter));
						if (os_snprintf_error(reply_size - total_len, len)) {
							debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
							return -1;
						}
						total_len += len;
					}
				}
			}

			len = os_snprintf(pos + total_len, reply_size - total_len,
							"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;
		}
	}

	len = os_snprintf(pos + total_len, reply_size - total_len, "2.gik pool list info\n");
	if (os_snprintf_error(reply_size - total_len, len)) {
		debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
		return -1;
	}
	total_len += len;
	dl_list_for_each(pool_entry, &gik_pool_list, struct gik_pool_entry, entry) {
		len = os_snprintf(pos + total_len, reply_size - total_len,
						"\t[key_identifier]	 %d\n", pool_entry->key_identifier);
		if (os_snprintf_error(reply_size - total_len, len)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			return -1;
		}
		total_len += len;

		len = os_snprintf(pos + total_len, reply_size - total_len, "\t[key]\n");
		if (os_snprintf_error(reply_size - total_len, len)) {
			debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
			return -1;
		}
		total_len += len;
		for (j = 1; j <= WPA_TK_MAX_LEN; j++) {
			if (j % 16 == 0) {
				len = os_snprintf(pos + total_len, reply_size - total_len, "\n");
				if (os_snprintf_error(reply_size - total_len, len)) {
					debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
					return -1;
				}
				total_len += len;
			}
			len = os_snprintf(pos + total_len, reply_size - total_len, "%02x", pool_entry->key[j - 1]);
			if (os_snprintf_error(reply_size - total_len, len)) {
				debug(DEBUG_ERROR, "[%d]snprintf fail!\n", __LINE__);
				return -1;
			}
			total_len += len;
		}
	}
	return total_len;
}

/*1905ctrl agent/controller set_key xx:xx:xx:xx:xx:xx 01 0123456789012345678901234567890123456789012345678901234567890123*/
int cmd_set_key(char* buf)
{
	unsigned int i = 0;
	unsigned char al_mac[ETH_ALEN];
	unsigned char key_identifier = 0;
	unsigned char key[WPA_TK_MAX_LEN];

	if (hwaddr_aton(buf, al_mac) < 0) {
		debug(DEBUG_ERROR, "wrong al mac\n");
		return -1;
	}
	buf += 18;

	key_identifier = hex2byte(buf);

	buf += 3;
	for (i = 0; i < WPA_TK_MAX_LEN; i++) {
		key[i] = hex2byte(buf);
		buf += 2;
	}

	debug(DEBUG_ERROR, "set key %02x:%02x:%02x:%02x:%02x:%02x %02x\n",
		PRINT_MAC(al_mac), key_identifier);
	hex_dump_all("key", key, WPA_TK_MAX_LEN);

	if (sec_set_key(al_mac, key, WPA_TK_MAX_LEN, key_identifier)) {
		debug(DEBUG_ERROR, "set key error\n");
		return -1;
	}
	return 0;
}

/*1905ctrl agent/controller set_key xx:xx:xx:xx:xx:xx 01 0123456789012345678901234567890123456789012345678901234567890123*/
int cmd_set_security_config(char* buf)
{
	unsigned char on = 0;

	on = atoi(buf);

	debug(DEBUG_ERROR, "set sucurity config %s%s%s\n", (on & 0x01) ? "[MULTICAST] " :"",
		(on & 0x02) ? "[UNICAST] " :"", on == 0 ? "DISABLED" : "");

	security_config = on;

	return 0;
}

int is_security_on(unsigned char* da)
{
	if (!os_memcmp(da, _1905_multicast_address, ETH_ALEN)) {
		return security_config & 0x01;
	} else
		return security_config & 0x02;
}

void security_hex_dump(int level, char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	if (level <= sec_debug_level)
		hex_dump_all(str, pSrcBufVA, SrcBufLen);
}

void cmd_raw_data_mic_calculate(IN unsigned char *key, IN size_t key_len, IN unsigned char *counter,
	IN unsigned char gik_identifier, IN unsigned char *payload, IN size_t payload_len);

void set_security_log_level(int level)
{

	if (level < 0 || level > DEBUG_INFO)
		debug(DEBUG_ERROR, "invalid security log level %d\n", level);

	sec_debug_level = level;
	debug(DEBUG_ERROR, "set security log level to %d\n", level);
#if 0

	unsigned char key[WPA_TK_MAX_LEN] = {0x30, 0x30, 0x31, 0x31, 0x30, 0x30, 0x31,
										 0x31, 0x30, 0x30, 0x31, 0x31, 0x30, 0x30,
										 0x31, 0x31, 0x30, 0x30, 0x31, 0x31, 0x30,
										 0x30, 0x31, 0x31, 0x30, 0x30, 0x31, 0x31,
										 0x30, 0x30, 0x31, 0x31};
	size_t key_len = WPA_TK_MAX_LEN;

	unsigned char counter[COUNTER_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char key_id = 0x00;

	unsigned char payload[256] = {	0x01, 0x80, 0xc2, 0x00, 0x00, 0x13, 0x02, 0x90, 0x4c, 0x30, 0x71, 0x44, 0x89, 0x3a, 0x00, 0x00,
									0x00, 0x01, 0x45, 0xa9, 0x00, 0xc0, 0x01, 0x00, 0x06, 0x02, 0x90, 0x4c, 0x30, 0x71, 0x44, 0x00,
									0x00, 0x00};
	size_t payload_len = 34;

	cmd_raw_data_mic_calculate(key, key_len, counter, key_id, payload, payload_len);
#endif
}

void cmd_raw_data_mic_calculate(IN unsigned char *key, IN size_t key_len, IN unsigned char *counter,
	IN unsigned char gik_identifier, IN unsigned char *payload, IN size_t payload_len)
{
	unsigned char hash[SHA256_MAC_LEN] = {0};
    unsigned char src_mac[ETH_ALEN] = {0};

	calculate_mic(key, key_len, counter, gik_identifier, payload, payload_len, src_mac, hash);

	hex_dump_all("mic", hash, SHA256_MAC_LEN);
}


