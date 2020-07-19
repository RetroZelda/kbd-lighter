
ifndef WORKING_DIR
$(error WORKING_DIR is not set)
endif

ifndef LIBRARY_DIR
$(error LIBRARY_DIR is not set)
endif

ifndef APPLICATION
$(error APPLICATION is not set)
endif

WORKING_DIR := $(WORKING_DIR)/common
OUTPUT = $(LIBRARY_DIR)/lib$(APPLICATION)-common.a

SOURCE_DIR = ./src
INCLUDE_DIR = ./include
SOURCES += $(shell find $(SOURCE_DIR) -name '*.c')
OBJECTS += $(patsubst $(SOURCE_DIR)/%.c, $(WORKING_DIR)/%.o, $(SOURCES))

HEADERS += $(shell find $(INCLUDE_DIR) -name '*.h')

LIB_LIST =
CFLAGS  = -c
LDFLAGS := $(addprefix -I,$(INCLUDE_DIR))
LIBS := $(addprefix -l,$(LIB_LIST))

AR = gcc-ar
ARFLAGS = rcs
all: $(OUTPUT)

dbg:
	@echo $(HEADERS)

clean:
	@echo cleaning $(OBJECTS)
	@rm -f $(OBJECTS)
	@echo cleaning $(WORKING_DIR)
	@rm -d -r -f $(WORKING_DIR)
	@echo cleaning $(OUTPUT)
	@rm -f $(OUTPUT)
	@echo cleaning $(HEADERS)
	@rm -f $(HEADERS)
	@echo done

$(OUTPUT): setup $(OBJECTS)
	@echo building ./$@
	#@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	@echo copying includes
	@cp $(INCLUDE_DIR)/* $(LIB_INC_DIR)
	@echo complete

setup: $(WORKING_DIR)

$(WORKING_DIR)/%.o: $(SOURCE_DIR)/%.c
	@echo building $@ from $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@ $(LIBS)

$(WORKING_DIR):
	@echo creating $(WORKING_DIR)
	@mkdir -p $(WORKING_DIR)
