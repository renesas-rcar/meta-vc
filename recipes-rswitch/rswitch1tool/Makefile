#
# Copyright (C) 2014, Renesas Electronics Corporation
# All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.



#AS	= $(CROSS_COMPILE)as
#CC = $(CROSS_COMPILE)gcc
#LD = $(CROSS_COMPILE)ld
#CPP	= $(CROSS_COMPILE)g++
#AR	= $(CROSS_COMPILE)ar
#NM	= $(CROSS_COMPILE)nm
#STRIP	= $(CROSS_COMPILE)strip
#SSTRIP	= $(CROSS_COMPILE)sstrip
#OBJCOPY	= $(CROSS_COMPILE)objcopy
#OBJDUMP	= $(CROSS_COMPILE)objdump
#QMAKE	= $(QMAKE_COMPILE)qmake


CP = cp
RM = rm -f

CFLAGS_TOOLCHAIN = -Wno-pointer-sign

APP_NAME := rswitchtool

TOPDIR=$(shell pwd)
PROJDIR=$(TOPDIR)

# Path to Target Kernel Source
KERNEL_PATH = "${TOPDIR}/tmp/work-shared/h3vc/kernel-source/"
#ifndef KERNEL_PATH
 # $(error KERNEL_PATH must point to Target Kernel source)
#endif
INCDIRS = -I$(KERNEL_PATH) 



SRC_FILES := rswitchtool \
			 rswitch_TSN \
             rswitch_FWD \
             mxml-file \
             mxml-private \
             mxml-node \
             mxml-attr \
             mxml-entity \
             mxml-string \
             mxml-get \
             mxml-search\
			 

SRCS := $(SRC_FILES)

OBJDIR_DBG:=Object/Debug
DEPDIR_DBG:=Object/Debug
OBJDIR_REL:=Object/Release
DEPDIR_REL:=Object/Release

LFLAGS  = -lc -lm
LDFLAGS = $(LFLAGS)

INCLUDEFLAGS += -I$(PROJDIR)/. $(INCDIRS)

OSFLAG = -DLINUX -D_GNU_SOURCE -D_REENTRANT

CFLAGS = $(OSFLAG) $(CDEFS) $(WARNING) $(INCLUDEFLAGS) -D_THREAD_SAFE

CFLAGS_DBG = -Wall -ggdb -O $(CFLAGS_TOOLCHAIN)
CFLAGS_REL = -Wall -O $(CFLAGS_TOOLCHAIN)

DBG_OBJS:= $(SRCS:%=$(OBJDIR_DBG)/%.o)
REL_OBJS:= $(SRCS:%=$(OBJDIR_REL)/%.o)
DBG_DEPS:= $(SRCS:%=$(DEPDIR_DBG)/%.d)
REL_DEPS:= $(SRCS:%=$(DEPDIR_REL)/%.d)
TARGETDIR_DBG = Debug
TARGETDIR_REL = Release

#TARGETS:= $(TARGETDIR_DBG)/$(APP_NAME)
TARGETS:= $(TARGETDIR_REL)/$(APP_NAME)

.PHONY: all clean release install debug

all: $(TARGETS)

debug: $(TARGETDIR_DBG)/$(APP_NAME)

release: $(TARGETDIR_REL)/$(APP_NAME)

install:
	sudo cp Release/* /tftpboot/CEES2/usr/bin/

clean:
	@echo clean
	@rm -rf Object $(TARGETDIR_REL) $(TARGETDIR_DBG)

$(TARGETDIR_DBG)/$(APP_NAME): $(DBG_OBJS) Makefile
	@echo linking  $@
	@[ -d $(TARGETDIR_DBG) ] || mkdir $(TARGETDIR_DBG)
	@$(CC) $(DBG_OBJS) $(LDFLAGS) -o $@

$(TARGETDIR_REL)/$(APP_NAME): $(REL_OBJS) Makefile
	@echo linking  $@
	@[ -d $(TARGETDIR_REL) ] || mkdir $(TARGETDIR_REL)
	@$(CC) $(REL_OBJS) $(LDFLAGS) -o $@

$(OBJDIR_DBG)/%.o: %.c Makefile
	@[ -d $(DEPDIR_DBG) ] || mkdir -p $(DEPDIR_DBG)
	@[ -d $(OBJDIR_DBG) ] || mkdir -p $(OBJDIR_DBG)
	@echo compiling $<
	$(CC) -MMD -MF $(DEPDIR_DBG)/$(*F).d -MT $@ $(CFLAGS) $(CFLAGS_DBG) -c $< -o $@
	@[ -s $(DEPDIR_DBG)/$(*F).d ] || rm -f $(DEPDIR_DBG)/$(*F).d

$(OBJDIR_REL)/%.o: %.c Makefile
	@[ -d $(DEPDIR_REL) ] || mkdir -p $(DEPDIR_REL)
	@[ -d $(OBJDIR_REL) ] || mkdir -p $(OBJDIR_REL)
	@echo compiling $<
	$(CC) -MMD -MF $(DEPDIR_REL)/$(*F).d -MT $@ $(CFLAGS) $(CFLAGS_REL) -c $< -o $@
	@[ -s $(DEPDIR_REL)/$(*F).d ] || rm -f $(DEPDIR_REL)/$(*F).d

-include $(DBG_DEPS)
