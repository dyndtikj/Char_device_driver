obj-m := mychrdev.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	-rm -f *.o *.ko *.cmd *.flags *.mod.c *.dwo *.mod
# BINARY     := mychrdev
# KERNEL      := /lib/modules/$(shell uname -r)/build
# C_FLAGS     := -Wall
# KMOD_DIR    := $(shell pwd)
# TARGET_PATH := /lib/modules/$(shell uname -r)/kernel/drivers/char

# ccflags-y += $(C_FLAGS)

# obj-m += $(BINARY).o

# $(BINARY).ko:
# 	make -C $(KERNEL) M=$(KMOD_DIR) modules

# install:
# 	cp $(BINARY).ko $(TARGET_PATH)
# 	depmod -a

# uninstall:
# 	rm $(TARGET_PATH)/$(BINARY).ko
# 	depmod -a

# clean:
# 	make -C $(KERNEL) M=$(KMOD_DIR) clean