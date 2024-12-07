#!/bin/bash
set -e

echo "
______            _   _    _ _     _____   ____ _____  _____ 
| ___ \          | | | |  (_) |   / __  \ / ___|  _  ||  _  |
| |_/ /___   ___ | |_| | ___| |_  \- / /'/ /___| |/' || |/' |
|    // _ \ / _ \| __| |/ / | __|   / /  | ___ \  /| ||  /| |
| |\ \ (_) | (_) | |_|   <| | |_  ./ /___| \_/ \ |_/ /\ |_/ /
\_| \_\___/ \___/ \__|_|\_\_|\__| \_____/\_____/\___/  \___/ 
                                                             
                                                             
                      _     ______ _____                     
                     | |    |  ___/  ___|                    
                     | |    | |_  \ |--.                     
                     | |    |  _|  \--. \                    
                     | |____| |   /\__/ /                    
                     \_____/\_|   \____/                     
                                                                                                                    
"

losetup -a | grep disk.img | cut -d ':' -f1 | while read loop; do
    sudo rm $loop
done

KERNEL_PATH="vmlinuz-6.1.0-27-amd64"
ROOTKIT_DIR="/home/lyzi/exo/A2-S1/rootkit"
DISK_IMG="disk.img"
DISK_SIZE="6.5G"
ROOTFS_DIR="/tmp/my-rootfs"
LOOP_DEVICE="/dev/loop0"
DEBIAN_RELEASE="stable"

echo "Creating disk image..."
truncate -s $DISK_SIZE $DISK_IMG

echo "Creating partition table..."
/sbin/parted -s $DISK_IMG mktable msdos
/sbin/parted -s $DISK_IMG mkpart primary ext4 1 "100%"
/sbin/parted -s $DISK_IMG set 1 boot on

echo "Setting up loop device..."
sudo losetup -Pf $DISK_IMG
LOOP_DEVICE=$(losetup -l | grep $DISK_IMG | awk '{print $1}')

echo "Formatting partition as ext4..."
sudo mkfs.ext4 -F ${LOOP_DEVICE}p1

echo "Mounting partition..."
mkdir -p $ROOTFS_DIR
sudo mount -o rw ${LOOP_DEVICE}p1 $ROOTFS_DIR

echo "Installing minimal Debian system..."
sudo debootstrap --arch amd64 $DEBIAN_RELEASE $ROOTFS_DIR http://ftp.fr.debian.org/debian/

echo "Configuring the system..."
sudo chroot $ROOTFS_DIR /bin/bash -c "
    apt update &&
    apt install -y linux-image-amd64 linux-headers-amd64 build-essential libncurses-dev libssl-dev libelf-dev bc flex bison initramfs-tools e2fsprogs sudo &&
    echo 'root:root' | chpasswd &&
    echo 'rootkit2600' > /etc/hostname &&
    useradd -m -s /bin/bash kit &&
    echo 'kit:kit' | chpasswd &&
    echo 'kit    ALL=(ALL:ALL) ALL' >> /etc/sudoers &&
    update-initramfs -c -k \6.1.0-27-amd64
"

echo "Copying rootkit project into the VM..."
sudo cp -r $ROOTKIT_DIR $ROOTFS_DIR/home/kit/rootkit
sudo chown -R kit:kit $ROOTFS_DIR/home/kit/rootkit
sudo chmod -R 700 $ROOTFS_DIR/home/kit/rootkit

sudo chroot $ROOTFS_DIR /bin/bash -c "
    chown -R kit:kit /home/kit/rootkit
    chmod -R 700 /home/kit/rootkit
"


echo "Installing GRUB and Kernel..."
sudo mkdir -p $ROOTFS_DIR/boot/grub
sudo cp $KERNEL_PATH $ROOTFS_DIR/boot/vmlinuz

cat <<EOF | sudo tee $ROOTFS_DIR/boot/grub/grub.cfg
set timeout=5
set default=0
menuentry "Debian Rootkit Linux" {
    linux /boot/vmlinuz root=/dev/sda1 console=ttyS0 rw
    initrd /boot/initrd.img-6.1.0-27-amd64
}
EOF

sudo grub-install --directory=/usr/lib/grub/i386-pc --boot-directory=$ROOTFS_DIR/boot $LOOP_DEVICE

echo "Cleaning up..."
sudo umount $ROOTFS_DIR
sudo losetup -d $LOOP_DEVICE

echo "Running QEMU..."
qemu-system-x86_64 \
    -hda $DISK_IMG \
    -nographic \
    -enable-kvm \
    -m 1024 \
    -net nic -net user