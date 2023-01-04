PREFIX ?= arm-none-eabi
CFLAGS += -mcpu=cortex-m3 -mthumb -Wno-address-of-packed-member 
LDFLAGS += -mcpu=cortex-m3 -mthumb -march=armv7 -mfix-cortex-m3-ldrd -msoft-float 

export PREFIX
export CFLAGS
export LDFLAGS

.PHONY: all doc FORCE 

all:
	$(MAKE) -C src

doc:
	doxygen Doxyfile

%: FORCE
	$(MAKE) -C src $@
