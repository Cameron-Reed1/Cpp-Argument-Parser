TYPE            := STATIC
SHARED_TYPE     := SHARED
STATIC_TYPE     := STATIC


SRC_DIRS        := src
INC_DIRS        := inc


BUILD_DIR       := build
OFILE_DIR       := $(BUILD_DIR)/objects
INSTALL_PREFIX  ?= /usr
INC_INSTALL_DIR := $(INSTALL_PREFIX)/include
LIB_INSTALL_DIR := $(INSTALL_PREFIX)/lib
PC_INSTALL_DIR  := $(LIB_INSTALL_DIR)/pkgconfig



LIB_NAME        := argParser
PC_FILE         := $(LIB_NAME).pc
ifeq ($(TYPE), $(SHARED_TYPE))
LIB_FILE_NAME   := lib$(LIB_NAME).so
else
LIB_FILE_NAME   := lib$(LIB_NAME).a
endif



INCLUDES        := $(addprefix -I, $(INC_DIRS))
C_SOURCES       := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
CXX_SOURCES     := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))
HEADERS         := $(foreach dir, $(INC_DIRS), $(wildcard $(dir)/*.h) $(wildcard $(dir)/*.hpp))


OFILES          := $(addprefix $(OFILE_DIR)/, $(notdir $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cpp=.o)))


OPT             := -O2
CPPFLAGS        := $(INCLUDES) -MMD
CFLAGS          := $(OPT) -std=c11 -Wall -Wextra -Wpedantic
CXXFLAGS        := $(OPT) -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS         := 
LDLIBS          := 
ARFLAGS         := rcs


DEPENDS         := $(OFILES:.o=.d)


DIRS            := $(BUILD_DIR) $(OFILE_DIR)


ifeq ($(TYPE), $(SHARED_TYPE))
CFLAGS    += -fpic
CXXFLAGS  += -fpic
LDFLAGS   += -shared
TESTFLAGS := -Wl,-rpath=$(PWD)/$(BUILD_DIR)
endif

ifeq ($(strip $(CXX_SOURCES)),)
LD := $(CC)
else
LD := $(CXX)
endif


define PKG_CONFIG
prefix=$(INSTALL_PREFIX)
exec_prefix=$${prefix}
includedir=$${prefix}/include
libdir=$${exec_prefix}/lib

Name: $(LIB_NAME)
Description: The argParser library
Version: 2.0.0
Cflags: -I$${includedir}
Libs: -L$${libdir} -l$(LIB_NAME)
endef


.PHONY: all test install uninstall clean $(BUILD_DIR)/$(PC_FILE)

all: $(BUILD_DIR)/$(LIB_FILE_NAME)

ifneq ($(TYPE), $(SHARED_TYPE))
.PHONY: shared

shared: clean
	@sed -i -e "s/^TYPE\( \+\):= .*$$/TYPE\1:= $(SHARED_TYPE)/" Makefile
else
.PHONY: static

static: clean
	@sed -i -e "s/^TYPE\( \+\):= .*$$/TYPE\1:= $(STATIC_TYPE)/" Makefile
endif

install: all $(BUILD_DIR)/$(PC_FILE)
	install -D -m 0755 $(BUILD_DIR)/$(LIB_FILE_NAME) -t $(LIB_INSTALL_DIR)
	install -D -m 0644 $(BUILD_DIR)/$(PC_FILE) -t $(PC_INSTALL_DIR)
	install -D -m 0644 $(HEADERS) -t $(INC_INSTALL_DIR)
ifeq ($(TYPE), $(SHARED_TYPE))
	@ldconfig
endif

uninstall:
	$(RM) $(LIB_INSTALL_DIR)/$(LIB_FILE_NAME)
	$(RM) $(PC_INSTALL_DIR)/$(PC_FILE)
	$(RM) $(addprefix $(INC_INSTALL_DIR)/, $(notdir $(HEADERS)))
ifeq ($(TYPE), $(SHARED_TYPE))
	@ldconfig
endif

clean:
	$(RM) -r $(DIRS)


-include $(DEPENDS)



$(BUILD_DIR)/lib$(LIB_NAME).so: $(OFILES) | $(BUILD_DIR)
	$(LD) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/lib$(LIB_NAME).a: $(OFILES) | $(BUILD_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)/$(PC_FILE): | $(BUILD_DIR)
	$(file > $@,$(PKG_CONFIG))


$(OFILE_DIR)/%.o: %.c | $(OFILE_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OFILE_DIR)/%.o: %.cpp | $(OFILE_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@



$(DIRS):
	mkdir -p $@



test: test.cpp all | $(BUILD_DIR)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $< -L$(BUILD_DIR) -l$(LIB_NAME) $(TESTFLAGS) -o $(BUILD_DIR)/test
	-$(BUILD_DIR)/test

test_installed: test.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -l$(LIB_NAME) -o $(BUILD_DIR)/test
	-$(BUILD_DIR)/test




vpath %.c $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)
