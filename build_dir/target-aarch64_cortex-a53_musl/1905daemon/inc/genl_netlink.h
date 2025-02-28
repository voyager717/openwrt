
#define MT753X_GENL_NAME "mt753x"
#define MT753X_MULTICAST_GROUP_PORT_STATUS		"port_status"

/*add your cmd to here*/
enum {
	MT753X_CMD_UNSPEC = 0, /*Reserved*/
	MT753X_CMD_REQUEST,    /*user->kernelrequest/get-response*/
	MT753X_CMD_REPLY,      /*kernel->user event*/
	MT753X_CMD_READ,
	MT753X_CMD_WRITE,
	MT753X_CMD_NOTIFY,
	__MT753X_CMD_MAX,
};
#define MT753X_CMD_MAX (__MT753X_CMD_MAX - 1)

/*define attar types */
enum
{
	MT753X_ATTR_TYPE_UNSPEC = 0,
	MT753X_ATTR_TYPE_MESG, /*MT753X message*/
	MT753X_ATTR_TYPE_PHY,
	MT753X_ATTR_TYPE_PHY_DEV,
	MT753X_ATTR_TYPE_REG,
	MT753X_ATTR_TYPE_VAL,
	MT753X_ATTR_TYPE_DEV_NAME,
	MT753X_ATTR_TYPE_DEV_ID,
	__MT753X_ATTR_TYPE_MAX,
};
#define MT753X_ATTR_TYPE_MAX (__MT753X_ATTR_TYPE_MAX - 1)

struct handler_args {  // For family_handler() and nl_get_multicast_id().
    const char *group;
    int id;
};

struct nl_sock * gen_netlink_init();
int gen_netlink_deinit();
int genl_netlink_recv(struct nl_sock *gsock, char *buf);

