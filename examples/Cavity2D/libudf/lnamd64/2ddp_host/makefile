##
## Copyright 1987-2023 ANSYS, Inc. All Rights Reserved. 
## All Rights Reserved  
##

#----------------------------------------------------------------------#
# makefile for user defined functions.
#----------------------------------------------------------------------#


#----------------------------------------------------------------------#
# User modifiable section.
#----------------------------------------------------------------------#
include user.udf

# Precompiled User Object files (for example .o files from .f sources)
USER_OBJECTS= $(if $(PRECICE_PATH),$(PRECICE_PATH)/libprecice.so,/usr/local/lib/libprecice.so)


#----------------------------------------------------------------------#
# Build targets (do not modify below this line).
#----------------------------------------------------------------------#
CC=cc
SHELL=/bin/sh
RELEASE=23.2.0
VERSION=`basename "\`pwd\`"`
ARCH=`expr "\`pwd\`" : '.*/\(.*\)/[23].*'`
ARCHC=$(ARCH)
TARGET=libudf.so
UDFDATA=udf_names.c
SOURCES=   $(CSOURCES) $(HSOURCES)
OBJS1 = $(CSOURCES:.cpp=.o)
OBJS2 = $(OBJS1:.c=.o)
OBJECTS= $(UDFDATA:.c=.o) $(OBJS2)
FLUENT_INC = mocked_folders

INCLUDES= -I$(FLUENT_INC)/fluent$(RELEASE)/src/


CFLAGS_LNAMD64=         -D_lnamd64 -D_GNU_SOURCE -fpic -Wno-unused-variable -Wno-unused-but-set-variable -shared -std=c99 -Wall -O -DPTR_RESTRICT=

LDFLAGS_LNAMD64=	-shared -lm



.c.o:
	@echo "# Compiling $@ because of $?"
	@rm -f $*.o
	$(CC) $(CFLAGS) $(HOST_FLAGS) $(INCLUDES) -c $*.c

default: $(UDFDATA)
	@$(MAKE) $(ARCHC)	

$(TARGET): makefile user.udf $(UDFDATA) $(OBJECTS)
	@echo "# Linking $@ because of $?"
	$(CC) $(LDFLAGS) $(OBJECTS) $(USER_OBJECTS) -o $@

$(UDFDATA): makefile $(CSOURCES)
	@echo '# Generating $@ because of $?'
	@echo '/* This file generated automatically. */' > $@
	@echo '/*          Do not modify.            */' >> $@
	@echo '#include "udf.h"' >> $@
	@echo '#include "prop.h"' >> $@
	@echo '#include "dpm.h"' >> $@
	@sed -n 's/^ *\(DEFINE_[_A-Z]* *([^{]*)\)[{ ]*/extern \1;/p' $(CSOURCES) >> $@
	@echo 'UDF_Data udf_data[] = {' >> $@
	@sed -n 's/^ *DEFINE_\([_A-Z]*\) *( *\([_a-zA-Z0-9]*\)[^{]*)[{ ]*/\{"\2", (void (*)(void))\2, UDF_TYPE_\1\},/p' $(CSOURCES) >> $@
	@echo '};' >> $@
	@echo 'int n_udf_data = sizeof(udf_data)/sizeof(UDF_Data);' >> $@
	@echo '#include "version.h"' >> $@
	@echo 'void UDF_Inquire_Release(int *major, int *minor, int *revision)' >> $@
	@echo '{' >> $@
	@echo '  *major = RampantReleaseMajor;' >> $@
	@echo '  *minor = RampantReleaseMinor;' >> $@
	@echo '  *revision = RampantReleaseRevision;' >> $@
	@echo '}' >> $@

$(UDFDATA:.c=.o): $(UDFDATA)
	@echo "# Compiling $@ because of $?"
	@rm -f $*.o
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c


lnamd64:
	$(MAKE) $(TARGET) "CFLAGS=$(CFLAGS_LNAMD64)" "LDFLAGS=$(LDFLAGS_LNAMD64)"



clean:
	@rm -f $(TARGET) $(UDFDATA) $(OBJECTS)

$(OBJECTS): $(HSOURCES)
