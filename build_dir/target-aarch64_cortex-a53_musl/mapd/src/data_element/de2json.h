#ifndef __DE2JSON_H__
#define __DE2JSON_H__

#include "mapd_i.h"

char * mapd_get_de_network(struct own_1905_device *ctx);
int create_sta_association_json_file(struct own_1905_device *ctx, unsigned char *sta_mac);
int create_sta_disassociation_json_file(struct own_1905_device *ctx, unsigned char *sta_mac, char remote);
int create_sta_fail_connect_json_file(struct own_1905_device *ctx,
	unsigned char *sta_mac, unsigned short reason, unsigned short status);

#endif/*__DE2JSON_H__*/
