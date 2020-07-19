
ifndef WORKING_DIR
$(error WORKING_DIR is not set)
endif

ifndef BINARY_DIR
$(error BINARY_DIR is not set)
endif

ifndef APPLICATION
$(error APPLICATION is not set)
endif

WORKING_DIR := $(WORKING_DIR)/client
OUTPUT = $(BINARY_DIR)/$(APPLICATION)

SOURCE_DIR = ./src
SOURCES += $(shell find $(SOURCE_DIR) -name '*.c')
OBJECTS += $(patsubst $(SOURCE_DIR)/%.c, $(WORKING_DIR)/%.o, $(SOURCES))

LIB_HEADERS = /usr/include
LIB_PATHS = /usr/lib/x86_64-linux-gnu
LIB_LIST = 
LIBS := $(addprefix -l,$(LIB_LIST))

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
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@ $(LIBS)

$(WORKING_DIR):
	@echo creating $(WORKING_DIR)
	@mkdir -p $(WORKING_DIR)
