ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif
CFLAGS = -Werror -g


shared: all
	$(INSTALL) -d ../lib
	$(INSTALL) -m 644 libmapd_interface_client.so ../lib


all:  libmapd_interface_client.so


mapd_interface_ctrl.o : mapd_interface_ctrl.c 
	$(CC) -I ./include -c -fPIC $^ -o $@ $(CFLAGS)
	
include/os_unix_lib.o : include/os_unix.c
	$(CC) -I ./include -c -fPIC $^ -o $@
	
libmapd_interface_client.so: mapd_interface_ctrl.o include/os_unix_lib.o
	$(CC) -shared -o $@ $^


clean:
	$(MAKE) -C ./include clean
	rm -f libmapd_interface_client.so
	rm -f core *~ *.o *.d *.gcno *.gcda *.gcov *.a
	rm -f $(ALL)

