.PHONY: test libco

all: libco-test-64 libco-test-32

test: libco all	
	@echo "==== TEST 64 bit ===="
	@LD_LIBRARY_PATH=.. ./libco-test-64
	@echo "==== TEST 32 bit ===="
	@LD_LIBRARY_PATH=.. ./libco-test-32
	
# libco-test-64: main.c

libco:
	@cd .. && make -s

libco-test-64: main.c
	gcc -g -O0 -I.. -L.. -m64 main.c -o libco-test-64 -lco-64

libco-test-32: main.c
	gcc -g -O0 -I.. -L.. -m32 main.c -o libco-test-32 -lco-32

print:	
	@echo LD_LIBRARY_PATH =: $(LD_LIBRARY_PATH)
LD_LIBRARY_PATH=/home/dargon/NJUOperatingSystem/os-workbench/libco
export LD_LIBRARY_PATH


clean:
	rm -f libco-test-*
