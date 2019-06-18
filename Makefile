
CC=clang
LDFLAGS=-framework IOKit -framework DirectHW -F/Library/Frameworks
all: rdmem wrmem rdpci lnkspd

rdmem: rdmem.o 
	$(CC) -o $@ $^  $(LDFLAGS)
wrmem: wrmem.o 
	$(CC) -o $@ $^ $(LDFLAGS)

rdpci: rdpci.o
	$(CC) -o $@ $^ $(LDFLAGS)

lnkspd: lnkspd.o 
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) *.o .*.d rdmem wrmem lnkspd rdpci

CFLAGS = \
	-g \
	-O3 \
	-W \
	-Wall \
	-MMD \
	-MF .$(notdir $@).d \
	-F/Library/Frameworks

-include .*.d
