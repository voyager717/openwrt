#ifndef SECURITY_ENGINE_H
#define SECURITY_ENGINE_H

#include "list.h"
#include "p1905_managerd.h"

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define WPA_TK_MAX_LEN 32

#define IN
#define OUT
#define INOUT

#define SEC_DEBUG_OFF 		LOG_EMERG
#define SEC_DEBUG_ERROR		LOG_ALERT  
#define SEC_DEBUG_WARN		LOG_CRIT 
#define SEC_DEBUG_TRACE		LOG_ERR 
#define SEC_DEBUG_INFO		LOG_WARNING

//#define HASH_TABLE_SIZE                 256	/* Size of hash tab must be power of 2. */
#define SEC_TEXT_LEN					20480
//#define MAC_ADDR_HASH(Addr)            (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
//#define MAC_ADDR_HASH_INDEX(Addr)      (MAC_ADDR_HASH(Addr) & (HASH_TABLE_SIZE - 1))
#define COUNTER_LENGTH					6

#define MIC_TLV_TYPE					0xAB
#define ENCRYPTED_TLV_TYPE				0xAC
enum KEY_TYPE {
	KEY_UNKNOWN,
	KEY_MULTICAST,
	KEY_UNICAST,
};

enum {
	SUC,
	NO_KEY_FOUND,
	INVALID_CMDU,
	HAMC_FAIL,
	HMAC_NOT_EQUAL,
	GIK_NOT_FOUND,
	UNKNOWN_KEY_TYPE,
	AES_SIV_ENC_FAIL,
	AES_SIV_DECRYPT_FAIL,
	NOT_ENCRYPTED,
	INVALID_RX_COUNTER,	
	HMAC_NOT_EQUAL_RETRY,
};

#define SHA256_MAC_LEN 32

#define MAX_KEY_IDENTIFIER 6;				/*temporally set to 6 octets*/

struct gik_pool_entry {
	unsigned char key[WPA_TK_MAX_LEN];
	size_t key_len;
	unsigned char key_identifier;
	struct dl_list entry;
};

struct gik_peer_entry {
	unsigned char peer_mac[ETH_ALEN];
	unsigned char multi_rx_counter[COUNTER_LENGTH];
	struct dl_list entry;
};

struct security_entry {
	unsigned char almac[ETH_ALEN];
	unsigned char key[WPA_TK_MAX_LEN];
	size_t key_len;
	enum KEY_TYPE key_type;
	unsigned char encry_tx_counter[COUNTER_LENGTH];
	unsigned char decry_rx_counter[COUNTER_LENGTH];
	unsigned int decry_fail_counter;
	struct dl_list entry;
	/*below members is valid only the key_type equals to MULTICAST*/
	unsigned char key_identifier;
	struct dl_list gik_peer_entry[HASH_TABLE_SIZE];
};

struct GNU_PACKED encryped_tlv {
	unsigned char tlv_type;
	unsigned short length;
	unsigned char tx_counter[COUNTER_LENGTH];
    unsigned char src_mac[ETH_ALEN];
    unsigned char dst_mac[ETH_ALEN];
	unsigned short aes_siv_len;
	unsigned char aes[0];
};

struct GNU_PACKED mic_tlv {
	unsigned char tlv_type;
	unsigned short length;
	unsigned char gik_identifier;
	unsigned char tx_counter[COUNTER_LENGTH];
    unsigned char src_mac[ETH_ALEN];
	unsigned short mic_len;
	unsigned char mic[0];
};

int security_engine_init(unsigned char *buf, unsigned int len);
void sec_update_buf(unsigned char *buf, unsigned int len);
int security_engine_deinit();
int sec_set_key(unsigned char *almac, unsigned char *key, size_t key_len, 
	unsigned char key_identifier);

int sec_encrypt(IN unsigned char *dst_mac, IN unsigned char *src_mac,
	INOUT unsigned char *msg, INOUT unsigned short *len, IN unsigned char fragment);

int sec_decrypt(IN unsigned char *dst_mac, IN unsigned char *src_mac,
	INOUT unsigned char *msg, INOUT unsigned short *len);

int dump_security_info(char *reply, int reply_size);
int cmd_set_key(char* buf);
char *code2string(unsigned char code);
int cmd_set_security_config(char* buf);
int is_security_on(unsigned char* da);
void security_hex_dump(int level, char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void set_security_log_level(int level);
int ptk_peer_reset_encrypt_txrx_counter(IN unsigned char *dst_mac);
struct security_entry *sec_entry_lookup(unsigned char *almac);
unsigned char *get_tlv_pos(unsigned char *tlvs, unsigned char tlv_type, unsigned short tlv_len);
int sec_entry_clear_by_mac(unsigned char *almac);
int sec_entry_get_by_mac(unsigned char *almac);

#endif
