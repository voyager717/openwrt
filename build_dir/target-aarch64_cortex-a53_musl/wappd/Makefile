LDFLAGS += -lpthread
HS_EXEC = wapp
HSCTRL_EXEC = wappctrl
WAPP_USR_INTF_LIB = libwapp_usr_intf_client.a

#PLATFORM = x86
PLATFORM = mipsel

ifeq ($(PLATFORM),x86)
CROSS_COMPILE=""
endif

ifeq ($(PLATFORM),mipsel)
CROSS_COMPILE = mipsel-linux-
endif

-include wappd_config
include $(TOPDIR)/.config

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -Werror -g
endif
CFLAGS += -Werror -Wall -g

CFLAGS += -I./include -fgnu89-inline

ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
CFLAGS += -DOPENWRT_SUPPORT
CFLAGS += -DDHCP_CTL_OPENWRT
CFLAGS += -I$(UAPD_INCLUDE_PATH)
LDFLAGS += -ldatconf -lkvcutil
else
GLOBAL_INCLUDES_H = $(ROOTDIR)/$(LINUXDIR)/include/uapi/linux/wapp/
CFLAGS += -I$(GLOBAL_INCLUDES_H)
endif

ifeq ($(CONFIG_DPP_SUPPORT),y)
LDFLAGS += -lssl -lcrypto
CFLAGS += -DDPP_SUPPORT
CFLAGS += -DCONFIG_SHA512
CFLAGS += -DCONFIG_SHA256
CFLAGS += -DCONFIG_SHA384
CFLAGS += -DCONFIG_DPP2
CFLAGS += -DMAP_R3
CFLAGS += -DMAP_R3_DE
CFLAGS += -DDPP_R2_SUPPORT
#CFLAGS += -DDPP_AUTOTEST
CFLAGS += -DMAP_R3_RECONFIG
CFLAGS += -DMAP_R3_DPP_CONFIG
#CFLAGS += -DDPP_R2_MUOBJ
CFLAGS += -DDPP_R2_RECONFIG
CFLAGS += -DCONFIG_ECC
CFLAGS += -DMAP_R3_WF6
CFLAGS += -I$(abspath ./dpp)
CFLAGS += -I$(abspath ./)
CFLAGS += -I$(abspath ./crypto)
CFLAGS += -I$(abspath ./utils)
CFLAGS += -I$(abspath ./gas)
CFLAGS += -DMAP_R4_SPT
endif

ifeq ($(CONFIG_MAP_R4_SUPPORT), y)
CFLAGS += -DMAP_R4
endif

# Add map feature
CFLAGS += -DMAP_SUPPORT -DKV_API_SUPPORT

ifeq ($(CONFIG_MAP_R2_SUPPORT), y)
CFLAGS += -DMAP_R2
CFLAGS += -DDFS_CAC_R2
CFLAGS += -DOFFCHANNEL_SCAN_FEATURE
#CFLAGS += -DMAP_320BW
ifeq ($(CONFIG_MTK_MAP_R2_6E_SUPPORT), y)
CFLAGS += -DMAP_6E_SUPPORT
endif
endif

#support WIFI MD COEX on colgin
ifeq ($(WIFI_MD_COEX_SUPPORT), y)
CFLAGS += -DWIFI_MD_COEX_SUPPORT
endif

#support CONFIG_LINUX_5_4
ifeq ($(CONFIG_LINUX_5_4), y)
CFLAGS += -DCONFIG_LINUX_5_4
endif

CFLAGS += -DQOS_R1
CFLAGS += -DQOS_R2

CFLAGS += -DMBO_SUPPORT
CFLAGS += -DWPS_UNCONFIG_FEATURE_SUPPORT
CFLAGS += -DOCE_SUPPORT
CFLAGS += -DPASSPOINT_SUPPORT
#CFLAGS += -DHOSTAPD_MAP_SUPPORT
CFLAGS += -DACL_CTRL

CC = $(CROSS_COMPILE)gcc
#support for local compilation validation
#make ARCH=mips BUILD=local
ifeq ($(BUILD), local)
ifeq ($(ARCH),arm)
CC=/proj/mtk15451/toolchain-7622/bin/aarch64-openwrt-linux-gnu-gcc
endif

ifeq ($(ARCH),mips)
CC= /opt/buildroot-gcc463/usr/bin/mipsel-linux-gcc
endif
endif

WAPP_OBJS = mbo/mbo.o
WAPP_OBJS += oce/oce.o
WAPP_OBJS += common/os_internal.o common/driver_wext.o common/netlink.o common/eloop.o
WAPP_OBJS += common/main.o common/util.o common/ctrl_iface_unix.o common/debug.o common/nr_list.o
WAPP_OBJS += common/wapp_cmm.o common/wdev.o common/wdev_ap.o common/wdev_sta.o
WAPP_OBJS += iapp/mt_iapp_util.o iapp/mtkiappd.o
WAPP_OBJS += hotspot/hotspot.o
WAPP_OBJS += map/map.o
WAPP_OBJS += map/map_1905.o
WAPP_OBJS += map/map_test.o
WAPP_OBJS += wps/wps.o
WAPP_OBJS += off_ch_scan/off_ch_scan.o
WAPP_OBJS += common/dot11v_btm.o common/dot11u_gas.o
WAPP_OBJS += wapp_usr_intf/wapp_usr_intf.o
WAPP_OBJS += common/arp.o
WAPP_OBJS += dhcp_ctl/dhcp_ctl.o
WAPP_OBJS += qos/qos.o
ifeq ($(CONFIG_DPP_SUPPORT),y)
WAPP_OBJS += dpp/dpp_wired.o
WAPP_OBJS += dpp/dpp.o
WAPP_OBJS += dpp/dpp_wdev.o
WAPP_OBJS += dpp/dpp_ctrl.o
WAPP_OBJS += utils/wpa_debug.o
WAPP_OBJS += utils/wpabuf.o
WAPP_OBJS += crypto/crypto_openssl.o
WAPP_OBJS += crypto/sha512-kdf.o
WAPP_OBJS += crypto/sha384-kdf.o
WAPP_OBJS += crypto/sha256-kdf.o
WAPP_OBJS += crypto/aes-omac1.o
WAPP_OBJS += crypto/random.o
WAPP_OBJS += crypto/aes-siv.o
WAPP_OBJS += crypto/aes-ctr.o
WAPP_OBJS += utils/base64.o
WAPP_OBJS += utils/json.o
WAPP_OBJS += utils/ip_addr.o
WAPP_OBJS += gas/gas.o
WAPP_OBJS += gas/gas_query.o
WAPP_OBJS += gas/gas_server.o
WAPP_OBJS += dpp/dpp_reconfig.o
endif

WAPPCTRL_OBJS = wapp_ctrl/wapp_cli.o wapp_ctrl/wapp_ctrl.o common/os_internal.o common/debug.o 
ifeq ($(CONFIG_DPP_SUPPORT),y)
WAPPCTRL_OBJS += wapp_ctrl/dpp_cli.o
endif

WAPP_USR_INTF_OBJS = wapp_usr_intf/wapp_usr_intf_ctrl.o common/os_internal_lib.o

DEPS=$(patsubst %o,%d,$(WAPP_OBJS))
DEPS_CON=$(patsubst %o,%d,$(WAPPCTRL_OBJS))

all: $(HS_EXEC) $(HSCTRL_EXEC) $(WAPP_USR_INTF_LIB)

wapp_usr_intf/wapp_usr_intf_ctrl.o : wapp_usr_intf/wapp_usr_intf_ctrl.c wapp_usr_intf/wapp_usr_intf.h
	$(CC) -I./include -I./wapp_usr_intf -Werror -fPIC -c wapp_usr_intf/wapp_usr_intf_ctrl.c -o wapp_usr_intf/wapp_usr_intf_ctrl.o

common/os_internal_lib.o: common/os_internal.c include/os.h
	$(CC) -I./include -Werror -fPIC -c common/os_internal.c -o common/os_internal_lib.o

$(WAPP_USR_INTF_LIB) : wapp_usr_intf/wapp_usr_intf_ctrl.o common/os_internal_lib.o include/os.h
	ar r $(WAPP_USR_INTF_LIB) $(WAPP_USR_INTF_OBJS)

$(HS_EXEC): $(WAPP_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $(HS_EXEC) $(WAPP_OBJS) $(LDFLAGS)

$(HSCTRL_EXEC): $(WAPPCTRL_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $(HSCTRL_EXEC) $(WAPPCTRL_OBJS) $(LDFLAGS)

-include $(DEPS)
-include $(DEPS_CON)

.PHONY: all clean

romfs_dir = $(ROOTDIR)/romfs

romfs_dirs = etc/wifi

icon_dir = ./config_and_icon_files

romfs:
	[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)
	for i in $(romfs_dirs); do \
		[ -d $(romfs_dir)/$$i ] || mkdir -p $(romfs_dir)/$$i; \
	done
	$(ROMFSINST) /bin/$(HS_EXEC)
	$(ROMFSINST) /bin/$(HSCTRL_EXEC)
	$(ROMFSINST) /sbin/hotspot.sh
	$(ROMFSINST) /sbin/map_gen_cfg.sh
	$(ROMFSINST) /sbin/mbo_nr.sh
ifneq ($(CONFIG_SUPPORT_OPENWRT),y)
	$(ROMFSINST) ./wifi_config_save.lua /usr/bin/wifi_config_save
	rm -rf $(ROMFSDIR)/usr/bin/wifi_config_save.lua
endif
	make -C $(icon_dir) romfs

clean:
	rm -f *.o $(HS_EXEC) $(HSCTRL_EXEC) $(WAPP_USR_INTF_LIB) common/*.o iapp/*.o mbo/*.o oce/*.o hotspot/*.o hotspot_ctrl/*.o wapp_ctrl/*.o map/*.o map_1905/*.o wapp_usr_intf/*.o dhcp_ctl/*.o wps/*.o $(DEPS) $(DEPS_CON)
	rm -f $(WAPP_OBJS)
ifeq ($(CONFIG_DPP_SUPPORT),y)
	rm -f *.o $(HS_EXEC) $(HSCTRL_EXEC) $(WAPP_USR_INTF_LIB) gas/*.o dpp/*.o crypto/*.o utils/*.o $(DEPS) $(DEPS_CON)
endif
