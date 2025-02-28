#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>	/*Get MAC Address from kernel*/

int init_eth(struct HOST_IF *fd, char *bridge_ifname);
int rev_cmd_eth(unsigned char *buf, int size);
void drv_add_addr(char *addr);
int find_mac_index(char *addr);
int rsp2host_eth(unsigned char *data, int size);
int close_eth();