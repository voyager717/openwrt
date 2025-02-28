#ifndef _MAP_DDP_H
#define _MAP_DDP_H

#include "list.h"
#include "p1905_managerd.h"
#include "dpp.h"

#define BSSNUM_PER_BSS_RESPONSE_TLV 16

typedef enum {
	dpp_idle = 0,
	dpp_receive_dpp_disc_req,
	dpp_send_dpp_disc_resp,
	dpp_send_dpp_disc_req,
	dpp_wait_dpp_disc_resp,
	dpp_disc_suc,
	dpp_disc_fail,
} dpp_state;

struct GNU_PACKED r3_information {
	unsigned short retry_cnt;
	unsigned short connector_len;
	unsigned char *connector;
	unsigned char pmkid[PMKID_LEN];
	unsigned char pmk[PMK_LEN_MAX];
	unsigned int pmk_len;
	dpp_state cur_dpp_state;
	struct os_time update_time;
};

struct r3_member {
	struct dl_list entry;
	unsigned char al_mac[ETH_ALEN];
	unsigned char active;
	unsigned char profile;
	unsigned char role;
	unsigned char security_1905;
	unsigned char dec_fail_cnt;
	struct akm_suite_cap fh_akm;
	struct akm_suite_cap bh_akm;
	struct r3_information r3_info;
	struct chirp_tlv_info chirp;
};

extern struct dl_list r3_member_head;

void r3_wait_for_dpp_auth_req(void *eloop_ctx, void *timeout_ctx);
void map_dpp_init();
void map_dpp_deinit(struct p1905_managerd_ctx *ctx);
struct r3_member *create_r3_member(unsigned char *al_mac);
struct r3_member *get_r3_member(unsigned char *al_mac);
void delete_r3_member(struct p1905_managerd_ctx *ctx, unsigned char *al_mac);
void map_dpp_handle_public_action_frame(struct p1905_managerd_ctx *ctx,
	unsigned char public_action_type, unsigned char *buf, size_t len,
	unsigned char *al_mac);
void map_dpp_trigger_member_dpp_intro(struct p1905_managerd_ctx *ctx, struct r3_member *peer);
void map_cancel_onboarding_timer(struct p1905_managerd_ctx *ctx, struct r3_member *peer);
void map_dpp_trigger_all_member_dpp_intro(struct p1905_managerd_ctx *ctx);
void map_dpp_save_keys(struct r3_dpp_information *dpp);
void map_dpp_save_bss_connector(struct bss_connector_item *bc);
void timeout_delete_r3_member(void *eloop_data, void *user_ctx);

#endif
