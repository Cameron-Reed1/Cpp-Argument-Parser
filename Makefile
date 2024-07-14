TYPE = STATIC
SHARED_TYPE = SHARED
STATIC_TYPE = STATIC

SOURCE_DIR = Src
HEADER_DIR = Inc
BUILD_DIR = bin

INSTALL_PREFIX ?= /usr/local
HEADER_INSTALL_DIR = $(INSTALL_PREFIX)/include
INSTALL_DIR = $(INSTALL_PREFIX)/lib

LIB_NAME = argParser
ifeq ($(TYPE), $(SHARED_TYPE))
LIB_FILE_NAME = lib$(LIB_NAME).so
else
LIB_FILE_NAME = lib$(LIB_NAME).a
endif

OPT = -O2

INCS = -IInc

C_SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
CXX_SOURCES = $(wildcard $(SOURCE_DIR)/*.cpp)

OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CXX_SOURCES:.cpp=.o)))

CFLAGS = $(OPT) $(INCS) -Wall
CXXFLAGS = -std=c++17 $(OPT) $(INCS) -Wall

ifeq ($(TYPE), $(SHARED_TYPE))
CFLAGS += -fpic
CXXFLAGS += -fpic

TESTFLAGS = -Wl,-rpath=$(PWD)/$(BUILD_DIR)
endif

HEADERS = $(wildcard $(HEADER_DIR)/*.h)

ARFLAGS = rvc

all: $(BUILD_DIR)/$(LIB_FILE_NAME)

ifneq ($(TYPE), $(SHARED_TYPE))
shared: clean
    @sed -i -e "s/^TYPE = .*$$/TYPE = $(SHARED_TYPE)/" Makefile

.PHONY: shared
endif

ifneq ($(TYPE), $(STATIC_TYPE))
static: clean
    @sed -i -e "s/^TYPE = .*$$/TYPE = $(STATIC_TYPE)/" Makefile

.PHONY: static
endif

$(BUILD_DIR)/$(LIB_FILE_NAME): $(OBJECTS)
ifeq ($(TYPE), $(SHARED_TYPE))
    $(CXX) $^ $(CXXFLAGS) -shared -o $@
else
    $(AR) $(ARFLAGS) $@ $^
endif

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
    $(CC) -c $< $(CFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
    $(CXX) -c $< $(CXXFLAGS) -o $@

$(BUILD_DIR):
    mkdir -p $@

test: test.cpp all | $(BUILD_DIR)
    $(CXX) $< $(CXXFLAGS) -l$(LIB_NAME) -L$(BUILD_DIR) $(TESTFLAGS) -o $(BUILD_DIR)/test
    -$(BUILD_DIR)/test

test_installed: test.cpp | $(BUILD_DIR)
    $(CXX) $< $(CXXFLAGS) -l$(LIB_NAME) -o $(BUILD_DIR)/test
    -$(BUILD_DIR)/test

install: all
    cp $(BUILD_DIR)/$(LIB_FILE_NAME) $(INSTALL_DIR)
    cp $(HEADERS) $(HEADER_INSTALL_DIR)
    @chmod 0755 $(INSTALL_DIR)/$(LIB_FILE_NAME)
ifeq ($(TYPE), $(SHARED_TYPE))
    @ldconfig
endif

uninstall:
    $(RM) $(INSTALL_DIR)/$(LIB_FILE_NAME)
    $(RM) $(addprefix $(HEADER_INSTALL_DIR)/, $(notdir $(HEADERS)))
ifeq ($(TYPE), $(SHARED_TYPE))
    @ldconfig
endif

clean:
    $(RM) -r $(BUILD_DIR)

.PHONY: all test install uninstall clean

