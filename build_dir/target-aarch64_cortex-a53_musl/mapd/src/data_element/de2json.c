#include <time.h>
#include "includes.h"
#include "os.h"
#include "cJSON.h"
#include "base64.h"
#include "de2json.h"

#include "topologySrv.h"
#include "client_db.h"


void get_timestamp_str(char *buf, char len)
{
	time_t timep;
	struct tm *p;
	//time_t time_utc = 0;
	//struct tm *p_tm_time;
	//int time_zone = 0;

	if (!buf || len < 64) {
		err("invalid param! buf(%p) len(%d)", buf, len);
		os_memset(buf, 0, len);
		return;
	}

	time(&timep);
	p = gmtime(&timep);

	os_snprintf(buf, len, "%04d-%02d-%02dT%02d:%02d:%02d.0+00:00",
		(1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday,
		 p->tm_hour, p->tm_min, p->tm_sec);

	//p_tm_time = localtime( &time_utc );
	//time_zone = (p_tm_time->tm_hour > 12) ? (p_tm_time->tm_hour -= 24) : p_tm_time->tm_hour;

	/*os_snprintf(buf, len, "%04d-%02d-%02dT%02d:%02d:%02d.0%s%02d:00",
		(1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday,
		 p->tm_hour, p->tm_min, p->tm_sec,
		 time_zone >= 0 ? "+" : "-", time_zone);*/


}



/*********************************************************************/
cJSON *de_create_sta_wf6_obj( struct client *cli)
{
	cJSON *wf6_obj = NULL;
	char *base64_str = NULL;
	int n = 0;
	unsigned char he_mcs[MAX_HE_MCS_LEN] = {0};
	char he_mcs_len = 0;

	if (!cli || !cli->che.valid) {
		err(DE_PREX"invalid parameters!!! cli is NULL or cli wf6 cap is invalid");
		wf6_obj = cJSON_CreateNull();
		return wf6_obj;
	}

	wf6_obj = cJSON_CreateObject();
	if (cli->che.he_160)
		cJSON_AddTrueToObject(wf6_obj, "HE160");
	else
		cJSON_AddFalseToObject(wf6_obj, "HE160");
	if (cli->che.he_80plus80)
		cJSON_AddTrueToObject(wf6_obj, "HE8080");
	else
		cJSON_AddFalseToObject(wf6_obj, "HE8080");

	/* mcs transfer to big endian */
	he_mcs_len = cli->che.he_mcs_len > MAX_HE_MCS_LEN ? MAX_HE_MCS_LEN : cli->che.he_mcs_len;
	for (n = 0; n < he_mcs_len; n += 2) {
		he_mcs[n + 1] = cli->che.he_mcs[n];
		he_mcs[n] = cli->che.he_mcs[n + 1];
	}
	base64_str =  (char *)base64_encode(he_mcs, he_mcs_len, NULL);
	if (base64_str) {
		cJSON_AddStringToObject(wf6_obj, "MCSNSS", base64_str);
		os_free(base64_str);
		base64_str = NULL;
	} else {
		cJSON_AddNullToObject(wf6_obj, "MCSNSS");
	}


	if (cli->che.su_beamformer)
		cJSON_AddTrueToObject(wf6_obj, "SUBeamformer");
	else
		cJSON_AddFalseToObject(wf6_obj, "SUBeamformer");
	if (cli->che.su_beamformee)
		cJSON_AddTrueToObject(wf6_obj, "SUBeamformee");
	else
		cJSON_AddFalseToObject(wf6_obj, "SUBeamformee");
	if (cli->che.mu_beamformer)
		cJSON_AddTrueToObject(wf6_obj, "MUBeamformer");
	else
		cJSON_AddFalseToObject(wf6_obj, "MUBeamformer");
	if (cli->che.beamformee_sts_less80)
		cJSON_AddTrueToObject(wf6_obj, "Beamformee80orLess");
	else
		cJSON_AddFalseToObject(wf6_obj, "Beamformee80orLess");
	if (cli->che.beamformee_sts_more80)
		cJSON_AddTrueToObject(wf6_obj, "BeamformeeAbove80");
	else
		cJSON_AddFalseToObject(wf6_obj, "BeamformeeAbove80");
	if (cli->che.ul_mu_mimo)
		cJSON_AddTrueToObject(wf6_obj, "ULMUMIMO");
	else
		cJSON_AddFalseToObject(wf6_obj, "ULMUMIMO");
	if (cli->che.ul_ofdma_supported)
		cJSON_AddTrueToObject(wf6_obj, "ULOFDMA");
	else
		cJSON_AddFalseToObject(wf6_obj, "ULOFDMA");
	if (cli->che.dl_ofdma_supported)
		cJSON_AddTrueToObject(wf6_obj, "DLOFDMA");
	else
		cJSON_AddFalseToObject(wf6_obj, "DLOFDMA");
	/*the following four parameters are used only on AP role*/
	cJSON_AddNumberToObject(wf6_obj, "MaxDLMUMIMO", 0);
	cJSON_AddNumberToObject(wf6_obj, "MaxULMUMIMO", 0);
	cJSON_AddNumberToObject(wf6_obj, "MaxDLOFDMA", 0);
	cJSON_AddNumberToObject(wf6_obj, "MaxULOFDMA", 0);
	if (cli->che.rts_status)
		cJSON_AddTrueToObject(wf6_obj, "RTS");
	else
		cJSON_AddFalseToObject(wf6_obj, "RTS");
	if (cli->che.mu_rts_status)
		cJSON_AddTrueToObject(wf6_obj, "MURTS");
	else
		cJSON_AddFalseToObject(wf6_obj, "MURTS");
	/*the following 2 parameters are used only on AP role*/
	cJSON_AddFalseToObject(wf6_obj, "MultiBSSID");
	cJSON_AddFalseToObject(wf6_obj, "MUEDCA");
	if (cli->che.twt_requester_status)
		cJSON_AddTrueToObject(wf6_obj, "TWTRequestor");
	else
		cJSON_AddFalseToObject(wf6_obj, "TWTRequestor");
	if (cli->che.twt_responder_status)
		cJSON_AddTrueToObject(wf6_obj, "TWTResponder");
	else
		cJSON_AddFalseToObject(wf6_obj, "TWTResponder");

	return wf6_obj;
}


cJSON *de_create_station_obj(struct own_1905_device *ctx, struct associated_clients *sta)
{
	cJSON *sa_obj = NULL;
	cJSON *measure_report_arr = NULL;
	cJSON *measure_report_obj = NULL;
#ifdef MAP_R3
	cJSON *tid_arr = NULL;
	cJSON *tid_obj = NULL;
#endif
#ifdef MAP_R2
	cJSON *wf6_obj = NULL;
#endif
    struct client *cli = NULL;
#ifdef MAP_R2
	char *base64_str = NULL;
#endif
	PEID_STRUCT eid_ptr = NULL;
	struct os_reltime cur_time;
	char mac_str[32] = {0};
	char timestamp_str[64] = {0};
#ifdef MAP_R2
	char he_cap[32] = {0};
	char he_mcs_len = 0;
	int n = 0;
#endif
	char cli_found = 0;
	char ht_cap = 0;
	char vht_cap[6] = {0};
#ifdef MAP_R3
	unsigned char i = 0;
#endif

	if (!ctx || !sta) {
		err(DE_PREX"invalid parameters!!! ctx(%p) or sta(%p) NULL", ctx, sta);
		goto fail;
	}

	sa_obj = cJSON_CreateObject();
	snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(sta->client_addr));
	cJSON_AddStringToObject(sa_obj, "MACAddress", mac_str);


	get_timestamp_str(timestamp_str, sizeof(timestamp_str));
	cJSON_AddStringToObject(sa_obj, "TimeStamp", timestamp_str);

	os_get_reltime(&cur_time);

	dl_list_for_each(cli, &ctx->sta_seen_list, struct client, sta_seen_entry)
    {
        if (!os_memcmp(cli->mac_addr, sta->client_addr, ETH_ALEN)) {
            cli_found = 1;
            break;
        }
    }
	if (cli_found) {
		if (cli->cht.valid) {
			ht_cap = (cli->cht.tx_stream << 6) | (cli->cht.rx_stream << 4) |
				(cli->cht.sgi_20 << 3) | (cli->cht.sgi_40 << 2) | (cli->cht.ht_40 << 1);
			base64_str = (char *)base64_encode((u8 *)&ht_cap, sizeof(ht_cap), NULL);
			if (base64_str) {
				cJSON_AddStringToObject(sa_obj, "HTCapabilities", base64_str);
				os_free(base64_str);
				base64_str = NULL;
			} else {
				cJSON_AddNullToObject(sa_obj, "HTCapabilities");
			}

		} else {
			cJSON_AddNullToObject(sa_obj, "HTCapabilities");
		}

		if (cli->cvht.valid) {
			/* mcs transfer to big endian */
			vht_cap[0] = cli->cvht.vht_tx_mcs[1];
			vht_cap[1] = cli->cvht.vht_tx_mcs[0];
			vht_cap[2] = cli->cvht.vht_rx_mcs[1];
			vht_cap[3] = cli->cvht.vht_rx_mcs[0];

			vht_cap[4] = (cli->cvht.tx_stream << 5) | (cli->cvht.rx_stream << 2) |
				(cli->cvht.sgi_80 << 1) | cli->cvht.sgi_160;
			vht_cap[5] = (cli->cvht.vht_8080 << 7) | (cli->cvht.vht_160 << 6) |
				(cli->cvht.su_beamformer << 5) | (cli->cvht.mu_beamformer << 4);
			base64_str =  (char *)base64_encode((u8 *)vht_cap, sizeof(vht_cap), NULL);
			cJSON_AddStringToObject(sa_obj, "VHTCapabilities", base64_str);
			if (base64_str) {
				cJSON_AddStringToObject(sa_obj, "VHTCapabilities", base64_str);
				os_free(base64_str);
				base64_str = NULL;
			} else {
				cJSON_AddNullToObject(sa_obj, "VHTCapabilities");
			}
		} else {
			cJSON_AddNullToObject(sa_obj, "VHTCapabilities");
		}
#ifdef MAP_R2
		if (cli->che.valid) {
			he_mcs_len = cli->che.he_mcs_len > MAX_HE_MCS_LEN ? MAX_HE_MCS_LEN : cli->che.he_mcs_len;
			he_cap[0] = he_mcs_len;
			/* mcs transfer to big endian */
			for (n = 0; n < he_mcs_len; n += 2) {
				he_cap[1 + n + 1] = cli->che.he_mcs[n];
				he_cap[1 + n] = cli->che.he_mcs[n + 1];
			}

			he_cap[1 + he_mcs_len] = (cli->che.tx_spatial_stream << 5) | (cli->che.rx_spatial_streams << 2) |
				(cli->che.he_80plus80 << 1) | cli->che.he_160;
			he_cap[2 + he_mcs_len] = (cli->che.su_beamformer << 7) | (cli->che.mu_beamformer << 6) |
				(cli->che.ul_mu_mimo << 5) | (cli->che.ul_ofdma_plus_mu_mimo << 4) |
				(cli->che.dl_ofdma_plus_mu_mimo << 3) | (cli->che.ul_ofdma_supported << 2) |
				(cli->che.dl_ofdma_supported << 1);
			base64_str = (char *)base64_encode((u8 *)he_cap, 3 + he_mcs_len, NULL);
			if (base64_str) {
				cJSON_AddStringToObject(sa_obj, "HECapabilities", base64_str);
				os_free(base64_str);
				base64_str = NULL;
			} else {
				cJSON_AddNullToObject(sa_obj, "HECapabilities");
			}
			cJSON_AddStringToObject(sa_obj, "HECapabilities", base64_str);

			/*wf6*/
			wf6_obj = de_create_sta_wf6_obj(cli);
			cJSON_AddItemToObject(sa_obj, "WiFi6Capabilities", wf6_obj);
		}  else {
			cJSON_AddNullToObject(sa_obj, "HECapabilities");
			cJSON_AddNullToObject(sa_obj, "WiFi6Capabilities");
		}
		if (cli->assoc_req_ie) {
			base64_str = (char *)base64_encode((unsigned char *)cli->assoc_req_ie, cli->assoc_req_ie_len, NULL);
			err("jing assoc_req_ie_len(%d) mac"MACSTR"", cli->assoc_req_ie_len, MAC2STR(cli->mac_addr));
			if (base64_str) {
				cJSON_AddStringToObject(sa_obj, "ClientCapabilities", base64_str);
				os_free(base64_str);
				base64_str = NULL;
			} else {
				cJSON_AddNullToObject(sa_obj, "ClientCapabilities");
			}
		} else {
			cJSON_AddNullToObject(sa_obj, "ClientCapabilities");
		}
#endif
		cJSON_AddNumberToObject(sa_obj, "LastConnectTime", difftime(cur_time.sec, cli->assoc_time.sec));
	} else {
		cJSON_AddNullToObject(sa_obj, "HTCapabilities");
		cJSON_AddNullToObject(sa_obj, "VHTCapabilities");
		cJSON_AddNullToObject(sa_obj, "HECapabilities");
		cJSON_AddNullToObject(sa_obj, "ClientCapabilities");
		cJSON_AddNumberToObject(sa_obj, "LastConnectTime", 0);
	}
#ifdef MAP_R2
	cJSON_AddNumberToObject(sa_obj, "LastDataDownlinkRate", sta->sta_ext_info.last_data_dl_rate);
	cJSON_AddNumberToObject(sa_obj, "LastDataUplinkRate", sta->sta_ext_info.last_data_ul_rate);
	cJSON_AddNumberToObject(sa_obj, "UtilizationReceive", sta->sta_ext_info.utilization_rx);
	cJSON_AddNumberToObject(sa_obj, "UtilizationTransmit", sta->sta_ext_info.utilization_tx);
#endif
	cJSON_AddNumberToObject(sa_obj, "EstMACDataRateDownlink", sta->erate_downlink);
	cJSON_AddNumberToObject(sa_obj, "EstMACDataRateUplink", sta->erate_uplink);
	cJSON_AddNumberToObject(sa_obj, "SignalStrength", sta->rssi_uplink);

#ifdef CENT_STR /*why wrap under CENT_STR*/
	cJSON_AddNumberToObject(sa_obj, "BytesSent", sta->stat_db.bytes_sent);
	cJSON_AddNumberToObject(sa_obj, "BytesReceived",  sta->stat_db.bytes_received);
	cJSON_AddNumberToObject(sa_obj, "PacketsSent",  sta->stat_db.packets_sent);
	cJSON_AddNumberToObject(sa_obj, "PacketsReceived",  sta->stat_db.packets_received);
	cJSON_AddNumberToObject(sa_obj, "ErrorsSent",  sta->stat_db.tx_packets_errors);
	cJSON_AddNumberToObject(sa_obj, "ErrorsReceived",  sta->stat_db.rx_packets_errors);
	cJSON_AddNumberToObject(sa_obj, "RetransCount",  sta->stat_db.retransmission_count);
#endif

	/*MeasurementReport not implement*/
	cJSON_AddNumberToObject(sa_obj, "NumberOfMeasureReports", cli->num_beacon_report);
	if (cli->num_beacon_report) {
		measure_report_arr = cJSON_AddArrayToObject(sa_obj, "MeasurementReport");
		eid_ptr = (PEID_STRUCT)cli->beacon_report_ie;
		while (((u8 *)eid_ptr + eid_ptr->Len + 1) < ((u8 *)cli->beacon_report_ie + cli->beacon_report_ie_len)) {
			base64_str = (char *)base64_encode((unsigned char *)eid_ptr, eid_ptr->Len + 2, NULL);
			if (base64_str) {
				measure_report_obj = cJSON_CreateString(base64_str);
				cJSON_AddItemToArray(measure_report_arr, measure_report_obj);
				os_free(base64_str);
				base64_str = NULL;
			} else {
				measure_report_obj = cJSON_CreateNull();
				cJSON_AddItemToArray(measure_report_arr, measure_report_obj);
				err(DE_PREX"memory alloc fail for base64_str to the measure_report_obj");
				break;
			}
			eid_ptr = (PEID_STRUCT)((u8 *)eid_ptr + 2 + eid_ptr->Len);
		}
	} else {
		cJSON_AddNullToObject(sa_obj, "MeasurementReport");
	}

#ifdef MAP_R3
	if (sta->num_tid) {
		tid_arr = cJSON_AddArrayToObject(sa_obj, "TIDQueueSizes");
		for (i = 0; i < sta->num_tid; i++) {
			tid_obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(tid_obj, "TID",  sta->tid[i]);
			cJSON_AddNumberToObject(tid_obj, "TID",  sta->tid_queue_size[i]);
			cJSON_AddItemToArray(tid_arr, tid_obj);
		}
	}
#endif

	return sa_obj;

fail:
	sa_obj = cJSON_CreateNull();

	return sa_obj;
}


cJSON * de_create_radio_bss_obj(struct own_1905_device *ctx, struct bss_info_db *bss)
{
	cJSON *bss_obj = NULL;
	cJSON *sta_arr = NULL;
	cJSON *sta_obj = NULL;
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *dev = NULL;
	unsigned char *base64_str = NULL;
	struct associated_clients *cli = NULL, *t_cli = NULL;
	struct os_reltime cur_time;
	char bssid_str[32] = {0};
	unsigned char esp_str[3] = {0};
	char timestamp_str[64] = {0};

	if (!bss) {
		err(DE_PREX"invalid parameters!!! bss(%p) NULL", bss);
		goto fail;
	}

	radio = bss->radio;
	if (!radio) {
		err(DE_PREX"can't get radio from bss("MACSTR")", MAC2STR(bss->bssid));
		goto fail;
	}

	dev = radio->parent_1905;
	if (!dev) {
		err(DE_PREX"can't get dev from bss("MACSTR")", MAC2STR(bss->bssid));
		goto fail;
	}

	bss_obj = cJSON_CreateObject();
	snprintf(bssid_str, sizeof(bssid_str), MACSTR, MAC2STR(bss->bssid));
	cJSON_AddStringToObject(bss_obj, "BSSID", bssid_str);
	cJSON_AddStringToObject(bss_obj, "SSID", (char *)bss->ssid);
	cJSON_AddTrueToObject(bss_obj, "Enabled");

	os_get_reltime(&cur_time);
	cJSON_AddNumberToObject(bss_obj, "LastChange", difftime(cur_time.sec, bss->first_ts.sec));

	get_timestamp_str(timestamp_str, sizeof(timestamp_str));
	cJSON_AddStringToObject(bss_obj, "TimeStamp", timestamp_str);
#ifdef MAP_R2
	cJSON_AddNumberToObject(bss_obj, "UnicastBytesSent", bss->uc_tx);
	cJSON_AddNumberToObject(bss_obj, "UnicastBytesReceived", bss->uc_rx);
	cJSON_AddNumberToObject(bss_obj, "MulticastBytesSent", bss->mc_tx);
	cJSON_AddNumberToObject(bss_obj, "MulticastBytesReceived", bss->mc_rx);
	cJSON_AddNumberToObject(bss_obj, "BroadcastBytesSent", bss->bc_tx);
	cJSON_AddNumberToObject(bss_obj, "BroadcastBytesReceived", bss->bc_rx);
	cJSON_AddNumberToObject(bss_obj, "ByteCounterUnits", dev->byte_cnt_unit);
	cJSON_AddNumberToObject(bss_obj, "AssociationAllowanceStatus", bss->status);
	cJSON_AddFalseToObject(bss_obj, "R1disallowed");
	cJSON_AddFalseToObject(bss_obj, "R2disallowed");
	/*not implement*/
	cJSON_AddFalseToObject(bss_obj, "Profile1bSTAsDisallowed");
	cJSON_AddFalseToObject(bss_obj, "Profile2bSTAsDisallowed");
#endif

	/*BK case*/
	if (get_bss_esp_str(bss, 1, esp_str, 3)) {
		base64_str = base64_encode(esp_str, 3, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(bss_obj, "EstServiceParametersBE", (char *)base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(bss_obj, "EstServiceParametersBE");
		}
	} else {
		cJSON_AddNullToObject(bss_obj, "EstServiceParametersBE");
	}

	/*BEcase*/
	if (get_bss_esp_str(bss, 0, esp_str, 3)) {
		base64_str = base64_encode(esp_str, 3, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(bss_obj, "EstServiceParametersBK", (char *)base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(bss_obj, "EstServiceParametersBK");
		}
	} else {
		cJSON_AddNullToObject(bss_obj, "EstServiceParametersBK");
	}

	/*VI case*/
	if (get_bss_esp_str(bss, 2, esp_str, 3)) {
		base64_str = base64_encode(esp_str, 3, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(bss_obj, "EstServiceParametersVI", (char *)base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(bss_obj, "EstServiceParametersVI");
		}
	} else {
		cJSON_AddNullToObject(bss_obj, "EstServiceParametersVI");
	}

	/*VO case*/
	if (get_bss_esp_str(bss, 3, esp_str, 3)) {
		base64_str = base64_encode(esp_str, 3, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(bss_obj, "EstServiceParametersVO", (char *)base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(bss_obj, "EstServiceParametersVO");
		}
	} else {
		cJSON_AddNullToObject(bss_obj, "EstServiceParametersVO");
	}

//#ifdef DATA_ELEMENT
#ifdef MAP_R3
	if (bss->backhual_in_use) {
		cJSON_AddTrueToObject(bss_obj, "BackhaulUse");
	} else {
		cJSON_AddFalseToObject(bss_obj, "BackhaulUse");
	}

	if (bss->fronthaul_in_use) {
		cJSON_AddTrueToObject(bss_obj, "FronthaulUse");
	} else {
		cJSON_AddFalseToObject(bss_obj, "FronthaulUse");
	}

	if (bss->r1_disallowed) {
		cJSON_AddTrueToObject(bss_obj, "R1disallowed");
	} else {
		cJSON_AddFalseToObject(bss_obj, "R1disallowed");
	}
	if (bss->r2_disallowed) {
		cJSON_AddTrueToObject(bss_obj, "R2disallowed");
	} else {
		cJSON_AddFalseToObject(bss_obj, "R2disallowed");
	}
	if (bss->multi_bssid) {
		cJSON_AddTrueToObject(bss_obj, "MultiBSSID");
	} else {
		cJSON_AddFalseToObject(bss_obj, "MultiBSSID");
	}
	if (bss->trans_bssid) {
		cJSON_AddTrueToObject(bss_obj, "TransmittedBSSID");
	} else {
		cJSON_AddFalseToObject(bss_obj, "TransmittedBSSID");
	}
#endif

	cJSON_AddNumberToObject(bss_obj, "NumberOfSTA", bss->assoc_sta_cnt);
	if (bss->assoc_sta_cnt) {
		sta_arr = cJSON_AddArrayToObject(bss_obj, "STAList");
		SLIST_FOREACH_SAFE(cli, &dev->assoc_clients, next_client, t_cli)
		{
			if (cli->bss == bss) {
				sta_obj = de_create_station_obj(ctx, cli);
				cJSON_AddItemToArray(sta_arr, sta_obj);
			}
		}
	} else {
		cJSON_AddArrayToObject(bss_obj, "STAList");
	}

	return bss_obj;

fail:
	bss_obj = cJSON_CreateNull();
	return bss_obj;
}

#ifdef MAP_R3_WF6
int de_create_wifi6_cap_obj(cJSON *wf6_obj, struct ap_wf6_caps *wf6_cap)
{
	char *base64_str = NULL;
	char he_mcs_len = 0;

	if (!wf6_obj || !wf6_cap) {
		err("invalid param!! wf6_obj(%p) or wf6_cap(%p) NULL\n", wf6_obj, wf6_cap);
		return -1;
	}

	if (wf6_cap->he_160)
		cJSON_AddTrueToObject(wf6_obj, "HE160");
	else
		cJSON_AddFalseToObject(wf6_obj, "HE160");

	if (wf6_cap->he_8080)
		cJSON_AddTrueToObject(wf6_obj, "HE8080");
	else
		cJSON_AddFalseToObject(wf6_obj, "HE8080");

	he_mcs_len = wf6_cap->he_mcs_len > MAX_HE_MCS_LEN ? MAX_HE_MCS_LEN : wf6_cap->he_mcs_len;
	base64_str = (char *)base64_encode(wf6_cap->he_mcs, he_mcs_len, NULL);
	if (base64_str) {
		cJSON_AddStringToObject(wf6_obj, "MCSNSS", base64_str);
		os_free(base64_str);
		base64_str = NULL;
	} else {
		cJSON_AddNullToObject(wf6_obj, "MCSNSS");
	}

	if (wf6_cap->su_bf_cap)
			cJSON_AddTrueToObject(wf6_obj, "SUBeamformer");
		else
			cJSON_AddFalseToObject(wf6_obj, "SUBeamformer");

	if (wf6_cap->mu_bf_cap)
		cJSON_AddTrueToObject(wf6_obj, "MUBeamformer");
	else
		cJSON_AddFalseToObject(wf6_obj, "MUBeamformer");

	if (wf6_cap->su_beamformee_status)
		cJSON_AddTrueToObject(wf6_obj, "SUBeamformee");
	else
		cJSON_AddFalseToObject(wf6_obj, "SUBeamformee");

	if (wf6_cap->beamformee_sts_less80)
		cJSON_AddTrueToObject(wf6_obj, "Beamformee80orLess");
	else
		cJSON_AddFalseToObject(wf6_obj, "Beamformee80orLess");

	if (wf6_cap->beamformee_sts_more80)
		cJSON_AddTrueToObject(wf6_obj, "BeamformeeAbove80");
	else
		cJSON_AddFalseToObject(wf6_obj, "BeamformeeAbove80");

	if (wf6_cap->ul_mu_mimo_cap)
		cJSON_AddTrueToObject(wf6_obj, "ULMUMIMO");
	else
		cJSON_AddFalseToObject(wf6_obj, "ULMUMIMO");

	if (wf6_cap->ul_ofdma_cap)
		cJSON_AddTrueToObject(wf6_obj, "ULOFDMA");
	else
		cJSON_AddFalseToObject(wf6_obj, "ULOFDMA");

	if (wf6_cap->dl_ofdma_cap)
		cJSON_AddTrueToObject(wf6_obj, "DLOFDMA");
	else
		cJSON_AddFalseToObject(wf6_obj, "DLOFDMA");

	cJSON_AddNumberToObject(wf6_obj, "MaxDLMUMIMO", wf6_cap->max_user_dl_tx_mu_mimo);
	cJSON_AddNumberToObject(wf6_obj, "MaxULMUMIMO", wf6_cap->max_user_ul_rx_mu_mimo);
	cJSON_AddNumberToObject(wf6_obj, "MaxDLOFDMA", wf6_cap->max_user_dl_tx_ofdma);
	cJSON_AddNumberToObject(wf6_obj, "MaxULOFDMA", wf6_cap->max_user_ul_rx_ofdma);

	if (wf6_cap->rts_status)
		cJSON_AddTrueToObject(wf6_obj, "RTS");
	else
		cJSON_AddFalseToObject(wf6_obj, "RTS");

	if (wf6_cap->mu_rts_status)
		cJSON_AddTrueToObject(wf6_obj, "MURTS");
	else
		cJSON_AddFalseToObject(wf6_obj, "MURTS");

	if (wf6_cap->m_bssid_status)
		cJSON_AddTrueToObject(wf6_obj, "MultiBSSID");
	else
		cJSON_AddFalseToObject(wf6_obj, "MultiBSSID");

	if (wf6_cap->mu_edca_status)
		cJSON_AddTrueToObject(wf6_obj, "MUEDCA");
	else
		cJSON_AddFalseToObject(wf6_obj, "MUEDCA");

	if (wf6_cap->twt_requester_status)
		cJSON_AddTrueToObject(wf6_obj, "TWTRequestor");
	else
		cJSON_AddFalseToObject(wf6_obj, "TWTRequestor");

	if (wf6_cap->twt_responder_status)
		cJSON_AddTrueToObject(wf6_obj, "TWTResponder");
	else
		cJSON_AddFalseToObject(wf6_obj, "TWTResponder");

	return 0;
}
#endif

int de_create_capabilities_obj(struct _1905_map_device *dev, struct radio_info_db *ra, cJSON *cap_obj)
{
#ifdef MAP_R3_WF6
	cJSON *wf6_obj = NULL;
#endif
	cJSON *opclass_arr = NULL;
	cJSON *opclass_obj = NULL;
	cJSON *non_sup_ch_arr = NULL;
	cJSON *non_sup_ch_num = NULL;
#ifdef MAP_R3
	cJSON *fh_akm_suit_arr = NULL;
	cJSON *fh_akm_suit_obj = NULL;
	cJSON *bh_akm_suit_arr = NULL;
	cJSON *bh_akm_suit_obj = NULL;
#endif
	char *base64_str = NULL;
	struct ht_caps *pht = NULL;
	struct vht_caps *pvht = NULL;
	struct he_caps *phe = NULL;
#ifdef MAP_R3_WF6
	struct ap_wf6_roles *pwf6_role = NULL;
	struct ap_wf6_caps *pwf6_cap = NULL;
#endif
	char *pos = NULL;
	struct basic_cap_db *cap = NULL, *t_cap = NULL;
#ifdef MAP_R3
	struct akm_suit_db* akm_suit = NULL, *t_akm_suit = NULL;
#endif
	char tmp_buf[32] = {0};
#ifdef MAP_R3
	char oui_str[4] = {0};
#endif
	unsigned char i = 0;
	char he_mcs_len = 0;
#ifdef MAP_R3_WF6
	char wf6_ap_role_set = 0, wf6_sta_role_set = 0;
#endif


	if (!dev || !ra || !cap_obj) {
		err("invalid param!! dev(%p) ra(%p) cap_obj(%p) NULL\n", dev, ra, cap_obj);
		return -1;
	}

	pht = &ra->radio_capability.ht_cap;
	if (pht->valid) {
		tmp_buf[0] = (pht->tx_stream << 6) | (pht->rx_stream << 4) |
				(pht->sgi_20 << 3) | (pht->sgi_40 << 2) | (pht->ht_40 << 1);
		base64_str = (char *)base64_encode((unsigned char *)tmp_buf, 1, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(cap_obj, "HTCapabilities", base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(cap_obj, "HTCapabilities");
		}
	} else {
		cJSON_AddNullToObject(cap_obj, "HTCapabilities");
	}

	pvht = &ra->radio_capability.vht_cap;
	if (pvht->valid) {
		pos = tmp_buf;
		os_memcpy(pos, (char *)&pvht->vht_tx_mcs, 2);
		pos += 2;
		os_memcpy(pos, (char *)&pvht->vht_rx_mcs, 2);
		pos += 2;
		*pos++ =  (pvht->tx_stream << 5) | (pvht->rx_stream << 2) | (pvht->sgi_80 << 1) | pvht->sgi_160;
		*pos++ = (pvht->vht_8080 << 7) | (pvht->vht_160 << 6) | (pvht->su_beamformer << 5)
			| (pvht->mu_beamformer << 4);
		base64_str = (char *)base64_encode((unsigned char *)tmp_buf, 6, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(cap_obj, "VHTCapabilities", base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(cap_obj, "VHTCapabilities");
		}
	} else {
		cJSON_AddNullToObject(cap_obj, "VHTCapabilities");
	}

	phe = &ra->radio_capability.he_cap;
	if (phe->valid) {
		os_memset(tmp_buf, 0, sizeof(tmp_buf));
		pos = tmp_buf;
		he_mcs_len = phe->he_mcs_len > MAX_HE_MCS_LEN ? MAX_HE_MCS_LEN :  phe->he_mcs_len;
		*pos++ = he_mcs_len;
		os_memcpy(pos, (char *)phe->he_mcs, he_mcs_len);
		pos += he_mcs_len;
		*pos++ =  (phe->tx_spatial_stream << 5) | (phe->rx_spatial_streams << 2) |
			(phe->he_80plus80 << 1) | phe->he_160;
		*pos++ = (phe->su_beamformer << 7) | (phe->mu_beamformer << 6) |
			(phe->ul_mi_mimo << 5) | (phe->ul_ofdma_plus_mu_mimo << 4) |
			(phe->dl_ofdma_plus_mu_mimo << 3) | (phe->ul_ofdma_supported << 2) |
			(phe->dl_ofdma_supported << 1);
		base64_str = (char *)base64_encode((u8 *)tmp_buf, he_mcs_len + 3, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(cap_obj, "HECapabilities", base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(cap_obj, "HECapabilities");
		}
	} else {
		cJSON_AddNullToObject(cap_obj, "HECapabilities");
	}
	/*wifi 6 cap belongs to R3; not implement*/
#ifdef MAP_R3_WF6
	pwf6_role = &ra->radio_capability.wf6_cap;
	if (pwf6_role->valid) {
		for (i = 0; i < pwf6_role->role_supp; i++) {
			pwf6_cap = &pwf6_role->wf6_role[i];
			if (pwf6_cap->agent_role == 0 && !wf6_ap_role_set) {
				wf6_ap_role_set = 1;
				wf6_obj = cJSON_AddObjectToObject(cap_obj, "WiFi6APRole");
				de_create_wifi6_cap_obj(wf6_obj, pwf6_cap);
			} else if (pwf6_cap->agent_role == 1 && !wf6_sta_role_set) {
				wf6_sta_role_set = 1;
				wf6_obj = cJSON_AddObjectToObject(cap_obj, "WiFi6bSTARole");
				de_create_wifi6_cap_obj(wf6_obj, pwf6_cap);
			}
		}
	}
	if (!wf6_ap_role_set) {
		cJSON_AddNullToObject(cap_obj, "WiFi6APRole");
	}
	if (!wf6_sta_role_set) {
		cJSON_AddNullToObject(cap_obj, "WiFi6STARole");
	}
#endif
#ifdef MAP_R3
	if (SLIST_EMPTY(&dev->fh_akm_suit_head)) {
		cJSON_AddNullToObject(cap_obj, "AKMFrontHaul");
	} else {
		fh_akm_suit_arr = cJSON_AddArrayToObject(cap_obj, "AKMFrontHaul");
		SLIST_FOREACH_SAFE(akm_suit, &dev->fh_akm_suit_head, akm_suit_entry, t_akm_suit)
		{
			fh_akm_suit_obj = cJSON_CreateObject();
			os_memcpy(oui_str, akm_suit->oui, 3);
			cJSON_AddStringToObject(fh_akm_suit_obj, "OUI", oui_str);
			cJSON_AddNumberToObject(fh_akm_suit_obj, "Type", akm_suit->type);
			cJSON_AddItemToArray(fh_akm_suit_arr, fh_akm_suit_obj);
		}
	}

	if (SLIST_EMPTY(&dev->bh_akm_suit_head)) {
		cJSON_AddNullToObject(cap_obj, "AKMBackHaul");
	} else {
		bh_akm_suit_arr = cJSON_AddArrayToObject(cap_obj, "AKMBackHaul");
		t_akm_suit = NULL;
		SLIST_FOREACH_SAFE(akm_suit, &dev->bh_akm_suit_head, akm_suit_entry, t_akm_suit)
		{
			bh_akm_suit_obj = cJSON_CreateObject();
			os_memcpy(oui_str, akm_suit->oui, 3);
			cJSON_AddStringToObject(bh_akm_suit_obj, "OUI", oui_str);
			cJSON_AddNumberToObject(bh_akm_suit_obj, "Type", akm_suit->type);
			cJSON_AddItemToArray(bh_akm_suit_arr, bh_akm_suit_obj);
		}
	}
#else
	cJSON_AddNullToObject(cap_obj, "AKMFrontHaul");
	cJSON_AddNullToObject(cap_obj, "AKMBackHaul");
#endif

	cJSON_AddNumberToObject(cap_obj, "NumberOfOpClass", ra->radio_capability.basic_caps.op_class_num);
	if (ra->radio_capability.basic_caps.op_class_num) {
		opclass_arr = cJSON_AddArrayToObject(cap_obj, "OperatingClasses");
		SLIST_FOREACH_SAFE(cap, &ra->radio_capability.basic_caps.bcap_head, basic_cap_entry, t_cap)
		{
			opclass_obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(opclass_obj, "Class", cap->op_class);
			cJSON_AddNumberToObject(opclass_obj, "MaxTxPower", cap->max_tx_pwr);
			cJSON_AddNumberToObject(opclass_obj, "NumberOfNonOperChan", cap->non_operch_num);
			non_sup_ch_arr = cJSON_AddArrayToObject(opclass_obj, "NonOperable");
			if (cap->non_operch_num) {
				for (i = 0; i < cap->non_operch_num; i++) {
					non_sup_ch_num = cJSON_CreateNumber(cap->non_operch_list[i]);
					cJSON_AddItemToArray(non_sup_ch_arr, non_sup_ch_num);
				}
			}
			cJSON_AddItemToArray(opclass_arr, opclass_obj);
		}
	}

	return 0;
}


cJSON * de_create_radio_ch_scan_neighbor_obj(struct nb_info *neighbor)
{
	cJSON *neighbor_obj = NULL;
	char bssid_str[32] = {0};
	char bw_str[16] = {0};
	unsigned char rssi = 0;

	neighbor_obj = cJSON_CreateObject();

	snprintf(bssid_str, sizeof(bssid_str), MACSTR, MAC2STR(neighbor->bssid));
	cJSON_AddStringToObject(neighbor_obj, "BSSID", bssid_str);
	cJSON_AddStringToObject(neighbor_obj, "SSID", (char *)neighbor->ssid);
	rssi = (unsigned char)rcpi_to_rssi(neighbor->RCPI);
	if (rssi > 220)
		rssi = 220;
	cJSON_AddNumberToObject(neighbor_obj, "SignalStrength", rssi);
	snprintf(bw_str, sizeof(bw_str), "%s", neighbor->ch_bw);
	cJSON_AddStringToObject(neighbor_obj, "ChannelBandwidth", bw_str);
	cJSON_AddNumberToObject(neighbor_obj, "ChannelUtilization", neighbor->cu);
	cJSON_AddNumberToObject(neighbor_obj, "StationCount", neighbor->sta_cnt);

	return neighbor_obj;
}


cJSON * de_create_radio_ch_scan_obj(struct radio_info_db *ra, unsigned char op_class, unsigned char channel)
{
	cJSON *ch_scan_obj = NULL;
	cJSON *neighbor_arr = NULL;
	cJSON *neighbor_obj = NULL;
	struct scan_result_tlv *res = NULL, *t_res = NULL;
	struct nb_info *neighbor = NULL, *t_neighbor = NULL;
	char timestamp_str[32] = {0};

	SLIST_FOREACH_SAFE(res, &ra->first_scan_result, next_scan_result, t_res)
	{
		if (res->oper_class != op_class || res->channel != channel)
			continue;
		ch_scan_obj = cJSON_CreateObject();
		cJSON_AddNumberToObject(ch_scan_obj, "Channel", channel);

		snprintf(timestamp_str, sizeof(timestamp_str), "%s", res->timestamp);
		cJSON_AddStringToObject(ch_scan_obj, "TimeStamp", timestamp_str);
		cJSON_AddNumberToObject(ch_scan_obj, "Utilization", res->utilization);
		cJSON_AddNumberToObject(ch_scan_obj, "Noise", res->noise);
		cJSON_AddNumberToObject(ch_scan_obj, "NumberOfNeighbors", res->neighbor_num);
		if (res->neighbor_num) {
			neighbor_arr = cJSON_AddArrayToObject(ch_scan_obj, "NeighborList");
			SLIST_FOREACH_SAFE(neighbor, &res->first_neighbor_info, next_neighbor_info, t_neighbor)
			{
				neighbor_obj = de_create_radio_ch_scan_neighbor_obj(neighbor);
				cJSON_AddItemToArray(neighbor_arr, neighbor_obj);
			}
		} else {
			cJSON_AddArrayToObject(ch_scan_obj, "NeighborList");
		}
		return ch_scan_obj;
	}

	ch_scan_obj = cJSON_CreateNull();
	return ch_scan_obj;
}

cJSON * de_create_radio_op_class_scan_obj(struct radio_info_db *ra, unsigned char op_class)
{
	cJSON *op_class_scan_obj = NULL;
	cJSON *ch_scan_arr = NULL;
	cJSON *ch_scan_obj = NULL;
	unsigned char *ch_arr = NULL;
	unsigned char num_ch = 0;
	unsigned char i = 0;

	op_class_scan_obj = cJSON_CreateObject();
	num_ch = get_radio_scan_channel_count(ra, op_class, &ch_arr);
	cJSON_AddNumberToObject(op_class_scan_obj, "OperatingClass", op_class);
	cJSON_AddNumberToObject(op_class_scan_obj, "NumberOfChannelScans", num_ch);
	if (num_ch) {
		ch_scan_arr = cJSON_AddArrayToObject(op_class_scan_obj, "ChannelScanList");
		for (i = 0; i < num_ch; i++) {
			ch_scan_obj = de_create_radio_ch_scan_obj(ra, op_class, ch_arr[i]);
			cJSON_AddItemToArray(ch_scan_arr, ch_scan_obj);
		}
	}

	if (ch_arr) {
		os_free(ch_arr);
		ch_arr = NULL;
	}

	return op_class_scan_obj;
}

int de_create_radio_scan_result_obj(struct radio_info_db *ra, cJSON *scan_result_obj)
{
	cJSON *op_class_scan_arr = NULL;
	cJSON *op_class_scan_obj = NULL;
	unsigned char *op_class_arr = NULL;
	struct scan_result_tlv *scan_result = NULL;
	unsigned char num_op_class = 0;
	unsigned char i = 0;
	char timestamp_str[32] = {0};

	num_op_class = get_radio_scan_op_class_count(ra, &op_class_arr);
	cJSON_AddNumberToObject(scan_result_obj, "NumberOfOpClassScans", num_op_class);

	scan_result = SLIST_FIRST(&ra->first_scan_result);
	if (scan_result) {
		os_snprintf(timestamp_str, sizeof(timestamp_str), "%s", scan_result->timestamp);
		cJSON_AddStringToObject(scan_result_obj, "TimeStamp", timestamp_str);
	} else {
		cJSON_AddStringToObject(scan_result_obj, "TimeStamp", "unknown");
	}
	if (num_op_class) {
		op_class_scan_arr = cJSON_AddArrayToObject(scan_result_obj, "OpClassScanList");
		for (i = 0; i < num_op_class; i++) {
			op_class_scan_obj = de_create_radio_op_class_scan_obj(ra, op_class_arr[i]);
			cJSON_AddItemToArray(op_class_scan_arr, op_class_scan_obj);
		}
	}

	if (op_class_arr) {
		os_free(op_class_arr);
		op_class_arr = NULL;
	}

	return 0;
}



cJSON * de_create_radio_cur_op_class_obj(struct radio_info_db *ra)
{
	cJSON *cur_op_class_obj = NULL;
	char timestamp_str[64] = {0};

	cur_op_class_obj = cJSON_CreateObject();

	get_timestamp_str(timestamp_str, sizeof(timestamp_str));
	cJSON_AddStringToObject(cur_op_class_obj, "TimeStamp", timestamp_str);
	cJSON_AddNumberToObject(cur_op_class_obj, "Class", ra->operating_class);
	cJSON_AddNumberToObject(cur_op_class_obj, "Channel", ra->channel[0]);
	cJSON_AddNumberToObject(cur_op_class_obj, "TxPower", ra->power);

	return cur_op_class_obj;
}

cJSON * de_create_radio_obj(struct own_1905_device *ctx, struct radio_info_db *ra)
{
	cJSON *radio_obj = NULL;
	cJSON *cur_op_class_arr = NULL;
	cJSON *cur_op_class_obj = NULL;
	cJSON *unassociated_sta_arr = NULL;
	cJSON *unassociated_sta_obj = NULL;
	cJSON *cap_obj = NULL;
	cJSON *backhaul_sta_obj = NULL;
	cJSON *scan_result_obj = NULL;
	cJSON *scan_cap_obj = NULL;
	cJSON *scan_cap_opclass_arr = NULL;
	cJSON *scan_cap_opclass_obj = NULL;
	cJSON *scan_cap_ch_arr = NULL;
	cJSON *scan_cap_ch = NULL;
	cJSON *cac_cap_arr = NULL;
	cJSON *cac_method_obj = NULL;
	cJSON *cac_cap_opclass_arr = NULL;
	cJSON *cac_cap_opclass_obj = NULL;
	cJSON *cac_cap_ch_arr = NULL;
	cJSON *cac_cap_ch = NULL;
	cJSON *bss_arr = NULL;
	cJSON *bss_obj = NULL;
	struct _1905_map_device *dev = NULL;
	unsigned char *base64_str = NULL;
	struct radio_policy_db *policy = NULL, *t_policy = NULL;
	struct metric_policy_db *mpolicy = NULL, *t_mpolicy = NULL;
	struct bss_info_db *bss = NULL, *t_bss = NULL;
	struct _1905_map_device *own_dev = NULL;
	struct client *cli = NULL;
	struct mapd_radio_info *radio_info = NULL;
	struct iface_info *iface = NULL, *t_iface = NULL;
	struct channel_body *ch_body = NULL;
	struct cac_cap_db *cac_cap = NULL, *t_cac_cap = NULL;
	struct cac_opcap_db *opcap = NULL, *t_opcap = NULL;
	struct ts_cap_db *ts_cap = NULL, *t_ts_cap = NULL;
	unsigned int cac_interval = 0;
	unsigned short num_sta = 0;
	unsigned short num_bss = 0;
	char mac_str[32] = {0};
#ifdef MAP_R3_DE
	char de_info_str[DE_MAX_LEN + 1] = {0};
#endif

	unsigned char radio_idx = 0;
	unsigned char i = 0, j = 0, k = 0;

	if (!ctx || !ra) {
		err(DE_PREX"invalid parameters!!! ctx(%p) or ra(%p) NULL", ctx, ra);
		goto fail;
	}

	own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	if (!own_dev) {
		err(DE_PREX"get own 1905.1 device fail");
		goto fail;
	}

	dev = ra->parent_1905;
	if (!dev || !dev->in_network) {
		err(DE_PREX"get parent_1905 dev fail");
		goto fail;
	}

	/*find own radio if dev is own_dev*/
	if (own_dev == dev) {
		for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
			radio_info = &ctx->dev_radio_info[radio_idx];
			if (radio_info->radio_idx != (uint8_t)-1 &&
				!os_memcmp(radio_info->identifier, ra->identifier, ETH_ALEN))
				break;
		}
		if (radio_idx >= MAX_NUM_OF_RADIO) {
			err(DE_PREX"can't find own radio_info");
			goto fail;
		}
	}

	radio_obj = cJSON_CreateObject();
	base64_str = base64_encode(ra->identifier, ETH_ALEN, NULL);
	if (base64_str) {
		cJSON_AddStringToObject(radio_obj, "ID", (char *)base64_str);
		os_free(base64_str);
		base64_str = NULL;
	} else {
		cJSON_AddNullToObject(radio_obj, "ID");
	}

	cJSON_AddTrueToObject(radio_obj, "Enabled");
#ifdef MAP_R3_DE
	for (i = 0; i < MAX_RUID; i++) {
		if (!os_memcmp(dev->de.ruid[i].identifier, ra->identifier, ETH_ALEN)) {
			snprintf(de_info_str, sizeof(de_info_str), "%s", dev->de.ruid[i].chip_ven);
			cJSON_AddStringToObject(radio_obj, "ChipsetVendor", de_info_str);
			break;
		}
	}
	if (i >= MAX_RUID) {
		cJSON_AddStringToObject(radio_obj, "ChipsetVendor", "unknown");
	}
#else
	cJSON_AddStringToObject(radio_obj, "ChipsetVendor", "unknown");
#endif
	cJSON_AddNumberToObject(radio_obj, "NumberOfCurrOpClass", 1);
	cur_op_class_arr = cJSON_AddArrayToObject(radio_obj, "CurrentOperatingClasses");
	cur_op_class_obj = de_create_radio_cur_op_class_obj(ra);
	cJSON_AddItemToArray(cur_op_class_arr, cur_op_class_obj);

#ifdef MAP_R2
	bss = topo_srv_get_next_bss(dev, NULL);
	if (bss)
		cJSON_AddNumberToObject(radio_obj, "Utilization", bss->ch_util);
	else
		cJSON_AddNumberToObject(radio_obj, "Utilization", 0);
#endif

	/*unassoc sta; only can get info on controller*/
	if (own_dev == dev) {
#ifdef MAP_R2
		cJSON_AddNumberToObject(radio_obj, "Noise", radio_info->radio_metrics.cu_noise);
		cJSON_AddNumberToObject(radio_obj, "Transmit", radio_info->radio_metrics.cu_tx);
		cJSON_AddNumberToObject(radio_obj, "ReceiveSelf", radio_info->radio_metrics.cu_rx);
		cJSON_AddNumberToObject(radio_obj, "ReceiveOther", radio_info->radio_metrics.cu_other);
		/*traffic separation*/
		cJSON_AddTrueToObject(radio_obj, "TrafficSeparationCombinedFronthaul");
		cJSON_AddTrueToObject(radio_obj, "TrafficSeparationCombinedBackhaul");
#endif
		/*steer policy*/
		SLIST_FOREACH_SAFE(policy, &ctx->map_policy.spolicy.radio_policy_head, radio_policy_entry, t_policy)
		{
			if (!os_memcmp(policy->identifier, ra->identifier, ETH_ALEN) == 0) {
				break;
			}
		}
		if (policy) {
			cJSON_AddNumberToObject(radio_obj, "SteeringPolicy", policy->steer_policy);
			cJSON_AddNumberToObject(radio_obj, "ChannelUtilizationThreshold", policy->ch_util_thres);
			cJSON_AddNumberToObject(radio_obj, "RCPISteeringThreshold", rssi_to_rcpi((char)policy->rssi_thres));
		}

		/*metrics policy*/
		SLIST_FOREACH_SAFE(mpolicy, &ctx->map_policy.mpolicy.policy_head, policy_entry, t_mpolicy)
		{
			if (!os_memcmp(mpolicy->identifier, ra->identifier, ETH_ALEN) == 0) {
				break;
			}
		}
		if (mpolicy) {
			cJSON_AddNumberToObject(radio_obj, "STAReportingRCPIThreshold", mpolicy->rssi_thres);
			cJSON_AddNumberToObject(radio_obj, "STAReportingRCPIHysteresisMarginOverride", mpolicy->hysteresis_margin);
			cJSON_AddNumberToObject(radio_obj, "ChannelUtilizationReportingThreshold", mpolicy->ch_util_thres);
			if (mpolicy->sta_stats_inclusion)
				cJSON_AddTrueToObject(radio_obj, "AssociatedSTATrafficStatsInclusionPolicy");
			else
				cJSON_AddFalseToObject(radio_obj, "AssociatedSTATrafficStatsInclusionPolicy");

			if (mpolicy->sta_metrics_inclusion)
				cJSON_AddTrueToObject(radio_obj, "AssociatedSTALinkMetricsInclusionPolicy");
			else
				cJSON_AddFalseToObject(radio_obj, "AssociatedSTALinkMetricsInclusionPolicy");
		}

		num_sta = get_cont_radio_unassociated_sta_count(ctx, radio_info);
		cJSON_AddNumberToObject(radio_obj, "NumberOfUnassocSta", num_sta);
		if (num_sta) {
			unassociated_sta_arr = cJSON_AddArrayToObject(radio_obj, "UnassociatedStaList");
			dl_list_for_each(cli, &ctx->sta_seen_list, struct client, sta_seen_entry)
			{
				if (!is_zero_ether_addr(cli->bssid))
					continue;

				if (cli->ul_rssi[radio_info->radio_idx]) {
					unassociated_sta_obj = cJSON_CreateObject();
					snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(cli->mac_addr));
					cJSON_AddStringToObject(unassociated_sta_obj, "MACAddress", mac_str);
					cJSON_AddNumberToObject(unassociated_sta_obj, "SignalStrength",
						(unsigned char)cli->ul_rssi[radio_info->radio_idx]);
					cJSON_AddItemToArray(unassociated_sta_arr, unassociated_sta_obj);
				}
			}
		} else {
			cJSON_AddNullToObject(radio_obj, "UnassociatedStaList");
		}
	} else {
#ifdef MAP_R2
		cJSON_AddNumberToObject(radio_obj, "Noise", ra->radio_metrics.cu_noise);
		cJSON_AddNumberToObject(radio_obj, "Transmit", ra->radio_metrics.cu_tx);
		cJSON_AddNumberToObject(radio_obj, "ReceiveSelf", ra->radio_metrics.cu_rx);
		cJSON_AddNumberToObject(radio_obj, "ReceiveOther", ra->radio_metrics.cu_other);

		SLIST_FOREACH_SAFE(ts_cap, &dev->ts_cap_head, ts_cap_entry, t_ts_cap)
		{
			if (!os_memcmp(ts_cap->identifier, ra->identifier, ETH_ALEN)) {
				break;
			}
		}
		if (ts_cap) {
			if (ts_cap->ts_combined_fh)
				cJSON_AddTrueToObject(radio_obj, "TrafficSeparationCombinedFronthaul");
			else
				cJSON_AddFalseToObject(radio_obj, "TrafficSeparationCombinedFronthaul");

			if (ts_cap->ts_combined_bh)
				cJSON_AddTrueToObject(radio_obj, "TrafficSeparationCombinedBackhaul");
			else
				cJSON_AddFalseToObject(radio_obj, "TrafficSeparationCombinedBackhaul");
		}
#endif

		/*steer policy*/
		t_policy = NULL;
		SLIST_FOREACH_SAFE(policy, &dev->policy_record.spolicy.radio_policy_head, radio_policy_entry, t_policy)
		{
			if (!os_memcmp(policy->identifier, ra->identifier, ETH_ALEN) == 0) {
				break;
			}
		}
		if (policy) {
			cJSON_AddNumberToObject(radio_obj, "SteeringPolicy", policy->steer_policy);
			cJSON_AddNumberToObject(radio_obj, "ChannelUtilizationThreshold", policy->ch_util_thres);
			cJSON_AddNumberToObject(radio_obj, "RCPISteeringThreshold", rssi_to_rcpi(policy->rssi_thres));
		}

		/*metrics policy*/
		t_mpolicy = NULL;
		SLIST_FOREACH_SAFE(mpolicy, &dev->policy_record.mpolicy.policy_head, policy_entry, t_mpolicy)
		{
			if (!os_memcmp(mpolicy->identifier, ra->identifier, ETH_ALEN) == 0) {
				break;
			}
		}
		if (mpolicy) {
			cJSON_AddNumberToObject(radio_obj, "STAReportingRCPIThreshold", mpolicy->rssi_thres);
			cJSON_AddNumberToObject(radio_obj, "STAReportingRCPIHysteresisMarginOverride", mpolicy->hysteresis_margin);
			cJSON_AddNumberToObject(radio_obj, "ChannelUtilizationReportingThreshold", mpolicy->ch_util_thres);
			if (mpolicy->sta_stats_inclusion)
				cJSON_AddTrueToObject(radio_obj, "AssociatedSTATrafficStatsInclusionPolicy");
			else
				cJSON_AddFalseToObject(radio_obj, "AssociatedSTATrafficStatsInclusionPolicy");

			if (mpolicy->sta_metrics_inclusion)
				cJSON_AddTrueToObject(radio_obj, "AssociatedSTALinkMetricsInclusionPolicy");
			else
				cJSON_AddFalseToObject(radio_obj, "AssociatedSTALinkMetricsInclusionPolicy");
		}
		cJSON_AddNumberToObject(radio_obj, "NumberOfUnassocSta", 0);
		cJSON_AddNullToObject(radio_obj, "UnassociatedStaList");
	}

	cap_obj = cJSON_AddObjectToObject(radio_obj, "Capabilities");
	de_create_capabilities_obj(dev, ra, cap_obj);

	/*backhaul sta; not final solution*/
	backhaul_sta_obj = cJSON_AddObjectToObject(radio_obj, "BackhaulSta");
	if (ra->uplink_bh_present) {
		SLIST_FOREACH_SAFE(iface, &dev->_1905_info.first_iface, next_iface, t_iface)
		{
			/*tmp code here*/
			if (*(((char *)(&iface->media_info)) + 6) == 0x40 &&
				(iface->media_type >= IEEE802_11_GROUP) &&
				(iface->media_type < IEEE1901_GROUP) ) {
				if (iface->radio == ra)
					break;
			}
		}
		if (iface) {
			snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(iface->iface_addr));
			cJSON_AddStringToObject(backhaul_sta_obj, "MACAddress", mac_str);
		} else {
			cJSON_AddStringToObject(backhaul_sta_obj, "MACAddress", "00:00:00:00:00:00");
		}
	} else {
		cJSON_AddStringToObject(backhaul_sta_obj, "MACAddress", "00:00:00:00:00:00");
	}

#ifdef MAP_R2
#if 1
	/*scan result*/
	if (SLIST_EMPTY(&ra->first_scan_result)) {
		cJSON_AddNullToObject(radio_obj, "ScanResultList");
	} else {
		scan_result_obj = cJSON_AddObjectToObject(radio_obj, "ScanResultList");
		de_create_radio_scan_result_obj(ra, scan_result_obj);
	}
#else
	/*tmp for certification*/
	cJSON_AddNullToObject(radio_obj, "ScanResultList");
#endif

	/*scan cap*/
	if (own_dev == dev) {
		if (!ctx->scan_capab) {
			cJSON_AddNullToObject(radio_obj, "ScanCapability");
		} else {
			for (i = 0; i < ctx->scan_capab->radio_num; i++) {
				if (!os_memcmp(ctx->scan_capab->radio_scan_params[i].radio_id,
					ra->identifier, ETH_ALEN))
					break;
			}
			if (i >= ctx->scan_capab->radio_num) {
				cJSON_AddNullToObject(radio_obj, "ScanCapability");
			} else {
				scan_cap_obj = cJSON_AddObjectToObject(radio_obj, "ScanCapability");
				if (ctx->scan_capab->radio_scan_params[i].boot_scan_only)
					cJSON_AddTrueToObject(scan_cap_obj, "OnBootOnly");
				else
					cJSON_AddFalseToObject(scan_cap_obj, "OnBootOnly");
				cJSON_AddNumberToObject(scan_cap_obj, "Impact",
					ctx->scan_capab->radio_scan_params[i].scan_impact);
				cJSON_AddNumberToObject(scan_cap_obj, "MinimumInterval",
					ctx->scan_capab->radio_scan_params[i].min_scan_interval);
				scan_cap_opclass_arr = cJSON_AddArrayToObject(scan_cap_obj, "OpClassList");
				for (j = 0; j < ctx->scan_capab->radio_scan_params[i].oper_class_num; j++) {
					ch_body = &ctx->scan_capab->radio_scan_params[i].ch_body[j];
					scan_cap_opclass_obj = cJSON_CreateObject();
					cJSON_AddNumberToObject(scan_cap_opclass_obj, "OperatingClass", ch_body->oper_class);
					scan_cap_ch_arr = cJSON_AddArrayToObject(scan_cap_opclass_obj, "ChannelList");
					for (k = 0; k < ch_body->ch_list_num; k++) {
						scan_cap_ch = cJSON_CreateNumber(ch_body->ch_list[k]);
						cJSON_AddItemToArray(scan_cap_ch_arr, scan_cap_ch);
					}
					cJSON_AddItemToArray(scan_cap_opclass_arr, scan_cap_opclass_obj);

				}
			}
		}
	} else {
		scan_cap_obj = cJSON_AddObjectToObject(radio_obj, "ScanCapability");
		if (ra->radio_scan_params.boot_scan_only)
			cJSON_AddTrueToObject(scan_cap_obj, "OnBootOnly");
		else
			cJSON_AddFalseToObject(scan_cap_obj, "OnBootOnly");
		cJSON_AddNumberToObject(scan_cap_obj, "Impact", ra->radio_scan_params.scan_impact);
		cJSON_AddNumberToObject(scan_cap_obj, "MinimumInterval", ra->radio_scan_params.min_scan_interval);
		scan_cap_opclass_arr = cJSON_AddArrayToObject(scan_cap_obj, "OpClassList");
		for (i = 0; i < ra->radio_scan_params.oper_class_num; i++) {
			ch_body = &ra->radio_scan_params.ch_body[i];
			scan_cap_opclass_obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(scan_cap_opclass_obj, "OperatingClass", ch_body->oper_class);
			scan_cap_ch_arr = cJSON_AddArrayToObject(scan_cap_opclass_obj, "ChannelList");
			for (j = 0; j < ch_body->ch_list_num; j++) {
				scan_cap_ch = cJSON_CreateNumber(ch_body->ch_list[j]);
				cJSON_AddItemToArray(scan_cap_ch_arr, scan_cap_ch);
			}
			cJSON_AddItemToArray(scan_cap_opclass_arr, scan_cap_opclass_obj);

		}
	}

	/*cac cap*/
	if (SLIST_EMPTY(&ra->cac_cap.cac_capab_head)) {
		cJSON_AddNullToObject(radio_obj, "CACCapability");
	} else {
		cac_cap_arr = cJSON_AddArrayToObject(radio_obj, "CACCapability");
		SLIST_FOREACH_SAFE(cac_cap, &ra->cac_cap.cac_capab_head, cac_cap_entry, t_cac_cap) {
			cac_method_obj = cJSON_CreateObject();
			cJSON_AddNumberToObject(cac_method_obj, "Method", cac_cap->cac_mode);
			cac_interval = cac_cap->cac_interval[0] << 16 |  cac_cap->cac_interval[1] << 8 || cac_cap->cac_interval;
			cJSON_AddNumberToObject(cac_method_obj, "NumberOfSeconds", cac_interval);
			cac_cap_opclass_arr = cJSON_AddArrayToObject(cac_method_obj, "OpClassList");
			SLIST_FOREACH_SAFE(opcap, &cac_cap->cac_opcap_head, cac_opcap_entry, t_opcap)
			{
				cac_cap_opclass_obj = cJSON_CreateObject();
				cJSON_AddNumberToObject(cac_cap_opclass_obj, "OperatingClass", opcap->op_class);
				cac_cap_ch_arr = cJSON_AddArrayToObject(cac_cap_opclass_obj, "ChannelList");
				for (i = 0; i < opcap->ch_num; i++) {
					cac_cap_ch = cJSON_CreateNumber(opcap->ch_list[i]);
					cJSON_AddItemToArray(cac_cap_ch_arr, cac_cap_ch);
				}
				cJSON_AddItemToArray(cac_cap_opclass_arr, cac_cap_opclass_obj);
			}
			cJSON_AddItemToArray(cac_cap_arr, cac_method_obj);
		}
	}
#endif

	num_bss = get_radio_bss_count(dev, ra);
	cJSON_AddNumberToObject(radio_obj, "NumberOfBSS", num_bss);
	if (num_bss) {
		bss_arr = cJSON_AddArrayToObject(radio_obj, "BSSList");
		SLIST_FOREACH_SAFE(bss, &dev->first_bss, next_bss, t_bss)
		{
			if (bss->radio == ra) {
				bss_obj = de_create_radio_bss_obj(ctx, bss);
				cJSON_AddItemToArray(bss_arr, bss_obj);
			}
		}
	} else {
		cJSON_AddNullToObject(radio_obj, "BSSList");
	}

	return radio_obj;

fail:
	radio_obj = cJSON_CreateNull();

	return radio_obj;
}


int de_create_ts_policy_arr(struct _1905_map_device *dev, cJSON *ts_policy_arr)
{
	cJSON *ts_policy_obj = NULL;
	char **ssid_arr = NULL, **ssid_arr_tmp = NULL;
	char *pssid = NULL;
	struct bss_info_db *bss = NULL, *t_bss = NULL;
	unsigned short i = 0, j = 0, k = 0;
	unsigned short num_ssid = 0;

	if (!dev || !ts_policy_arr) {
		err("invalid param!! dev(%p) or ts_policy_arr(%p) is NULL",
			dev, ts_policy_arr);
		goto fail;
	}

	if (!dev->setting) {
		err("no ts policy");
		goto fail;
	}

	for (i = 0; i < dev->setting->fh_bss_setting.itf_num; i++) {
		SLIST_FOREACH_SAFE(bss, &dev->first_bss, next_bss, t_bss)
		{
			if (!os_memcmp(bss->bssid, dev->setting->fh_bss_setting.fh_configs[i].itf_mac,ETH_ALEN))
				break;
		}
		if (!bss)
			continue;
		if (num_ssid == 0) {
			ssid_arr = os_calloc(num_ssid + 1, sizeof(char *));
			if (!ssid_arr) {
				err("alloc ssid_arr_tmp fail for ssid(%s)", bss->ssid);
				goto fail;
			}
			pssid = (char *)os_zalloc(MAX_SSID_LEN + 1);
			if (!pssid) {
				err("alloc pssid fail for ssid(%s)", bss->ssid);
				goto fail;
			}
			snprintf(pssid, MAX_SSID_LEN + 1, "%s", (char *)bss->ssid);
			ssid_arr[num_ssid++] = pssid;
			ts_policy_obj = cJSON_CreateObject();
			cJSON_AddStringToObject(ts_policy_obj, "SSID", (char *)bss->ssid);
			cJSON_AddNumberToObject(ts_policy_obj, "VID", dev->setting->fh_bss_setting.fh_configs[i].vid);
			cJSON_AddItemToArray(ts_policy_arr, ts_policy_obj);
			continue;
		}

		/*check ssid in ssid_arr*/
		for (j = 0; j < num_ssid; j++) {
			if (!os_memcmp(bss->ssid, ssid_arr[j], MAX_SSID_LEN))
				break;
		}
		if (j < num_ssid)
			continue;
		ssid_arr_tmp = os_realloc_array(ssid_arr, num_ssid + 1, sizeof(char *));
		if (ssid_arr_tmp == NULL) {
			err("alloc ssid_arr_tmp fail for ssid(%s)", bss->ssid);
			goto fail;
		}
		ssid_arr = ssid_arr_tmp;
		pssid = (char *)os_zalloc(MAX_SSID_LEN + 1);
		if (!pssid) {
			err("alloc pssid fail for ssid(%s)", bss->ssid);
			goto fail;
		}
		snprintf(pssid, MAX_SSID_LEN + 1, "%s", (char *)bss->ssid);
		ssid_arr[num_ssid++] = pssid;
		ts_policy_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(ts_policy_obj, "SSID", (char *)bss->ssid);
		cJSON_AddNumberToObject(ts_policy_obj, "VID", dev->setting->fh_bss_setting.fh_configs[i].vid);
		cJSON_AddItemToArray(ts_policy_arr, ts_policy_obj);
	}

	if (ssid_arr) {
		for (k = 0; k < num_ssid; k++) {
			os_free(ssid_arr[k]);
		}
		os_free(ssid_arr);
		ssid_arr = NULL;
	}
	return 0;

fail:
	if (ssid_arr) {
		for (k = 0; k < num_ssid; k++) {
			os_free(ssid_arr[k]);
		}
		os_free(ssid_arr);
		ssid_arr = NULL;
	}

	return -1;
}

cJSON * de_create_device_obj(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	cJSON *device_obj = NULL;
	cJSON *local_steer_disallow_sta_arr = NULL;
	cJSON *btm_steer_disallow_sta_arr = NULL;
	cJSON *sta_cjson = NULL;
#ifdef MAP_R2
	cJSON *d80211Q_obj = NULL;
	cJSON *ts_policy_arr = NULL;
#endif
	cJSON *radio_arr = NULL;
	cJSON *radio_obj = NULL;
#ifdef MAP_R3
	cJSON *prio_arr = NULL;
	cJSON *prio_obj = NULL;
	cJSON *_1905_secure_obj = NULL;
#endif
	struct _1905_map_device *own_dev = NULL;
	unsigned char *base64_str = NULL;
	struct sta_db *sta = NULL, *t_sta = NULL;
	struct radio_info_db *radio = NULL, *t_radio = NULL;
#ifdef MAP_R3
	struct sp_rule_db *sp_rule = NULL, t_sp_rule = NULL;
#endif
	char dev_id_str[32] = {0};
	char sta_mac_str[32] = {0};
#ifdef MAP_R2
	char cc_str[4] = {0};
#endif

#ifdef MAP_R3
	unsigned int rule_id = 0;
#endif
	unsigned short num_radio = 0;
	unsigned char cap = 0;


	if (!ctx || !dev || !dev->in_network) {
		err(DE_PREX"invalid parameters!!! ctx or dev NULL or dev not in network");
		device_obj = cJSON_CreateNull();
		return device_obj;
	}

	own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	if (!own_dev) {
		err(DE_PREX"get own 1905.1 device fail");
		device_obj = cJSON_CreateNull();
		return device_obj;
	}

	device_obj = cJSON_CreateObject();

	snprintf(dev_id_str, sizeof(dev_id_str), MACSTR, MAC2STR(dev->_1905_info.al_mac_addr));
	cJSON_AddStringToObject(device_obj, "ID", dev_id_str);
	os_memcpy(cc_str, dev->country_code, 2);
	cJSON_AddStringToObject(radio_obj, "CountryCode", cc_str);

	cap = dev->ap_cap.sta_report_on_cop << 7 | dev->ap_cap.sta_report_not_cop << 6
		| dev->ap_cap.rssi_steer << 5;
	base64_str = base64_encode(&cap, sizeof(cap), NULL);
	if (base64_str) {
		cJSON_AddStringToObject(device_obj, "MultiAPCapabilities", (char *)base64_str);
		os_free(base64_str);
		base64_str = NULL;
	} else {
		cJSON_AddNullToObject(device_obj, "MultiAPCapabilities");
	}
	cJSON_AddNumberToObject(device_obj, "MultiAPProfile", dev->map_version);

#ifdef MAP_R2
	/*all dev has the same ts policy; use controller policy to sync*/
	if (own_dev->setting) {
		/*Default8021Q*/
		d80211Q_obj = cJSON_AddObjectToObject(device_obj, "Default8021Q");
		cJSON_AddNumberToObject(d80211Q_obj, "PrimaryVID", own_dev->setting->common_setting.primary_vid);
		cJSON_AddNumberToObject(d80211Q_obj, "DefaultPCP", own_dev->setting->common_setting.primary_pcp);

		/*TrafficSeparationPolicy*/
		ts_policy_arr = cJSON_AddArrayToObject(device_obj, "TrafficSeparationPolicy");
		if (de_create_ts_policy_arr(own_dev, ts_policy_arr) < 0) {
			err(DE_PREX"create ts arr error");
			/*free memory and recreate again*/
			cJSON_Delete(ts_policy_arr);
			cJSON_AddNullToObject(device_obj, "TrafficSeparationPolicy");
		}
	} else {
		cJSON_AddNullToObject(device_obj, "Default8021Q");
		cJSON_AddNullToObject(device_obj, "TrafficSeparationPolicy");
	}
#endif
	/*CollectionInterval
	* should be implement in mapd later.
	*/

#ifdef MAP_R3_DE
	cJSON_AddStringToObject(device_obj, "Manufacturer", "unknown");
	cJSON_AddStringToObject(device_obj, "SerialNumber", dev->de.ser_num);
	cJSON_AddStringToObject(device_obj, "ManufacturerModel", "unknown");
	cJSON_AddStringToObject(device_obj, "SoftwareVersion", dev->de.sw_ver);
	cJSON_AddStringToObject(device_obj, "ExecutionEnv", dev->de.exec_env);
#endif

	/*controller case*/
	if (own_dev == dev) {
#ifdef MAP_R2
		/*it's hard code in mapd*/
		cJSON_AddTrueToObject(device_obj, "ReportUnsuccessfulAssociations");
		cJSON_AddNumberToObject(device_obj, "MaxReportingRate", 10);
#endif
		cJSON_AddNumberToObject(device_obj, "APMetricsReportingInterval", ctx->map_policy.mpolicy.report_interval);

		if (ctx->map_policy.spolicy.local_disallow_count) {
			local_steer_disallow_sta_arr = cJSON_AddArrayToObject(device_obj, "LocalSteeringDisallowedSTAList");
			SLIST_FOREACH_SAFE(sta, &ctx->map_policy.spolicy.local_disallow_head, sta_entry, t_sta)
			{
				snprintf(sta_mac_str, sizeof(sta_mac_str), MACSTR, MAC2STR(sta->mac));
				sta_cjson = cJSON_CreateString(sta_mac_str);
				cJSON_AddItemToArray(local_steer_disallow_sta_arr, sta_cjson);
			}
		} else {
			cJSON_AddNullToObject(device_obj, "LocalSteeringDisallowedSTAList");
		}

		if (ctx->map_policy.spolicy.btm_disallow_count) {
			btm_steer_disallow_sta_arr = cJSON_AddArrayToObject(device_obj, "BTMSteeringDisallowedSTAList");
			t_sta = NULL;
			SLIST_FOREACH_SAFE(sta, &ctx->map_policy.spolicy.btm_disallow_head, sta_entry, t_sta)
			{
				snprintf(sta_mac_str, sizeof(sta_mac_str), MACSTR, MAC2STR(sta->mac));
				sta_cjson = cJSON_CreateString(sta_mac_str);
				cJSON_AddItemToArray(btm_steer_disallow_sta_arr, sta_cjson);
			}
		} else {
			cJSON_AddNullToObject(device_obj, "BTMSteeringDisallowedSTAList");
		}
#ifdef MAP_R2
		if (ctx->r2_ap_capab) {
			cJSON_AddNumberToObject(device_obj, "MaxPrioritizationRules", ctx->r2_ap_capab->max_total_num_sp_rules);
			cJSON_AddNumberToObject(device_obj, "MaxVIDs", ctx->r2_ap_capab->max_total_num_vid);
		} else {
			cJSON_AddNullToObject(device_obj, "MaxPrioritizationRules");
			cJSON_AddNullToObject(device_obj, "MaxVIDs");
		}
#endif
	} else { /*agent case*/
#ifdef MAP_R2
		if (dev->policy_record.assoc_failed_policy.report_unsuccessful_association) {
			cJSON_AddTrueToObject(device_obj, "ReportUnsuccessfulAssociations");
			cJSON_AddNumberToObject(device_obj, "MaxReportingRate",
				dev->policy_record.assoc_failed_policy.max_supporting_rate);
		} else {
			cJSON_AddFalseToObject(device_obj, "ReportUnsuccessfulAssociations");
			cJSON_AddNumberToObject(device_obj, "MaxReportingRate", 0);
		}
#endif
		cJSON_AddNumberToObject(device_obj, "APMetricsReportingInterval", dev->policy_record.mpolicy.report_interval);
		if (dev->policy_record.spolicy.local_disallow_count) {
			local_steer_disallow_sta_arr = cJSON_AddArrayToObject(device_obj, "LocalSteeringDisallowedSTAList");
			t_sta = NULL;
			SLIST_FOREACH_SAFE(sta, &dev->policy_record.spolicy.local_disallow_head, sta_entry, t_sta)
			{
				snprintf(sta_mac_str, sizeof(sta_mac_str), MACSTR, MAC2STR(sta->mac));
				sta_cjson = cJSON_CreateString(sta_mac_str);
				cJSON_AddItemToArray(local_steer_disallow_sta_arr, sta_cjson);
			}
		} else {
			cJSON_AddNullToObject(device_obj, "LocalSteeringDisallowedSTAList");
		}

		if (dev->policy_record.spolicy.btm_disallow_count) {
			btm_steer_disallow_sta_arr = cJSON_AddArrayToObject(device_obj, "BTMSteeringDisallowedSTAList");
			t_sta = NULL;
			SLIST_FOREACH_SAFE(sta, &dev->policy_record.spolicy.btm_disallow_head, sta_entry, t_sta)
			{
				snprintf(sta_mac_str, sizeof(sta_mac_str), MACSTR, MAC2STR(sta->mac));
				sta_cjson = cJSON_CreateString(sta_mac_str);
				cJSON_AddItemToArray(btm_steer_disallow_sta_arr, sta_cjson);
			}
		} else {
			cJSON_AddNullToObject(device_obj, "BTMSteeringDisallowedSTAList");
		}
#ifdef MAP_R2
		cJSON_AddNumberToObject(device_obj, "MaxPrioritizationRules", dev->max_prio_rules);
		cJSON_AddNumberToObject(device_obj, "MaxVIDs", dev->max_vid);
#endif
	}

#ifdef MAP_R3
	_1905_secure_obj = cJSON_AddObjectToObject(device_obj, "IEEE1905Security");
	cJSON_AddNumberToObject(_1905_secure_obj, "OnboardingProtocol", dev->onboarding_proto);
	cJSON_AddNumberToObject(_1905_secure_obj, "IntegrityAlgorithm", dev->msg_int_alg);
	cJSON_AddNumberToObject(_1905_secure_obj, "EncryptionAlgorithm", dev->msg_enc_alg);
#else
	cJSON_AddNullToObject(device_obj, "IEEE1905Security");
#endif

	/*not support*/
	cJSON_AddNullToObject(device_obj, "CACStatus");

#ifdef MAP_R3
	/*Prioritization*/
	if (!SLIST_EMPTY(&own_dev->sp_rule_head)) {
		prio_arr = cJSON_AddArrayToObject(device_obj, "Prioritization");
		SLIST_FOREACH_SAFE(sp_rule, &own_dev->sp_rule_head, sp_rule_entry, t_sp_rule)
		{
			prio_obj = cJSON_CreateObject();
			rule_id = sp_rule->rule_id[0] << 24 | sp_rule->rule_id[1] << 16 |
				sp_rule->rule_id[2] << 8 | sp_rule->rule_id[3];
			cJSON_AddNumberToObject(prio_obj, "ID", rule_id);
			cJSON_AddNumberToObject(prio_obj, "Precedence", sp_rule->precedence);
			cJSON_AddNumberToObject(prio_obj, "Output", sp_rule->output);
			if (sp_rule->always_match)
				cJSON_AddTrueToObject(prio_obj, "AlwaysMatch");
			else
				cJSON_AddFalseToObject(prio_obj, "AlwaysMatch");
			cJSON_AddItemToArray(prio_arr, prio_obj);
		}
	}

	if (own_dev->dscp_tbl) {
		base64_str = base64_encode(own_dev->dscp_tbl, 64, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(device_obj, "DSCPMap", (char *)base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(device_obj, "DSCPMap");
		}
	} else {
		cJSON_AddNullToObject(device_obj, "DSCPMap");
	}
#else
	cJSON_AddNullToObject(device_obj, "DSCPMap");
#endif

	num_radio = topo_srv_get_device_radio_count(dev);
	cJSON_AddNumberToObject(device_obj, "NumberOfRadios", num_radio);
	if (num_radio) {
		radio_arr = cJSON_AddArrayToObject(device_obj, "RadioList");
		SLIST_FOREACH_SAFE(radio, &dev->first_radio, next_radio, t_radio)
		{
			radio_obj = de_create_radio_obj(ctx, radio);
			cJSON_AddItemToArray(radio_arr, radio_obj);
		}
	} else {
		cJSON_AddNullToObject(device_obj, "RadioList");
	}

	return device_obj;
}


char * mapd_get_de_network(struct own_1905_device *ctx)
{
	cJSON *root_obj = NULL;
	cJSON *network_obj = NULL;
	cJSON *device_obj = NULL;
	cJSON *device_arr = NULL;
	char *cjson_string_network = NULL;
	struct _1905_map_device *own_dev = NULL;
	struct _1905_map_device *tmp_dev = NULL, *t_tmp_dev = NULL;
	char cont_id_str[32] = {0};
	char timestamp_str[64] = {0};
	unsigned short num_dev = 0;

	if (!ctx) {
		err(DE_PREX"invalid parameters!!! ctx NULL");
		return NULL;
	}

	own_dev = topo_srv_get_next_1905_device(ctx, NULL);
	if (!own_dev) {
		err(DE_PREX"get own 1905.1 device fail");
		return NULL;
	}

	root_obj = cJSON_CreateObject();
	network_obj = cJSON_AddObjectToObject(root_obj, "wfa-dataelements:Network");
	cJSON_AddStringToObject(network_obj, "ID", "Mesh-Network");

	get_timestamp_str(timestamp_str, sizeof(timestamp_str));
	cJSON_AddStringToObject(network_obj, "TimeStamp", timestamp_str);

	snprintf(cont_id_str, sizeof(cont_id_str), MACSTR, MAC2STR(own_dev->_1905_info.al_mac_addr));
	cJSON_AddStringToObject(network_obj, "ControllerID", cont_id_str);

	num_dev = topo_srv_get_device_count(ctx);
	cJSON_AddNumberToObject(network_obj, "NumberOfDevices", num_dev);
	if (num_dev) {
		device_arr = cJSON_AddArrayToObject(network_obj, "DeviceList");
		SLIST_FOREACH_SAFE(tmp_dev, &ctx->_1905_dev_head, next_1905_device, t_tmp_dev)
		{
			if (tmp_dev->in_network) {
				device_obj = de_create_device_obj(ctx, tmp_dev);
				cJSON_AddItemToArray(device_arr, device_obj);
			}
		}
	} else {
		err(DE_PREX"num_dev(%d); at least one device expected!!", num_dev);
		cJSON_AddNullToObject(network_obj, "DeviceList");
	}

	 cjson_string_network = cJSON_Print(root_obj);
	 cJSON_Delete(root_obj);

	 return cjson_string_network;
}


int create_sta_association_json_file(struct own_1905_device *ctx, unsigned char *sta_mac)
{
	cJSON *root_obj = NULL;
	cJSON *event_obj = NULL;
	cJSON *wfa_de_assoc_event_obj = NULL;
	cJSON *assoc_event_obj = NULL;
#ifdef MAP_R2
	cJSON *wf6_obj = NULL;
#endif
	FILE *defptr = NULL;
	struct client *cli = NULL;
#ifdef MAP_R2
	char *base64_str = NULL;
#endif
	char *cjson_string = NULL;
	char timestamp_str[64] = {0};
	char zero_mac[ETH_ALEN] = {0};
	char mac_str[32] = {0};
	char cap_str[32] = {0};
	char cli_found = 0;
#ifdef MAP_R2
	char he_mcs_len = 0;
	int n = 0;
#endif

	if (!sta_mac) {
		err("invalid params!! sta_mac is null");
		return -1;
	}

	dl_list_for_each(cli, &ctx->sta_seen_list, struct client, sta_seen_entry)
	{
		if (!os_memcmp(&cli->mac_addr[0], sta_mac, ETH_ALEN) &&
			os_memcmp(cli->bssid, zero_mac, ETH_ALEN)) {
			cli_found = 1;
			break;
		}
	}
	if (!cli_found) {
		err("invalid params!! can't find associted client by mac("MACSTR")", MAC2STR(sta_mac));
		return -1;
	}

	defptr = fopen(ctx->last_con_event_json_file, "w");
	if(defptr == NULL) {
		err(DE_PREX"can't open file(%s)", ctx->last_con_event_json_file);
		return -1;
	}

	root_obj = cJSON_CreateObject();
	event_obj = cJSON_AddObjectToObject(root_obj, "notification");
	get_timestamp_str(timestamp_str, sizeof(timestamp_str));
	cJSON_AddStringToObject(event_obj, "eventTime", timestamp_str);
	wfa_de_assoc_event_obj = cJSON_AddObjectToObject(event_obj, "wfa-dataelements:AssociationEvent");
	assoc_event_obj = cJSON_AddObjectToObject(wfa_de_assoc_event_obj, "AssocData");
	snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(cli->bssid));
	cJSON_AddStringToObject(assoc_event_obj, "BSSID", mac_str);
	snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(cli->mac_addr));
	cJSON_AddStringToObject(assoc_event_obj, "MACAddress", mac_str);
	/*current design; driver only report successful association for the client to mapd*/
	cJSON_AddNumberToObject(assoc_event_obj, "StatusCode", 0);

	if (cli->cht.valid) {
		cap_str[0] = (cli->cht.tx_stream << 6) | (cli->cht.rx_stream << 4) |
			(cli->cht.sgi_20 << 3) | (cli->cht.sgi_40 << 2) | (cli->cht.ht_40 << 1);
		base64_str = (char *)base64_encode((u8 *)&cap_str, 1, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(assoc_event_obj, "HTCapabilities", base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(assoc_event_obj, "HTCapabilities");
		}
	} else {
		cJSON_AddNullToObject(assoc_event_obj, "HTCapabilities");
	}

	if (cli->cvht.valid) {
		/* mcs transfer to big endian */
		cap_str[0] = cli->cvht.vht_tx_mcs[1];
		cap_str[1] = cli->cvht.vht_tx_mcs[0];
		cap_str[2] = cli->cvht.vht_rx_mcs[1];
		cap_str[3] = cli->cvht.vht_rx_mcs[0];

		cap_str[4] = (cli->cvht.tx_stream << 5) | (cli->cvht.rx_stream << 2) |
			(cli->cvht.sgi_80 << 1) | cli->cvht.sgi_160;
		cap_str[5] = (cli->cvht.vht_8080 << 7) | (cli->cvht.vht_160 << 6) |
			(cli->cvht.su_beamformer << 5) | (cli->cvht.mu_beamformer << 4);
		base64_str =  (char *)base64_encode((u8 *)cap_str, 6, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(assoc_event_obj, "VHTCapabilities", base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(assoc_event_obj, "VHTCapabilities");
		}
	} else {
		cJSON_AddNullToObject(assoc_event_obj, "VHTCapabilities");
	}
#ifdef MAP_R2
	if (cli->che.valid) {
		he_mcs_len = cli->che.he_mcs_len > MAX_HE_MCS_LEN ? MAX_HE_MCS_LEN : cli->che.he_mcs_len;
		cap_str[0] = he_mcs_len;
		/* mcs transfer to big endian */
		for (n = 0; n < he_mcs_len; n += 2) {
			cap_str[1 + n + 1] = cli->che.he_mcs[n];
			cap_str[1 + n] = cli->che.he_mcs[n + 1];
		}
		cap_str[1 + he_mcs_len] = (cli->che.tx_spatial_stream << 5) | (cli->che.rx_spatial_streams << 2) |
			(cli->che.he_80plus80 << 1) | cli->che.he_160;
		cap_str[2 + he_mcs_len] = (cli->che.su_beamformer << 7) | (cli->che.mu_beamformer << 6) |
			(cli->che.ul_mu_mimo << 5) | (cli->che.ul_ofdma_plus_mu_mimo << 4) |
			(cli->che.dl_ofdma_plus_mu_mimo << 3) | (cli->che.ul_ofdma_supported << 2) |
			(cli->che.dl_ofdma_supported << 1);
		base64_str = (char *)base64_encode((u8 *)cap_str, 3 + he_mcs_len, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(assoc_event_obj, "HECapabilities", base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(assoc_event_obj, "HECapabilities");
		}
		cJSON_AddStringToObject(assoc_event_obj, "HECapabilities", base64_str);

		/*wf6*/
		wf6_obj = de_create_sta_wf6_obj(cli);
		cJSON_AddItemToObject(assoc_event_obj, "WiFi6Capabilities", wf6_obj);
	}  else {
		cJSON_AddNullToObject(assoc_event_obj, "HECapabilities");
		cJSON_AddNullToObject(assoc_event_obj, "WiFi6Capabilities");
	}
	if (cli->assoc_req_ie) {
		base64_str = (char *)base64_encode((unsigned char *)cli->assoc_req_ie, cli->assoc_req_ie_len, NULL);
		if (base64_str) {
			cJSON_AddStringToObject(assoc_event_obj, "ClientCapabilities", base64_str);
			os_free(base64_str);
			base64_str = NULL;
		} else {
			cJSON_AddNullToObject(assoc_event_obj, "ClientCapabilities");
		}
	} else {
		cJSON_AddNullToObject(assoc_event_obj, "ClientCapabilities");
	}
#endif


	cjson_string = cJSON_Print(root_obj);
	cJSON_Delete(root_obj);

	if (!cjson_string) {
		err(DE_PREX"genarate data element Association Event json str fail on controller");
		goto fail;
	}
	fprintf(defptr, "%s", cjson_string);
	fclose(defptr);

	os_free(cjson_string);
	cjson_string = NULL;

	return 0;

fail:
	if (defptr) {
		fclose(defptr);
		defptr = NULL;
	}

	return -1;
}

int create_sta_disassociation_json_file(struct own_1905_device *ctx, unsigned char *sta_mac, char remote)
{
	cJSON *root_obj = NULL;
	cJSON *event_obj = NULL;
	cJSON *wfa_de_disassoc_event_obj = NULL;
	cJSON *disassoc_event_obj = NULL;
	struct _1905_map_device *own_dev = NULL, *dev = NULL;
	struct associated_clients *sta = NULL, *t_sta = NULL;
	FILE *defptr = NULL;
	struct client *cli = NULL;
	char *cjson_string = NULL;
	char timestamp_str[64] = {0};
	char mac_str[32] = {0};
	char cli_found = 0;

	if (!sta_mac) {
		err("invalid params!! sta_mac is null");
		return -1;
	}

	own_dev = topo_srv_get_next_1905_device(ctx, NULL);

	if (!remote) {
		SLIST_FOREACH_SAFE(sta, &own_dev->assoc_clients, next_client, t_sta)
		{
			if (!os_memcmp(&sta->client_addr[0], sta_mac, ETH_ALEN))
				break;
		}
	} else {
		dev = SLIST_NEXT(own_dev, next_1905_device);
		while (dev) {
			t_sta = NULL;
			SLIST_FOREACH_SAFE(sta, &dev->assoc_clients, next_client, t_sta)
			{
				if (!os_memcmp(&sta->client_addr[0], sta_mac, ETH_ALEN)) {
					cli_found = 1;
					break;
				}
			}
			if (cli_found)
				break;
		}
	}

	if (!sta || !sta->bss) {
		err("invalid params!! can't find associted client by mac("MACSTR")", MAC2STR(sta_mac));
		return -1;
	}


	cli_found = 0;
	dl_list_for_each(cli, &ctx->sta_seen_list, struct client, sta_seen_entry)
	{
		if (!os_memcmp(&cli->mac_addr[0], sta_mac, ETH_ALEN)) {
			cli_found = 1;
			break;
		}
	}
	if (!cli_found) {
		err("invalid params!! can't find client by mac("MACSTR")", MAC2STR(sta_mac));
		return -1;
	}

	defptr = fopen(ctx->last_con_event_json_file, "w");
	if(defptr == NULL) {
		err(DE_PREX"can't open file(%s)", ctx->last_con_event_json_file);
		return -1;
	}

	root_obj = cJSON_CreateObject();
	event_obj = cJSON_AddObjectToObject(root_obj, "notification");
	get_timestamp_str(timestamp_str, sizeof(timestamp_str));
	cJSON_AddStringToObject(event_obj, "eventTime", timestamp_str);
	wfa_de_disassoc_event_obj = cJSON_AddObjectToObject(event_obj, "wfa-dataelements:DisassociationEvent");
	disassoc_event_obj = cJSON_AddObjectToObject(wfa_de_disassoc_event_obj, "DisassocData");
	snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(sta->bss->bssid));
	cJSON_AddStringToObject(disassoc_event_obj, "BSSID", mac_str);
	snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(sta->client_addr));
	cJSON_AddStringToObject(disassoc_event_obj, "MACAddress", mac_str);
#ifdef CENT_STR
	cJSON_AddNumberToObject(disassoc_event_obj, "BytesSent", sta->stat_db.bytes_sent);
	cJSON_AddNumberToObject(disassoc_event_obj, "BytesReceived", sta->stat_db.bytes_received);
	cJSON_AddNumberToObject(disassoc_event_obj, "PacketsSent", sta->stat_db.packets_sent);
	cJSON_AddNumberToObject(disassoc_event_obj, "PacketsReceived", sta->stat_db.packets_received);
	cJSON_AddNumberToObject(disassoc_event_obj, "ErrorsSent", sta->stat_db.tx_packets_errors);
	cJSON_AddNumberToObject(disassoc_event_obj, "ErrorsReceived", sta->stat_db.rx_packets_errors);
	cJSON_AddNumberToObject(disassoc_event_obj, "RetransCount", sta->stat_db.retransmission_count);
	err("sta("MACSTR") bys(%d) byr(%d) ps(%d) pr(%d) txe(%d) rxe(%d) re(%d)",
		MAC2STR(sta->client_addr),
		sta->stat_db.bytes_sent, sta->stat_db.bytes_received,
		sta->stat_db.packets_sent, sta->stat_db.packets_received,
		sta->stat_db.tx_packets_errors, sta->stat_db.rx_packets_errors,
		sta->stat_db.retransmission_count);
#endif
	cJSON_AddNumberToObject(disassoc_event_obj, "ReasonCode", cli->disassoc_reason);

	cjson_string = cJSON_Print(root_obj);
	cJSON_Delete(root_obj);

	if (!cjson_string) {
		err(DE_PREX"genarate data element Disassociation Event json str fail on controller");
		goto fail;
	}
	fprintf(defptr, "%s", cjson_string);
	fclose(defptr);

	os_free(cjson_string);
	cjson_string = NULL;

	return 0;

fail:
	if (defptr) {
		fclose(defptr);
		defptr = NULL;
	}

	return -1;
}


int create_sta_fail_connect_json_file(struct own_1905_device *ctx,
	unsigned char *sta_mac, unsigned short reason, unsigned short status)
{
	cJSON *root_obj = NULL;
	cJSON *event_obj = NULL;
	cJSON *wfa_de_fail_conn_event_obj = NULL;
	cJSON *fail_conn_event_obj = NULL;
	FILE *defptr = NULL;
	char *cjson_string = NULL;
	char mac_str[32] = {0};
	char timestamp_str[64] = {0};

	if (!sta_mac) {
		err("invalid params!! sta_mac is null");
		return -1;
	}

	defptr = fopen(ctx->last_con_event_json_file, "w");
	if(defptr == NULL) {
		err(DE_PREX"can't open file(%s)", ctx->last_con_event_json_file);
		return -1;
	}

	root_obj = cJSON_CreateObject();
	event_obj = cJSON_AddObjectToObject(root_obj, "notification");
	get_timestamp_str(timestamp_str, sizeof(timestamp_str));
	cJSON_AddStringToObject(event_obj, "eventTime", timestamp_str);
	wfa_de_fail_conn_event_obj = cJSON_AddObjectToObject(event_obj, "wfa-dataelements:FailedConnectionEvent");
	fail_conn_event_obj = cJSON_AddObjectToObject(wfa_de_fail_conn_event_obj, "FailedConnectionEventData");

	snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(sta_mac));
	cJSON_AddStringToObject(fail_conn_event_obj, "MACAddress", mac_str);
	cJSON_AddNumberToObject(fail_conn_event_obj, "StatusCode", status);
	cJSON_AddNumberToObject(fail_conn_event_obj, "ReasonCode", reason);

	cjson_string = cJSON_Print(root_obj);
	cJSON_Delete(root_obj);

	if (!cjson_string) {
		err(DE_PREX"genarate data element fail connection event json str fail on controller");
		goto fail;
	}
	fprintf(defptr, "%s", cjson_string);
	fclose(defptr);

	os_free(cjson_string);
	cjson_string = NULL;

	return 0;

fail:
	if (defptr) {
		fclose(defptr);
		defptr = NULL;
	}

	return -1;
}



