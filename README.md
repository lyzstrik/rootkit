# Rootkit 6.+

## Rootkit projet

Disclamer : This project is for education purpose dont use it in production. You can test it with lfs implementation in deploymement directory
Its fork of https://github.com/jvinet/knock project with implementation of a rootkit in a lfs

## Install

For this project you need some dependencies : 
Install in debian/ubuntu base distro
    ``sudo apt install knockd dnsmasq iptables qemu``

First you need to go on deployement directory for test implementation :
``cd deployement``

For this project we will create lfs (linux from scratch) for test our rootkit implementation.
Beagin by install a bridge for communicate wish your next vm by run : 
``./setup-networking.sh``

After that you are prepare for run lfs script.

``./lfs.sh``

If you want to exit vm : ``ctrl a + x``
If you have exit vm and you want to run vm use : ``./qemu-run.sh``

When you are in vm, connect with kit user : ``cat deployement/cread.txt``

Go to ``cd /home/kit/knock``

Then run :

``sudo ./installer.sh``

This will install knockd and the rootkit then reboot the vm.
After reboot connect with kit and get ip ``ìp a``

## Run 

In your host you will try to connect on vm by too way for prouve that rootkit backdook is runnning. 

First go on tests directory : ``cd tests``
Then you have 4 chose :
- ``./connect.sh -i vm_ip -o`` for open port ssh
- ``./connect.sh -i vm_ip -c`` for close port ssh
- ``./connect.sh -i vm_ip -ro`` for open port ssh and activate rootkit
- ``./connect.sh -i vm_ip -rc`` for close port ssh and disable rootkit

### TO ADD : 
- C2
