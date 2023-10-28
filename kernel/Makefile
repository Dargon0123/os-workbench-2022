NAME           := kernel
SRCS           := framework/main.c $(shell find -L ./src/ -name "*.c")
INC_PATH       := include/ framework/

export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH    := x86_64-qemu
endif

ifeq ($(smp),)
export smp :=2
endif

COMMA :=,
ifneq ($(test),)
export CFLAGS += $(patsubst %, -DTEST%, $(subst $(COMMA), ,$(test)))
endif


include $(AM_HOME)/Makefile
include ../Makefile.lab
image: git

print:
	@echo AM_HOME: $(AM_HOME)
	@echo SRCS: $(SRCS)
	@echo ARCH: $(ARCH)
	@echo SMP: $(smp)
	@echo CFLAGS: $(CFLAGS)
	@echo test: $(test)

