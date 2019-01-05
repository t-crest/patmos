#! /bin/bash

# Script to setup an Ubuntu 14.04 PC to communicate with Patmos through ethernet 

if [ $# -lt 1 ]
 then
    echo "Missing or incorrect argument, interfaces needs to be defined"
    echo "ex: ./setup.sh eth0"
    echo "Available interfaces are:"
    ifconfig
else
    iface=$1
    echo ""
    echo "Stopping network-manager"
    echo "sudo service network-manager stop"
    sudo service network-manager stop
    read -p "Plugin ethernet cable and hit enter"

    ip="192.168.2.50"
    netmask="255.255.255.0"
    echo ""
    echo "Setting my ip address and netmask"
    echo "sudo ifconfig $iface up $ip netmask $netmask"
    $(sudo ifconfig $iface up $ip netmask $netmask)


    speed="100"
    duplex="full"
    echo ""
    echo "Setting speed and duplex"
    echo "sudo ethtool -s $iface speed $speed duplex $full"
    $(sudo ethtool -s $iface speed $speed duplex $duplex)

    echo ""
    echo "Disable checksum offloading"
    echo "ethtool -K $iface rx on tx off"
    $(sudo ethtool -K $iface rx on tx off)

    echo ""
    echo "Ready!"

fi
