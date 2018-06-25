##
## Copyright 2003-2011 ANSYS, Inc. 
## All Rights Reserved  
##

#----------------------------------------------------------------------#
# Makefile to call user's makfile for user defined functions.  
# Do not modify this Makefile.
#
# Usage: make "FLUENT_ARCH=arch"
# were arch is ultra, hp700, irix6r8, etc.
#
# sccs id: %W% %G%
#----------------------------------------------------------------------#
SHELL= /bin/sh
FLUENT_ARCH= lnamd64
DIR= $(FLUENT_ARCH)/[23]*
SRC= ../../src/*.{c,h,cpp,hpp} ../../src/makefile ../../src/user.udf

all:
	for d in $(DIR); do \
	  ( \
	    cd $$d; \
		rm -rf *.{c,h,cpp,hpp}; \
	    for f in $(SRC); do \
	      if [ -f $$f -a ! -f `basename $$f` ]; then \
	        echo "# linking to" $$f "in" $$d; \
	        ln -s $$f .; \
	      fi; \
	    done; \
	    echo ""; \
	    echo "# building library in" $$d; \
	    if [ "$(USE_GCC64)" = "1" ]; then \
		echo "# using gcc64"; \
		make ARCHC=gcc64 -k>makelog 2>&1; \
	    else \
		if [ "$(USE_GCC)" = "1" ]; then \
			echo "# using gcc"; \
			make ARCHC=gcc -k>makelog 2>&1; \
		else \
			make -k>makelog 2>&1; \
		fi; \
	    fi;\
	    cat makelog; \
	  ) \
	done

clean:
	for d in $(DIR); do \
	  ( \
            if [ -f "$$d/makefile" ]; then \
	       cd $$d; \
	       make clean; \
	    fi;\
	  ) \
	done
