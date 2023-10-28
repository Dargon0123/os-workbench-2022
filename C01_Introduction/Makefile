NAME := hello
SRCS := main.c say.c
export MODULE  := L0
export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH := x86_64-qemu
endif
include $(AM_HOME)/Makefile

# clean:
# 	rm *.o *.out