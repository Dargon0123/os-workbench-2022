
COMMA :=,
ifneq ($(test),)
export CFLAGS += $(patsubst %, -DTEST%, $(subst $(COMMA), ,$(test)))
endif

objects = foo.o
foo : $(objects)
	gcc -o foo $(objects)

clean:
	rm *.o