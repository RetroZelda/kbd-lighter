
ifndef WORKING_DIR
$(error WORKING_DIR is not set)
endif

ifndef LIBRARY_DIR
$(error LIBRARY_DIR is not set)
endif

ifndef LIBRARY
$(error LIBRARY is not set)
endif

WORKING_DIR := $(WORKING_DIR)/common
OUTPUT = $(LIBRARY_DIR)/lib$(LIBRARY).a

SOURCE_DIR = $(LIBRARY_ROOT)/src
INCLUDE_DIR = $(LIBRARY_ROOT)/include
SOURCES += $(shell find $(SOURCE_DIR) -name '*.c')
OBJECTS += $(patsubst $(SOURCE_DIR)/%.c, $(WORKING_DIR)/%.o, $(SOURCES))

HEADERS += $(shell find $(INCLUDE_DIR) -name '*.h')
HEADERS_DEST += $(patsubst $(INCLUDE_DIR)/%.h, $(LIB_INC_DIR)/%.h, $(HEADERS))

LIB_LIST = 
LDFLAGS += $(addprefix -I,$(INCLUDE_DIR))
LIBS := $(addprefix -l,$(LIB_LIST))

AR = gcc-ar
ARFLAGS = rcs
all: $(OUTPUT) $(HEADERS_DEST)

dbg:
	@echo $(HEADERS)
	@echo $(HEADERS_DEST)

clean:
	@echo cleaning $(OBJECTS)
	@rm -f $(OBJECTS)
	@echo cleaning $(WORKING_DIR)
	@rm -d -r -f $(WORKING_DIR)
	@echo cleaning $(OUTPUT)
	@rm -f $(OUTPUT)
	@echo cleaning $(HEADERS_DEST)
	@rm -f $(HEADERS_DEST)
	@echo done

$(OUTPUT): setup $(OBJECTS)
	@echo building $@
	@$(AR) $(ARFLAGS) $@ $(OBJECTS)
	@echo complete

$(LIB_INC_DIR)/%.h: $(INCLUDE_DIR)/%.h
	@echo copying ./$< to $@
	@cp $< $@

setup: $(WORKING_DIR)

$(WORKING_DIR)/%.o: $(SOURCE_DIR)/%.c
	@echo building $@ from $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@ $(LIBS)

$(WORKING_DIR):
	@echo creating $(WORKING_DIR)
	@mkdir -p $(WORKING_DIR)
