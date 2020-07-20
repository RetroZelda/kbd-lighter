
ifndef WORKING_DIR
$(error WORKING_DIR is not set)
endif

ifndef BINARY_DIR
$(error BINARY_DIR is not set)
endif

ifndef DAEMON
$(error DAEMON is not set)
endif

WORKING_DIR := $(WORKING_DIR)/server
OUTPUT = $(BINARY_DIR)/$(DAEMON)

SOURCE_DIR = $(DAEMON_ROOT)/src
SOURCES += $(shell find $(SOURCE_DIR) -name '*.c')
OBJECTS += $(patsubst $(SOURCE_DIR)/%.c, $(WORKING_DIR)/%.o, $(SOURCES))

LIB_LIST = X11 m Xext Xi pulse pulse-simple 
LIBS := $(addprefix -l,$(LIB_LIST))
LIBS += $(LIBRARY_DIR)/lib$(LIBRARY).a

CFLAGS  += -pthread

all: $(OUTPUT)

dbg:
	@echo $(OBJECTS)

clean:
	@echo cleaning $(OBJECTS)
	@rm -f $(OBJECTS)
	@echo cleaning $(WORKING_DIR)
	@rm -d -r -f $(WORKING_DIR)
	@echo cleaning $(OUTPUT)
	@rm -f $(OUTPUT)
	@echo done

$(OUTPUT): setup $(OBJECTS)
	@echo building ./$@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)
	@echo complete

setup: $(WORKING_DIR)

$(WORKING_DIR)/%.o: $(SOURCE_DIR)/%.c
	@echo building $@ from $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

$(WORKING_DIR):
	@echo creating $(WORKING_DIR)
	@mkdir -p $(WORKING_DIR)