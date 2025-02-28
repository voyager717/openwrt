ifneq ($(CONFIG_SUPPORT_OPENWRT),)

obj-$(CONFIG_MAP_FILTER_SUPPORT) := mapfilter.o
mapfilter-objs = mtkmapfilter.o load_balance.o traffic_separation.o service_prioritization.o dual_bh.o

ccflags-y = -Wall -Wstrict-prototypes -Wno-trigraphs -Wframe-larger-than=4096 -Werror

else

PWD = $(shell pwd)
KERNEL_DIR = $(PWD)/../../$(LINUXDIR)
ROMFS_DIR = $(PWD)/../../romfs
#@echo $(PWD);
#ccflags-y := -I../proprietary_driver/drivers/cryptodev
mapfilter-objs = mtkmapfilter.o load_balance.o traffic_separation.o service_prioritization.o dual_bh.o
obj-m := mapfilter.o

modules:
	make -C $(KERNEL_DIR) M=$(PWD) modules

modules_install:
	make INSTALL_MOD_PATH=$(ROMFS_DIR) -C $(KERNEL_DIR) M=$(PWD) modules_install

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

endif