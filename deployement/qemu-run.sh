qemu-system-x86_64 \
    -hda disk.img \
    -nographic \
    -enable-kvm \
    -m 1024 \
    -net nic -net user