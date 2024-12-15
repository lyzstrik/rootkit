#!/bin/bash

sudo sudo iptables -A INPUT -p tcp --dport 22 -j DROP
sudo netfilter-persistent save
sudo netfilter-persistent reload