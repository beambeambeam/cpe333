# Run inside the Linux VM (./vm.sh shell, or ./vm.sh make <target> from macOS).
BIG := /tmp/pfcount-big.bin

.PHONY: all module faultgen load unload reload demo clean

all: module faultgen

module:
	$(MAKE) -C module

faultgen: tests/faultgen
tests/faultgen: tests/faultgen.c
	$(CC) -O2 -Wall -o $@ $<

load: module
	sudo insmod module/pfcount.ko
	@ls /proc/pfcount

unload:
	-sudo rmmod pfcount

reload: unload load

$(BIG):
	dd if=/dev/urandom of=$@ bs=1M count=32 status=none

# Run faultgen under pfstat: 64 MB anon touch -> minor faults,
# 32 MB file read with cold cache -> major faults.
demo: all $(BIG)
	@lsmod | grep -q '^pfcount' || sudo insmod module/pfcount.ko
	sudo tools/pfstat -w 0.5 -- tests/faultgen sleep:1 anon:64 sleep:1 file:$(BIG) sleep:1 free sleep:1

clean:
	$(MAKE) -C module clean
	rm -f tests/faultgen $(BIG)
