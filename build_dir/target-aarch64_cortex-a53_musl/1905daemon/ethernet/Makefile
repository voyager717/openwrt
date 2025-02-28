include ./config.mk
ETH_OPS_LIB = libeth_1905ops.so
INC =
ETH_OPS_LIB_OBJS = ./default/default_eth_ops.o

ifeq ($(SWITCH_TYPE), mediatek)
	ifneq ($(CONFIG_SOC_MT7621)$(CONFIG_RALINK_MT7621)$(CONFIG_MT7622_7530_KT)$(CONFIG_MT753X_GSW)$(CONFIG_RALINK_MT7628),)
		ETH_OPS_LIB_OBJS := ./mediatek/switch_legacy_ioctl.o ./mediatek/switch_layer.o
		ifeq ($(CONFIG_MT753X_GSW),y)
			ETH_OPS_LIB_OBJS += ./mediatek/switch_mt753x_nl.o ./mediatek/bridge_parse.o
		endif
	endif
else ifeq ($(SWITCH_TYPE), mediatek_dsa)
	INC := ./mediatek_DSA/libbridge
	ETH_OPS_LIB_OBJS := ./mediatek_DSA/eth_ops_dsa.o
	EXTRA_LIB = ./mediatek_DSA/libbridge/libbridge.a
	LIBDIR = ./mediatek_DSA/libbridge
else
#add other ethernet vendors' solution here
#assign an new value to ETH_OPS_LIB_OBJS
endif

$(warning $(ETH_OPS_LIB_OBJS))
all: $(ETH_OPS_LIB) $(ETH_OPS_LIB_OBJS) $(EXTRA_LIB)

ifeq ($(SWITCH_TYPE), mediatek_dsa)
$(EXTRA_LIB):
	make -C $(LIBDIR)
endif

$(ETH_OPS_LIB_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I../inc -I. -I$(INC) -fPIC -shared -c $< -o $@

$(ETH_OPS_LIB): $(ETH_OPS_LIB_OBJS) $(EXTRA_LIB)
	$(CC) $(CFLAGS) -I../inc -I. -I$(INC) -fPIC -shared -o $@ $^

clean:
	rm -f $(ETH_OPS_LIB_OBJS)
	rm -f $(ETH_OPS_LIB)