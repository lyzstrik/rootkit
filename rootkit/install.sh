#!/bin/bash

set -e

BRIDGE_NAME="br0"
BRIDGE_IP="192.168.100.1"
DHCP_RANGE_START="192.168.100.10"
DHCP_RANGE_END="192.168.100.20"
DHCP_LEASE_TIME="12h"

if [[ $EUID -ne 0 ]]; then
  echo "Ce script doit être exécuté en tant que root. Utilisez sudo."
  exit 1
fi

read -p "Entrez le nom de l'interface réseau avec accès Internet (ex: eth0, wlan0) : " HOST_INTERFACE

apt update
apt install -y dnsmasq iptables

if ! ip link show $BRIDGE_NAME &>/dev/null; then
  echo "Création du pont $BRIDGE_NAME..."
  ip link add $BRIDGE_NAME type bridge
  ip addr add $BRIDGE_IP/24 dev $BRIDGE_NAME
  ip link set $BRIDGE_NAME up
else
  echo "Le pont $BRIDGE_NAME existe déjà."
fi

echo "Configuration de NAT sur l'interface $HOST_INTERFACE..."
iptable_rule_exists=$(iptables -t nat -C POSTROUTING -o $HOST_INTERFACE -j MASQUERADE 2>/dev/null || echo "non")
if [[ $iptable_rule_exists == "non" ]]; then
  iptables -t nat -A POSTROUTING -o $HOST_INTERFACE -j MASQUERADE
else
  echo "La règle NAT existe déjà."
fi

iptables -A FORWARD -i $BRIDGE_NAME -o $HOST_INTERFACE -j ACCEPT
iptables -A FORWARD -i $HOST_INTERFACE -o $BRIDGE_NAME -m state --state RELATED,ESTABLISHED -j ACCEPT

DNSMASQ_CONF="/etc/dnsmasq.d/$BRIDGE_NAME.conf"
echo "Configuration de dnsmasq pour $BRIDGE_NAME..."
cat <<EOF > $DNSMASQ_CONF
interface=$BRIDGE_NAME
dhcp-range=$DHCP_RANGE_START,$DHCP_RANGE_END,$DHCP_LEASE_TIME
dhcp-option=3,$BRIDGE_IP
dhcp-option=6,8.8.8.8,8.8.4.4
EOF

systemctl restart dnsmasq

echo "Activation du transfert IP..."
echo 1 > /proc/sys/net/ipv4/ip_forward

if ! grep -q "net.ipv4.ip_forward=1" /etc/sysctl.conf; then
  echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf
  sysctl -p
fi

echo "Configuration terminée avec succès :"
echo "- Pont : $BRIDGE_NAME ($BRIDGE_IP/24)"
echo "- Interface réseau hôte : $HOST_INTERFACE"
echo "- Plage DHCP : $DHCP_RANGE_START à $DHCP_RANGE_END"
echo "Vous pouvez maintenant connecter une VM au pont $BRIDGE_NAME."
