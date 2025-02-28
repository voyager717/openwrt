EXE = fwdd
OBJS = fwdd.o

#stdlibc++ known issue:
#If your program  needs libstdc++, please turn off code optimization
CFLAGS += -Wall -Werror -O2

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

romfs:
	$(ROMFSINST) /bin/$(EXE)

clean:
	@rm -rf $(EXE)  *.o

