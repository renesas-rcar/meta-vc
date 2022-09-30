#CROSS=aarch64-poky-linux-
#CROSS_SYSROOTS=$(BBPATH)/tmp/sysroots/vc4/
#PATH:=$(PATH):$(BBPATH)/tmp/sysroots/x86_64/usr/bin/aarch64-poky-linux
#
CFLAGS = -Wall -Wno-unused-variable -Wno-sign-compare -O2 -fmax-errors=4
#CC = $(CROSS)g++ --sysroot=$(CROSS_SYSROOTS)

all: rsw2dump

#$(CROSS_SYSROOTS): 
#	cd $(BBPATH) && bitbake build-sysroots

rsw2dump: rsw2dump.c Makefile 
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f rsw2dump
