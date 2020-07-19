
export WORKING_DIR = $(PWD)/intermediate
export OUTPUT_DIR = $(PWD)/output
export BINARY_DIR = $(OUTPUT_DIR)/bin
export LIBRARY_DIR = $(OUTPUT_DIR)/lib
export LIB_INC_DIR = $(OUTPUT_DIR)/include
export APPLICATION = kbd-lighter

export CC = gcc
export CFLAGS = -Wall -g -std=c18 -D_POSIX_C_SOURCE=200809L

export LIB_HEADERS = /usr/include
export LIB_PATHS = /usr/lib/x86_64-linux-gnu

LDFLAGS += $(addprefix -I,$(LIB_HEADERS))
LDFLAGS += $(addprefix -I,$(LIB_PATHS))
export LDFLAGS

all: setup daemon interface

dbg:
	$(MAKE) -C ./common dbg

daemon: common-lib setup
	@$(MAKE) -C ./server

interface: common-lib setup
	@$(MAKE) -C ./client

common-lib: setup
	@$(MAKE) -C ./common

clean:
	$(MAKE) -C ./server clean
	$(MAKE) -C ./client clean
	@echo cleaning $(WORKING_DIR)
	@rm -d -r -f $(WORKING_DIR)
	@echo cleaning $(BINARY_DIR)
	@rm -d -r -f $(BINARY_DIR)
	@echo cleaning $(LIBRARY_DIR)
	@rm -d -r -f $(LIBRARY_DIR)
	@echo cleaning $(LIB_INC_DIR)
	@rm -d -r -f $(LIB_INC_DIR)
	@echo done

setup: $(WORKING_DIR) $(BINARY_DIR) $(LIBRARY_DIR)

$(WORKING_DIR):
	@echo creating $(WORKING_DIR)
	@mkdir -p $(WORKING_DIR)

$(BINARY_DIR):
	@echo creating $(BINARY_DIR)
	@mkdir -p $(BINARY_DIR)

$(LIBRARY_DIR):
	@echo creating $(LIBRARY_DIR)
	@mkdir -p $(LIBRARY_DIR)

$(LIB_INC_DIR):
	@echo creating $(LIB_INC_DIR)
	@mkdir -p $(LIB_INC_DIR)
