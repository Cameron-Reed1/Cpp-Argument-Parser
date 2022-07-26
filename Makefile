BUILD_DIR = bin
SOURCE_DIR = Src
HEADER_DIR = Inc
HEADER_INSTALL_DIR = /usr/local/include
INSTALL_DIR = /usr/local/lib

LIB_NAME = argParser
LIB_FILE_NAME = lib$(LIB_NAME).a

OPT = -O2

CC = gcc
CXX = g++

INCS = \
-IInc

C_SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
CXX_SOURCES = $(wildcard $(SOURCE_DIR)/*.cpp)

OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/, $(notdir $(CXX_SOURCES:.cpp=.o)))

CFLAGS = $(OPT) $(INCS) -Wall
CXXFLAGS = -std=c++17 $(OPT) $(INCS) -Wall

HEADERS = $(wildcard $(HEADER_DIR)/*.h)

CP = cp
MKDIR = mkdir -p
ARFLAGS = rvc

all: $(BUILD_DIR)/$(LIB_FILE_NAME)

$(BUILD_DIR)/$(LIB_FILE_NAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $< $(CFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(BUILD_DIR):
	$(MKDIR) $@

test: all
	$(CXX) test.cpp $(CXXFLAGS) -l$(LIB_NAME) -L$(BUILD_DIR) -o $(BUILD_DIR)/test
	-$(BUILD_DIR)/test

test_installed:
	$(CXX) test.cpp $(CXXFLAGS) -l$(LIB_NAME) -o $(BUILD_DIR)/test
	-$(BUILD_DIR)/test

install:
	$(CP) $(BUILD_DIR)/$(LIB_FILE_NAME) $(INSTALL_DIR)
	$(CP) $(HEADERS) $(HEADER_INSTALL_DIR)

uninstall:
	$(RM) $(INSTALL_DIR)/$(LIB_FILE_NAME)
	$(RM) $(addprefix $(HEADER_INSTALL_DIR)/, $(notdir $(HEADERS)))

clean:
	$(RM) -r $(BUILD_DIR)

.PHONY: test install uninstall clean
