include ./config.mk
BIN=p1905_managerd
CTRL_EXEC = 1905ctrl
OBJPATH=obj
SRCPATH=src
INCPATH=inc
INCLUDES = -I$(INCPATH)
EXPORT_LIB = lib_export/lib1905_interface.a
ETH_OPS_LIB_DIR = ethernet
UT_DIR = src/ut


CFLAGS += -fPIC -lpthread $(INCLUDES) -Isrc/chips -Iethernet -g -Ilib_export -Ilib -Wall -Werror -MMD -O2 -DVERSION=\"$(VERSION)\"

CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)

ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/ethernet/raeth
CFLAGS += -I$(ROOTDIR)/kernel_headers/include
else
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth -Werror
CFLAGS += -I$(ROOTDIR)/kernel_headers/include
endif

ifeq ($(BACKTRACE_SUPPORT),y)
EXTRA_CFLAGS += -DSUPPORT_BACKTRACE
CFLAGS += -funwind-tables -fasynchronous-unwind-tables
LDFLAGS = -rdynamic -ldl
endif
LDFLAGS += -lpthread
ifeq ($(COREDUMP_SUPPORT),y)
EXTRA_CFLAGS += -DSUPPORT_COREDUMP
endif

ifneq ($(PLANTFORM_DISABLE_GENL_NETLINK), y)
CFLAGS += -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include/libnl-tiny
LDFLAGS += -lnl-tiny
endif

#################################################################################
#support interface
#################################################################################
ifeq ($(WIFI_SUPPORT),y)
EXTRA_CFLAGS+= -DSUPPORT_WIFI
endif
ifeq ($(ETH_SUPPORT),y)
EXTRA_CFLAGS+= -DSUPPORT_ETH
endif
ifeq ($(WIFI_STATION_SUPPORT),y)
EXTRA_CFLAGS+= -DSUPPORT_WIFI_STATION
endif
#################################################################################
#support 1905.1 function
#################################################################################
ifeq ($(PBC_SUPPORT),y)
EXTRA_CFLAGS+= -DSUPPORT_PBC
endif

ifeq ($(CONTROL_SOCKET_SUPPORT),y)
EXTRA_CFLAGS+= -DSUPPORT_CONTROL_SOCKET
endif

EXTRA_CFLAGS+= -DSUPPORT_AP_AUTO_CONFIG
EXTRA_CFLAGS+= -DSUPPORT_CMDU_RELIABLE

#################################################################################
#support wifi chips
#################################################################################


#################################################################################
#below is for source code include
#################################################################################
SRCS=cmdu.c cmdu_fragment.c cmdu_message.c cmdu_message_parse.c cmdu_tlv.c cmdu_tlv_parse.c \
     common.c lldp.c lldp_message.c lldp_message_parse.c p1905_managerd.c p1905_utils.c \
     multi_ap.c debug.c cmdu_retry_message.c message_wait_queue.c\
     crypt_aes.c crypt_biginterger.c crypt_dh.c crypt_hmac.c crypt_sha2.c file_io.c \
     _1905_interface.c _1905_lib_io.c topology.c eloop.c ethernet_layer.c netlink_event.c worker_task.c \
     mapfilter_if.c os_internal.c

ifneq ($(PLANTFORM_DISABLE_GENL_NETLINK), y)
SRCS+=genl_netlink.c
EXTRA_CFLAGS += -DDISABLE_SWITCH_POLLING
endif

ifeq ($(WIFI_SUPPORT),y)
SRCS+=wifi_utils.c
endif

ifeq ($(AP_AUTO_CONFIG_SUPPORT),y)
SRCS+=wsc_attr_tlv.c
SRCS+=wsc_message.c
SRCS+=p1905_ap_autoconfig.c
endif

ifeq ($(RETRY_MESSAGE_SUPPORT),y)
SRCS+=cmdu_retry_message.c
endif

ifeq ($(MAP_R2_SUPPORT),y)
EXTRA_CFLAGS += -DMAP_R2
ifeq ($(CONFIG_MTK_MAP_R2_6E_SUPPORT),y)
CFLAGS += -DMAP_6E_SUPPORT
endif
endif

ifeq ($(MAP_R3_SUPPORT),y)
INCLUDES += -Isrc/crypto
LDFLAGS += -lssl -lcrypto
CFLAGS += -DDPP_SUPPORT
CFLAGS += -DCONFIG_SHA512
CFLAGS += -DCONFIG_SHA256
CFLAGS += -DCONFIG_SHA384
#CFLAGS += -DCONFIG_DPP2
CFLAGS += -DCONFIG_ECC
EXTRA_CFLAGS += -DMAP_R3_DE
EXTRA_CFLAGS += -DMAP_R3
CRYPTO_LIB = src/crypto/libcrypto.a
SRCS += wpabuf.c wpa_debug.c wpa_auth.c security_engine.c dpp.c map_dpp.c base64.c json.c sha512-kdf.c sha256-kdf.c sha384-kdf.c gas.c crypto_openssl.c

CFLAGS += -DMAP_R4_SPT
EXTRA_CFLAGS += -DMAP_R3_SP
SRCS += service_prioritization.c
endif
ifeq ($(CENT_STR_SUPPORT),y)
EXTRA_CFLAGS += -DCENT_STR
endif

#################################################################################
#below is for build
#################################################################################


OBJS=$(addprefix $(OBJPATH)/,$(SRCS:.c=.o))
DEPS=$(patsubst %o,%d,$(OBJS))
CTRL_OBJS = src/1905_ctrl/1905_cli.o src/1905_ctrl/1905_ctrl.o
CTRL_DEPS_OBJS = src/1905_ctrl/1905_cli.d src/1905_ctrl/1905_ctrl.d
LIB_OBJS = obj/_1905_interface_ctrl.o obj/os_internal.o
DEPS_OBJS = obj/_1905_interface_ctrl.d obj/os_internal.d

all: $(ETH_OPS_LIB_DIR) $(BIN) $(EXPORT_LIB) $(CTRL_EXEC)

eth_ops_lib:
	cd $(ETH_OPS_LIB_DIR) && $(MAKE) clean
	make -C $(ETH_OPS_LIB_DIR)

ifeq ($(MAP_R3_SUPPORT),y)
$(BIN): $(OBJS) eth_ops_lib $(CRYPTO_LIB) sp_ut
	$(CC) $(LDFLAGS) $(CFLAGS) -g -o $@ $(OBJS) -L ./ethernet -leth_1905ops $(LDLIBS) -Wl,--start-group,$(CRYPTO_LIB),--end-group

sp_ut: $(EXPORT_LIB)
	cd $(UT_DIR) && $(MAKE)

$(CRYPTO_LIB):
	cd src/crypto && $(MAKE)
else

$(BIN): $(OBJS) eth_ops_lib
	$(CC) $(LDFLAGS) $(CFLAGS)  -g -o $@ $(OBJS) -L ./ethernet -leth_1905ops
endif

$(OBJPATH)/%.o: $(SRCPATH)/%.c eth_ops_lib
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(EXTRA_CFLAGS_AGT)  -o $@  -L ./ethernet -leth_1905ops -c $<

$(EXPORT_LIB) : $(LIB_OBJS) inc/_1905_interface.h inc/common.h lib_export/interface.h lib_export/data_def.h
	ar r $(EXPORT_LIB) $(LIB_OBJS)
$(CTRL_EXEC): $(CTRL_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $(CTRL_EXEC) $(CTRL_OBJS)

$(OBJS): $(OBJPATH)

$(OBJPATH):
	mkdir $(OBJPATH)


-include $(DEPS)

.PHONY: all clean

romfs:
	$(ROMFSINST) /sbin/$(BIN)
	$(ROMFSINST) /sbin/$(CTRL_EXEC)
clean:
	rm -f $(OBJS) $(LIB_OBJS) $(DEPS) $(DEPS_OBJS) $(BIN) $(EXPORT_LIB) $(CTRL_EXEC) $(CTRL_OBJS) $(CTRL_DEPS_OBJS) config.mk.tmp
	rmdir $(OBJPATH)
