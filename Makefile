THIS_MAKE_FILE     := $(abspath $(lastword $(MAKEFILE_LIST)))
THIS_MAKE_FILE_DIR := $(realpath $(patsubst %/,%,$(dir $(THIS_MAKE_FILE))))

SRCROOT     := $(THIS_MAKE_FILE_DIR)

CONFIG_FILE ?= $(SRCROOT)/.config

ifeq ($(shell test -e $(CONFIG_FILE) && echo -n yes),yes)
	include $(SRCROOT)/.config
endif


PHONY := \
	default all build \
	format fmt lint \
	test tests \
	clean clean_external \
	veryclean


RM    := rm -rf
ALL   :=
CLEAN :=

EXTERNALS := $(SRCROOT)/external

GLFW_DIR  := $(EXTERNALS)/glfw
GLFW_BIN  := $(GLFW_DIR)/build/src/libglfw3.a

GLAD_DIR  := $(EXTERNALS)/glad
GLAD_BIN  := $(GLAD_DIR)/libglad.a

CGLM_DIR  := $(EXTERNALS)/cglm
CGLM_BIN  := $(CGLM_DIR)/build/libcglm.a

default: all

CC       := gcc
CFLAGS   := -Wall -Wextra -pedantic -std=c2x
LDFLAGS  := -lm
INCFLAGS := \
	-I $(SRCROOT)/src \
	-I $(SRCROOT)/include \
	-I $(GLFW_DIR)/include \
	-I $(GLAD_DIR)/include \
	-I $(CGLM_DIR)/include


AR       := ar
ARFLAGS  := rcs

ifeq ($(BUILD_CONFIG), DEBUG)
    CFLAGS += -O0 -g -D BUILD_CONFIG_DEBUG
else
    CFLAGS += -Werror -O2 -D BUILD_CONFIG_RELEASE
endif

ifeq ($(USE_SANITIZE), ADDRESS)
   CFLAGS += -fsanitize=address
endif

ifeq ($(USE_SANITIZE), UNDEFINE)
    CFLAGS += -fsanitize=undefine
endif

ifeq ($(USE_SANITIZE), LEAK)
    CFLAGS += -fsanitize=leak
endif


include $(SRCROOT)/mk/os.mk
include $(SRCROOT)/mk/tools.mk

BREAKOUT_SRC   := $(SRCROOT)/src
BREAKOUT_BIN   := $(SRCROOT)/breakout

BREAKOUT_SRCS  := \
	$(BREAKOUT_SRC)/main.c \
	$(BREAKOUT_SRC)/io.c \
	$(BREAKOUT_SRC)/shader.c \

BREAKOUT_OBJS  := $(patsubst $(BREAKOUT_SRC)/%.c, $(BREAKOUT_SRC)/%.o, $(BREAKOUT_SRCS))

BREAKOUT_DEPS  := \
	$(GLFW_BIN) \
	$(GLAD_BIN) \
	$(CGLM_BIN)


$(BREAKOUT_BIN): $(BREAKOUT_OBJS) $(BREAKOUT_DEPS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


$(BREAKOUT_SRC)/%.o: $(BREAKOUT_SRC)/%.c
	$(CC) $(CFLAGS) $(INCFLAGS) -c $^ -o $@


$(CGLM_BIN):
	cmake \
		-B $(CGLM_DIR)/build \
		-S $(CGLM_DIR) \
		-D CGLM_STATIC=true
	cmake --build $(CGLM_DIR)/build

$(GLFW_BIN):
	cmake \
		-B $(GLFW_DIR)/build \
		-S $(GLFW_DIR)\
		-D GLFW_BUILD_TESTS=false\
		-D GLFW_BUILD_EXAMPLES=false\
		-D GLFW_BUILD_DOCS=false\
		-D GLFW_BUILD_WAYLAND=true
	cmake --build $(GLFW_DIR)/build


$(GLAD_BIN): $(GLAD_DIR)/src/glad.o
	$(AR) $(ARFLAGS) $@ $^

$(GLAD_DIR)/src/glad.o: $(GLAD_DIR)/src/glad.c
	$(CC) -I $(GLAD_DIR)/include -c $^ -o $@


ALL     += $(BREAKOUT_BIN)
CLEAN   += $(BREAKOUT_OBJS) $(BREAKOUT_BIN)


all: $(ALL)


build: $(BREAKOUT_BIN)


clean:
	$(RM) $(CLEAN)


clean_external:
	$(RM) \
		$(GLFW_DIR)/build \
		$(GLAD_BIN) \
		$(GLAD_DIR)/src/glad.o


veryclean: clean clean_external


lint: run-clang-format run-cppcheck

format fmt: run-clang-reformat

.PHONY: $(PHONY)
