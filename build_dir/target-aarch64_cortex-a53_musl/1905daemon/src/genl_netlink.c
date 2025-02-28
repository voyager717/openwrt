#include <errno.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include "genl_netlink.h"
//#include "ethernet_layer.h"

#define MT753X_GENL_NAME "mt753x"
#define MT753X_MULTICAST_GROUP_PORT_STATUS		"port_status"

static struct nl_sock *user_sock;
static struct nl_cache *cache;
static struct genl_family *family;
static struct nlattr *attrs[MT753X_ATTR_TYPE_MAX + 1];

static int ack_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_ACK.
    int *ret = arg;
    printf("ack_handler() called.\n");
    *ret = 0;
    return NL_STOP;
}

static int no_seq_check(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_SEQ_CHECK.

    printf("no_seq_check() called.\n");
    return NL_OK;
}

static int notify_msg_handler(struct nl_msg *msg, void *arg)
{
	int phy = -1, value = -1;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *buf = (char *)arg;

	if (nla_parse(attrs, MT753X_ATTR_TYPE_MAX, genlmsg_attrdata(gnlh, 0),
		      genlmsg_attrlen(gnlh, 0), NULL) < 0) {
		return 0;
	}

	if (gnlh->cmd == MT753X_CMD_NOTIFY) {
		if (attrs[MT753X_ATTR_TYPE_PHY])
			phy = nla_get_u32(attrs[MT753X_ATTR_TYPE_PHY]);
		if (attrs[MT753X_ATTR_TYPE_VAL])
			value = nla_get_u32(attrs[MT753X_ATTR_TYPE_VAL]);
		if (phy != -1 && value != -1) {
			memcpy(buf, (char *)&phy, sizeof(int));
			memcpy(buf + sizeof(int), (char *)&value, sizeof(int));

			printf("%s phy(%d) value(%d)\n",__func__, phy, value);
			return 0;
		} else {
			printf("%s invalid port and status\n",__func__);
			return -1;
		}
	}

	return 0;
}

static int family_handler(struct nl_msg *msg, void *arg) {
    struct handler_args *grp = arg;
    struct nlattr *tb[CTRL_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *mcgrp;
    int rem_mcgrp;

    printf("family_handler() called.\n");

    nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[CTRL_ATTR_MCAST_GROUPS]) return NL_SKIP;

    nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {
        struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

        nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp), nla_len(mcgrp), NULL);

        if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] || !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]) continue;
        if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]), grp->group,
                nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]))) {
            continue;
                }

        grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);

		printf("family_handler() grp->id(%d).\n", grp->id);
        break;
    }

    return NL_SKIP;
}

int nl_get_multicast_id(struct nl_sock *sock, const char *family, const char *group) {
    struct nl_msg *msg;
    struct nl_cb *cb;
    int ret, ctrlid;
    struct handler_args grp = { .group = group, .id = -ENOENT, };

    msg = nlmsg_alloc();
    if (!msg) return -ENOMEM;

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        ret = -ENOMEM;
        goto out_fail_cb;
    }

    ctrlid = genl_ctrl_resolve(sock, "nlctrl");

    genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);

    ret = -ENOBUFS;
    NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

    ret = nl_send_auto_complete(sock, msg);
    if (ret < 0) goto out;

    ret = 1;
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &ret);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, family_handler, &grp);

    while (ret > 0) {
		nl_recvmsgs(sock, cb);
		printf("nl_get_multicast_id ret(%d) grp.id(%d)\n",
			ret, grp.id);
    }

    if (ret == 0) {
		ret = grp.id;
		printf("nl_get_multicast_id grp.id(%d)\n", grp.id);
    }

    nla_put_failure:
        out:
            nl_cb_put(cb);
        out_fail_cb:
            nlmsg_free(msg);
            return ret;
}

struct nl_sock * gen_netlink_init()
{
	int mcid = -1;

	user_sock = NULL;
	cache = NULL;
	family = NULL;
	printf("gen_netlink_init in==>\n");

	/*Allocate an new netlink socket*/
	user_sock = nl_socket_alloc();
	if (!user_sock) {
		fprintf(stderr, "Failed to create user socket\n");
		goto error;
	}
	/*Connetct the genl controller*/
	if (genl_connect(user_sock)) {
		fprintf(stderr, "Failed to connetct to generic netlink\n");
		goto error;
	}
	/*Allocate an new nl_cache*/
	if (genl_ctrl_alloc_cache(user_sock, &cache) < 0) {
		fprintf(stderr, "Failed to allocate netlink cache\n");
		goto error;
	}

	/*Look up generic netlik family by "mt753x" in the provided cache*/
	family = genl_ctrl_search_by_name(cache, MT753X_GENL_NAME);
	if (!family) {
		goto error;
	}

    mcid = nl_get_multicast_id(user_sock, MT753X_GENL_NAME, MT753X_MULTICAST_GROUP_PORT_STATUS);
	printf("gen_netlink_init in==>mcid(%d)\n", mcid);
	if (mcid < 0)
		goto error;

    if (nl_socket_add_membership(user_sock, mcid) < 0) {
		printf("Failed to nl_socket_add_membership\n");
		goto error;
	}

	printf("gen_netlink_init success==>\n");
	return user_sock;

error:
    if (family)
		nl_object_put((struct nl_object *)family);
	if (cache)
		nl_cache_free(cache);
	if (user_sock)
		nl_socket_free(user_sock);
	user_sock = NULL;
	cache = NULL;
	family = NULL;

	return NULL;
}


int gen_netlink_deinit()
{
    if (family)
		nl_object_put((struct nl_object *)family);
	if (cache)
		nl_cache_free(cache);
	if (user_sock)
		nl_socket_free(user_sock);
	user_sock = NULL;
	cache = NULL;
	family = NULL;

	return 0;
}

int genl_netlink_recv(struct nl_sock *gsock, char *buf)

{
	struct nl_cb *callback = NULL;
	int ret = 0;

	 /*Allocate an new callback handler*/
	 callback = nl_cb_alloc(NL_CB_CUSTOM);
	 if (!callback) {
	   printf("%s Failed to allocate callback handler\n",__func__);
	   return -1;
	 }

	 nl_cb_set(callback, NL_CB_VALID, NL_CB_CUSTOM, notify_msg_handler, (void *)buf);
	 nl_cb_set(callback, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);

	 /*receive message from kernel request*/
	 ret = nl_recvmsgs(gsock, callback);
	 if (ret < 0) {
		printf("%s nl_recvmsgs fail\n",__func__);
		ret = -1;
	 }

	 if (callback)
		nl_cb_put(callback);

	 return ret;
}

