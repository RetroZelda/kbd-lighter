ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
export WORKING_DIR = $(ROOT_DIR)/intermediate
export OUTPUT_DIR = $(ROOT_DIR)/output
export BINARY_DIR = $(OUTPUT_DIR)/bin
export LIBRARY_DIR = $(OUTPUT_DIR)/lib
export LIB_INC_DIR = $(OUTPUT_DIR)/include
export INTERFACE = kbd-lighter
export DAEMON = $(INTERFACE)d
export LIBRARY = kbd-common
export INSTALL_DIR= /usr/local/bin

export CC = gcc
export CFLAGS = -Wall -g -std=c18 -D_POSIX_C_SOURCE=200809L

export LIB_HEADERS = /usr/include $(LIB_INC_DIR)
export LIB_PATHS = /usr/lib/x86_64-linux-gnu $(LIBRARY_DIR)

LDFLAGS += $(addprefix -I,$(LIB_HEADERS))
LDFLAGS += $(addprefix -I,$(LIB_PATHS))
export LDFLAGS

export DAEMON_ROOT 		= $(ROOT_DIR)/daemon
export LIBRARY_ROOT 	= $(ROOT_DIR)/common
export INTERFACE_ROOT 	= $(ROOT_DIR)/application

export CALLING_USER		= $(shell logname)

.PHONY: $(DAEMON) $(INTERFACE) $(LIBRARY) clean dbg setup daemon application common install uninstall

all: setup $(LIBRARY) $(DAEMON) $(INTERFACE)

dbg:
	#@$(MAKE) -C $(DAEMON_ROOT) dbg
	#@$(MAKE) -C $(INTERFACE_ROOT) dbg
	#@$(MAKE) -C $(LIBRARY_ROOT) dbg
	#$(PWD)
	#$(ROOT_DIR)
	#$(CALLING_USER)

install: 
	@echo installing to $(INSTALL_DIR)...
	@cp $(BINARY_DIR)/$(INTERFACE) $(INSTALL_DIR)/$(INTERFACE)
	@cp $(BINARY_DIR)/$(DAEMON) $(INSTALL_DIR)/$(DAEMON)
	@echo making executable...
	@chmod +x $(INSTALL_DIR)/$(INTERFACE)
	@chmod +x $(INSTALL_DIR)/$(DAEMON)
	@echo enabling sudo override...
	@touch /etc/sudoers.d/$(INTERFACE)
	@echo "$(CALLING_USER) ALL=(ALL) NOPASSWD: $(INSTALL_DIR)/$(INTERFACE)" > /etc/sudoers.d/$(INTERFACE)
	@chmod 0440 /etc/sudoers.d/$(INTERFACE)
	@echo done

uninstall:
	@echo uninstalling $(INSTALL_DIR)/$(INTERFACE)...
	@rm -f $(INSTALL_DIR)/$(INTERFACE)
	@echo uninstalling $(INSTALL_DIR)/$(DAEMON)...
	@rm -f $(INSTALL_DIR)/$(DAEMON)
	@echo removing sudo override...
	@rm -f /etc/sudoers.d/$(INTERFACE)
	@echo done

daemon: $(DAEMON)
application: $(INTERFACE)
common: $(LIBRARY)

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
