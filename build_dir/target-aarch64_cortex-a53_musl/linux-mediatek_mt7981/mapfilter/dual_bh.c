/*
 * dual_bh, it's used to support MAP dual backhaul
 *
 * Author: Zheng Zhou <Zheng.Zhou@mediatek.com>
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_bridge.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <net/netlink.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/kobject.h>
#include <linux/version.h>
#include <linux/jhash.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include "mtkmapfilter.h"
#include "debug.h"

unsigned int dual_bh_hook_fn_pre_rout(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	struct sk_buff *skb, const struct net_device *in, const struct net_device *out,
	int (*okfn)(struct sk_buff *)
#endif
)
{
	struct ethhdr *hdr = eth_hdr(skb);
	const struct net_device *indev = NULL;
	struct map_net_device *in_map_dev = NULL;
	unsigned char device_type, primary_interface;
	int ret = NF_ACCEPT;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
#else
	indev = in;
#endif

	rcu_read_lock();
	in_map_dev = get_map_net_dev_by_mac(indev->dev_addr);
	if (!in_map_dev) {
		rcu_read_unlock();
		ret = NF_ACCEPT;
		goto end;
	}
	device_type = in_map_dev->inf.dev_type;
	primary_interface = in_map_dev->primary_interface;
	rcu_read_unlock();

	/*drop multicast/broadcast packet which is not received from primary interface*/
	if ((is_multicast_ether_addr(hdr->h_dest) || is_broadcast_ether_addr(hdr->h_dest))) {
		if (device_type == APCLI && primary_interface == INF_NONPRIMARY) {
			ret = NF_DROP;
			goto end;
		}
	}
end:
	return ret;
}


unsigned int  dual_bh_hook_fn_forward(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	struct ethhdr *hdr;
	const struct net_device *indev = NULL, *outdev = NULL;
	struct map_net_device *in_map_dev = NULL, *out_map_dev = NULL;
	unsigned char data_dir;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	indev = state->in;
	outdev = state->out;
#else
	indev = in;
	outdev = out;
#endif
	hdr = eth_hdr(skb);

	if (unlikely(!indev || !outdev))
		goto accept;

	rcu_read_lock();
	out_map_dev = get_map_net_dev_by_mac(outdev->dev_addr);
	if (!out_map_dev) {
//			pr_info("%s-should not happen(out_map_dev " MAC2STR " not found)\n", __func__, PRINT_MAC(outdev->dev_addr));
		rcu_read_unlock();
		goto accept;
	}

	data_dir = out_map_dev->inf.dev_type == APCLI ? DIR_UPSTREAM : DIR_UNKNOWN;

	in_map_dev = get_map_net_dev_by_mac(indev->dev_addr);

	if (!in_map_dev) {
//			pr_info("%s-should not happen(in_map_dev " MAC2STR " not found)\n", __func__, PRINT_MAC(indev->dev_addr));
		rcu_read_unlock();
		goto accept;
	}

	if (data_dir == DIR_UPSTREAM) {
	/*1.for multicast/broadcast packets, drop it if is not forwarded to the primary interface*/
		if ((is_multicast_ether_addr(hdr->h_dest) || is_broadcast_ether_addr(hdr->h_dest))) {
			if (out_map_dev->primary_interface == INF_NONPRIMARY) {
				rcu_read_unlock();
				goto drop;
			}
		/*2.for unicast packets, check if it is obey the forwarding rule*/
		} else {
			if (!in_map_dev->dest_dev) {
				rcu_read_unlock();
				goto accept;
			}

			/*if ethernt backhaul link exists, drop it, let it be forwarded by switch*/
			if (in_map_dev->inf.dev_type == ETH && in_map_dev->dest_dev == in_map_dev->dev) {
				rcu_read_unlock();
				goto drop;
			}

			/*if the packet received from apcli and send to apcli, drop it*/
			/*because we assuem the muilti Wifi backaul connect to same upstream device*/
			if (in_map_dev->inf.dev_type == APCLI) {
				rcu_read_unlock();
				goto drop;
			}

			skb->dev = in_map_dev->dest_dev;
		}
	}

	rcu_read_unlock();
accept:
	return NF_ACCEPT;
drop:
	return NF_DROP;
}


unsigned int  dual_bh_hook_fn_local_out_post_routing(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	void *priv, struct sk_buff *skb, const struct nf_hook_state *state
#else
	unsigned int hooknum, struct sk_buff *skb, const struct net_device *in,
	const struct net_device *out, int (*okfn)(struct sk_buff *)
#endif
)
{
	struct ethhdr *hdr;
	const struct net_device *outdev = NULL;
	struct map_net_device *out_map_dev = NULL;
	unsigned int ret = NF_ACCEPT;
	unsigned char device_type, primary_interface;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
	outdev = state->out;
#else
	outdev = out;
#endif

	hdr = eth_hdr(skb);
	/*
	a. process the multi backhaul to avoid loop
	b. process traffic separation for Mixed Ethernet interface
	*/

	rcu_read_lock();

	out_map_dev = get_map_net_dev_by_mac(outdev->dev_addr);
	if (!out_map_dev) {
		rcu_read_unlock();
		goto end;
	}

	device_type = out_map_dev->inf.dev_type;
	primary_interface = out_map_dev->primary_interface;
	rcu_read_unlock();


	if ((is_multicast_ether_addr(hdr->h_dest) || is_broadcast_ether_addr(hdr->h_dest))) {
		if (device_type == APCLI && primary_interface == INF_NONPRIMARY) {
			ret = NF_DROP;
			goto end;
		}
	}

end:
	return ret;
}

static struct nf_hook_ops dual_bh_ops[] = {
	{
		.hook		= dual_bh_hook_fn_pre_rout,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		/*fowarding hook for multi backhaul*/
		.hook		= dual_bh_hook_fn_forward,
		.pf		= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_FORWARD,
		.priority	= NF_BR_PRI_BRNF,
		//.owner		= THIS_MODULE,
	},
	{
		.hook 		= dual_bh_hook_fn_local_out_post_routing,
		.pf			= NFPROTO_BRIDGE,
		.hooknum	= NF_BR_POST_ROUTING,
		.priority	= NF_BR_PRI_FILTER_OTHER,
	}
};

void dual_bh_init(void)
{
	int ret = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	ret = nf_register_hooks(&dual_bh_ops[0], ARRAY_SIZE(dual_bh_ops));
#else
	ret = nf_register_net_hooks(&init_net, &dual_bh_ops[0], ARRAY_SIZE(dual_bh_ops));
#endif
	if (ret < 0) {
		pr_info("register nf hook fail, ret = %d\n", ret);
	}
}

void dual_bh_deinit(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(&dual_bh_ops[0], ARRAY_SIZE(dual_bh_ops));
#else
	nf_unregister_net_hooks(&init_net, &dual_bh_ops[0], ARRAY_SIZE(dual_bh_ops));
#endif
}
