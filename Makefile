obj-m := rootkit.o
rootkit-objs := src/module/rootkit.o src/utils/hook.o src/utils/root.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(abspath $(PWD))
BIN_DIR := $(PWD)/bin
TMP_DIR := $(PWD)/tmp
SRC_DIR_UTILS := $(PWD)/src/utils
EXTRA_CFLAGS := -I$(PWD)/include

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I$(PWD)/include
SRC = src/main.c src/utils/logger.o src/utils/personality.o
INC = -I$(PROJECT_DIR)/include
OUT = bin/personality


all: rootkit personality

rootkit:
	make -C $(KDIR) M=$(PWD) modules
	@mv rootkit.ko $(BIN_DIR)/
	@mv *.o *.mod *.mod.c .*.cmd *.symvers *.order $(TMP_DIR)/

personality: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

install:
	sudo insmod bin/rootkit.ko

uninstall:
	sudo rmmod rootkit

clean:
	make -C $(KDIR) M=$(PWD) clean
	@rm -rf $(TMP_DIR)/*
	@rm -f $(BIN_DIR)/rootkit.ko $(BIN_DIR)/compagnon