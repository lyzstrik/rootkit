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
                     \_____/_|   \____/                     
                                                                                                                     
"

if ! command -v losetup &> /dev/null; then
    echo "losetup not found, adding /usr/sbin to PATH"
    export PATH=$PATH:/usr/sbin
    if ! command -v losetup &> /dev/null; then
        echo "Error: losetup is still not found after modifying PATH."
        exit 1
    fi
fi

losetup -a | grep disk.img | cut -d ':' -f1 | while read loop; do
    sudo rm $loop
done

KERNEL_PATH="vmlinuz-6.1.0-27-amd64"
ROOTKIT_DIR=$(realpath "$(pwd)/../../knock")
DISK_IMG="disk.img"
DISK_SIZE="3G"
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

echo "iptables-persistent iptables-persistent/autosave_v4 boolean true" | sudo chroot $ROOTFS_DIR /usr/bin/debconf-set-selections
echo "iptables-persistent iptables-persistent/autosave_v6 boolean true" | sudo chroot $ROOTFS_DIR /usr/bin/debconf-set-selections

echo "Configuring the system..."
sudo chroot $ROOTFS_DIR /bin/bash -c "
    apt update &&
    apt install -y linux-image-amd64 linux-headers-amd64 build-essential libncurses-dev libssl-dev libelf-dev bc flex bison initramfs-tools e2fsprogs sudo net-tools ifupdown git ssh openssh-server wget libpcap-dev autoconf iptables iptables-persistent &&
    echo 'root:root' | chpasswd &&
    echo 'rootkit2600' > /etc/hostname &&
    echo '127.0.0.1       rootkit2600' >> /etc/hosts &&
    useradd -m -s /bin/bash kit &&
    echo 'kit:kit' | chpasswd &&
    echo 'kit    ALL=(ALL:ALL) ALL' >> /etc/sudoers &&
    update-initramfs -c -k \6.1.0-27-amd64
"

echo "Configuring network..."
cat <<EOF | sudo tee $ROOTFS_DIR/etc/network/interfaces
auto lo
iface lo inet loopback

auto ens3
iface ens3 inet dhcp
EOF

sudo chroot $ROOTFS_DIR /bin/bash -c "
    echo 'nameserver 8.8.8.8' > /etc/resolv.conf &&
    systemctl enable networking
"

echo "Copying rootkit project into the VM..."
sudo rsync -av --exclude='deployement' --exclude='.vscode' --exclude='.git' $ROOTKIT_DIR/ $ROOTFS_DIR/home/kit/knock/
sudo chown -R kit:kit $ROOTFS_DIR/home/kit/knock
sudo chmod -R 700 $ROOTFS_DIR/home/kit/knock

sudo chroot $ROOTFS_DIR /bin/bash -c "
    chown -R kit:kit /home/kit/knock
    chmod -R 700 /home/kit/knock
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
sudo qemu-system-x86_64 \
    -drive file=$DISK_IMG,format=raw \
    -nographic \
    -enable-kvm \
    -m 1024 \
    -net nic -net bridge,br=br0
