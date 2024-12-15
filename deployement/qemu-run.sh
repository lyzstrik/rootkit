sudo qemu-system-x86_64 \
    -drive file=disk.img,format=raw \
    -nographic \
    -enable-kvm \
    -m 1024 \
    -net nic -net bridge,br=br0