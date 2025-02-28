SP_UT_BIN = sp_ut
ETH_OPS_LIB_OBJS += ../service_prioritization.o ../common.o ../os_internal.o ../mapfilter_if.o sp_ut.o 


CFLAGS += -fPIC -lpthread $(INCLUDES) -Isrc/chips -Iethernet -g -Ilib_export -Ilib_wapp -Ilib -Wall -Werror -MMD -O2 -DVERSION=\"$(VERSION)\"
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)
EXTRA_CFLAGS += -DSUPPORT_COREDUMP
CFLAGS += -D_GNU_SOURCE -I$(STAGING_DIR)/usr/include/libnl-tiny
EXTRA_CFLAGS+= -DSUPPORT_WIFI
EXTRA_CFLAGS+= -DSUPPORT_ETH
EXTRA_CFLAGS += -DMAP_R2
EXTRA_CFLAGS += -DCENT_STR
EXTRA_CFLAGS += -DMAP_R3_SP
EXTRA_CFLAGS+= -DSUPPORT_AP_AUTO_CONFIG
EXTRA_CFLAGS+= -DSUPPORT_CMDU_RELIABLE

$(warning $(ETH_OPS_LIB_OBJS))
all: $(SP_UT_BIN) $(ETH_OPS_LIB_OBJS)

$(ETH_OPS_LIB_OBJS): %.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I../../inc -I../../lib_export -I../../lib_wapp -I../../ethernet -I. -c $< -o $@ ../../lib_export/lib1905_interface.a

$(SP_UT_BIN): $(ETH_OPS_LIB_OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I../../inc -I../../lib_export -I../../lib_wapp -I../../ethernet -I. -o $@ $^ ../../lib_export/lib1905_interface.a
	
clean:
	rm -f $(ETH_OPS_LIB_OBJS)
	rm -f $(SP_UT_BIN)