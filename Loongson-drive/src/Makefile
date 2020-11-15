# Fill in Loongson kernel source code location
KERNELDIR := /home/ufhsykg/Desktop/nmsl/linux-3.10

CURRENT_PATH := $(shell pwd)

obj-m := junbian.o

build: kernel_modules

# Fill in cross compilation tool chain location
kernel_modules:
	$(MAKE)  ARCH=mips CROSS_COMPILE=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-  -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean