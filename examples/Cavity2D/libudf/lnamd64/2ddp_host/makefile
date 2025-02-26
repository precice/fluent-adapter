include user.udf

CC             = cc
TARGET         = libudf.so

SOURCES        = $(CSOURCES) $(HSOURCES)

OBJS1          = $(CSOURCES:.cpp=.o)
OBJS2          = $(OBJS1:.c=.o)

OBJECTS        = $(UDFDATA:.c=.o) $(OBJS2)

CFLAGS_LNAMD64 = -D_lnamd64 -D_GNU_SOURCE -fpic -shared \
                 -Wno-unused-variable -Wno-unused-but-set-variable \
                 -std=c99 -Wall -O -DPARALLEL -DPTR_RESTRICT= \


LDFLAGS_LNAMD64 = -shared -lm \
                
.PHONY: main
main: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "# Linking $(TARGET) from object files"
	$(CC) $(LDFLAGS_LNAMD64) $^ -o $@

%.o: %.c
	@echo "# Compiling $@ from $<"
	$(CC) $(CFLAGS_LNAMD64) -c $< -o $@

$(UDFDATA):
	@echo "# Generating $(UDFDATA) if needed"
	# Example:
	# echo "/* stub file for udf_names */" > $(UDFDATA)
	# echo "#include \"udf.h\"" >> $(UDFDATA)
	# echo "/* ... etc. ... */" >> $(UDFDATA)

.PHONY: clean
clean:
	@echo "# Cleaning build artifacts"
	rm -f $(TARGET) $(OBJECTS)
