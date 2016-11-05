#########################################################################################
# 文 件 名	: Makefile																	#
# 项目名称	: YVGA1207001A																#
# 模 块 名	:																			#
# 功   能	: Makefile																	#
# 操作系统	: LINUX																		#
# 修改记录	: 修复CLEAN的一个BUG															#
# 版   本	: Rev 0.5.2																	#
#--------------------------------------------------------------------------------------	#
# 设   计	: reiaya      '2012-8-10													#
# 编   码	: reiaya      '2012-8-10													#
# 修   改	: reiaya	  '2012-8-22		Rev 0.5.2									#
#########################################################################################
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	#
# 公司介绍及版权说明																		#
#																						#
#               (C)Copyright 2012 YView    Corporation All Rights Reserved.				#
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	#
#########################################################################################

EXE	= 
OBJ	= 	

LIB	= libyv00bsponc.a
OBJ_SO	= libyv00bsponc.so
OBJ_LIB	= 	yv00bsponc_sender.o		yv00bsponc_recver.o		yv00bsponc_packetQueue.o\
			yv00bsponc_util.o		yv00bsponc_netIO.o

OUT_DIR   =										
LIB_DIR   = 				
INSTALL_BIN_DIR = ../../../bin/
INSTALL_LIB_DIR = ../../../lib/

#specify your custmized compile flags here
CFLAGS   += -Wall -I"../../../inc/" -I"." -fPIC
LDFLAGS  += -L"../../../lib" -lpthread

TYPE=

#specify prefix here
PREFIX    =

#HOST	  = arm-none-linux-gnueabi
CC  = gcc
AR	= ar

ifneq ($(HOST),)
HOSTE  	  = $(addsuffix -,$(HOST))
CC     	  = $(addprefix $(HOSTE),gcc)
RANLIB 	  = $(addprefix $(HOSTE),ranlib)
endif

ifeq ($(TYPE), debug)
CFLAGS 	 += -g -DDEBUG
else
ifeq ($(TYPE), release)
CFLAGS   += -O3
else
CFLAGS   += -g
endif
endif

ifndef EXE
ALL    = lib
CLEAN  = $(RM) $(OBJ) $(OBJ_LIB) $(LIB_DIR)$(OBJ_SO)
else
ALL   += $(EXE)
endif
ifndef OBJ_LIB
ALL    = $(EXE)
CLEAN  = $(RM) $(OBJ) $(OUT_DIR)$(EXE)
else
ALL   += lib
endif

CLEAN ?= $(RM) $(OUT_DIR)$(EXE) $(OBJ) $(LIB_DIR)$(LIB) $(LIB_DIR)$(OBJ_SO) $(OBJ_LIB)

all	: $(ALL)
$(EXE)	: $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT_DIR)$(EXE) $^ $(LDFLAGS)
$(LIB)	: $(OBJ_LIB)
	$(AR) rvs $(LIB_DIR)$(LIB) $(OBJ_LIB)

install : $(ALL)
ifneq ($(EXE),)
	cp -f $(EXE) $(INSTALL_BIN_DIR)
endif
ifneq ($(LIB),)
	cp -f $(LIB) $(INSTALL_LIB_DIR)
endif
ifneq ($(OBJ_SO),)
	cp -f $(OBJ_SO) $(INSTALL_LIB_DIR)
endif

lib	  : staticlib sharedlib
staticlib : $(LIB)
sharedlib : $(OBJ_LIB)
	$(CC) $(CFLAGS) -shared -o $(LIB_DIR)$(OBJ_SO) $(OBJ_LIB)

clean:
	$(CLEAN)
