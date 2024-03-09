# NOTE(wittenbb): Reference:
# <https://stackoverflow.com/questions/714100/os-detecting-makefile>


ifeq ($(OS),Windows_NT)
	$(error We in S21 doesn't use Windows...)
else
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CFLAGS += -D OS_LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -D OS_OSX
    endif
endif
