
export WORKING_DIR = $(PWD)/intermediate
export OUTPUT_DIR = $(PWD)/output
export BINARY_DIR = $(OUTPUT_DIR)/bin
export LIBRARY_DIR = $(OUTPUT_DIR)/lib
export LIB_INC_DIR = $(OUTPUT_DIR)/include
export INTERFACE = kbd-lighter
export DAEMON = $(INTERFACE)d
export LIBRARY = kbd-common

export CC = gcc
export CFLAGS = -Wall -g -std=c18 -D_POSIX_C_SOURCE=200809L

export LIB_HEADERS = /usr/include $(LIB_INC_DIR)
export LIB_PATHS = /usr/lib/x86_64-linux-gnu $(LIBRARY_DIR)

LDFLAGS += $(addprefix -I,$(LIB_HEADERS))
LDFLAGS += $(addprefix -I,$(LIB_PATHS))
export LDFLAGS

export DAEMON_ROOT 		= $(PWD)/server
export LIBRARY_ROOT 	= $(PWD)/common
export INTERFACE_ROOT 	= $(PWD)/client

all: setup $(LIBRARY) $(DAEMON) $(INTERFACE)

dbg:
	@$(MAKE) -C $(DAEMON_ROOT) dbg
	@$(MAKE) -C $(INTERFACE_ROOT) dbg
	@$(MAKE) -C $(LIBRARY_ROOT) dbg

$(DAEMON): $(LIBRARY)
	@$(MAKE) -C $(DAEMON_ROOT)

$(INTERFACE): $(LIBRARY)
	@$(MAKE) -C $(INTERFACE_ROOT)

$(LIBRARY): setup
	@$(MAKE) -C $(LIBRARY_ROOT)

clean:
	@$(MAKE) -C $(DAEMON_ROOT) clean
	@$(MAKE) -C $(INTERFACE_ROOT) clean
	@$(MAKE) -C $(LIBRARY_ROOT) clean
	@echo cleaning $(WORKING_DIR)
	@rm -d -r -f $(WORKING_DIR)
	@echo cleaning $(BINARY_DIR)
	@rm -d -r -f $(BINARY_DIR)
	@echo cleaning $(LIBRARY_DIR)
	@rm -d -r -f $(LIBRARY_DIR)
	@echo cleaning $(LIB_INC_DIR)
	@rm -d -r -f $(LIB_INC_DIR)
	@echo cleaning $(OUTPUT_DIR)
	@rm -d -r -f $(OUTPUT_DIR)
	@echo done

setup: $(WORKING_DIR) $(BINARY_DIR) $(LIBRARY_DIR) $(LIB_INC_DIR)

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
