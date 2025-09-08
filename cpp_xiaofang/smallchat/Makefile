# ---- config ----------------------------------------------------
CC      := gcc
CXX     := g++
BUILD_DIR := build

TARGET_SERVER  := $(BUILD_DIR)/smallchat_server
TARGET_CLIENT  := $(BUILD_DIR)/smallchat-client


SRCS_SERVER_CPP := smallchat_server.cpp
SRCS_COMMON_C   := chatlib.c
SRCS_CLIENT_C   := smallchat-client.c

OBJS_SERVER := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS_SERVER_CPP)) \
                   $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS_COMMON_C))

OBJS_CLIENT     := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS_CLIENT_C))  \
                   $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS_COMMON_C))

DEPS := $(OBJS_SERVER:.o=.d) $(OBJS_CLIENT:.o=.d)

# 编译参数
CPPFLAGS := -I.
CFLAGS   := -g -O0 -Wall -W -std=c99 -MMD -MP
CXXFLAGS := -g -O0 -Wall -Wextra -std=c++17 -MMD -MP
LDFLAGS  :=
LDLIBS   :=

# ---- targets ---------------------------------------------------
.PHONY: all clean

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# C++ 源 -> .o（到 build/）
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# C 源 -> .o（到 build/）
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# 链接
$(TARGET_SERVER): $(OBJS_SERVER)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# 头文件自动依赖
-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)
