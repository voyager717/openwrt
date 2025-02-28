#!/bin/bash

echo -e "----- ${RED}DPP CERT SCRIPT usage example start -----"
echo "dev_role  --> CTT_CONF or CTT_STA or CTT_AP or DUT"
echo "dpp_role  --> conf or sta or ap"
echo "usage: sh dpp_cert_script.sh dev_role dpp_role"
echo "example: sh dpp_cert_script.sh CTT_CONF ap"
echo -e "----- ${RED}DPP CERT SCRIPT usage example end -----"

dev_role="$1"
dpp_role="$2"
echo "dev_role=$1 dpp_role=$2"

# set ip
echo "setting ip start..."
if [ "$dev_role" = "CTT_CONF" -o "$dev_role" = "CTT_STA" -o "$dev_role" = "DUT" -o "$dev_role" = "CTT_AP" ]; then
uci set dhcp.lan.ignore='1'
uci set firewall.@zone[1].input='ACCEPT'
uci set firewall.@zone[1].forward='ACCEPT'
uci set network.wan6.proto='static'
uci set network.wan.proto='static'
uci set network.lan.proto='static'
fi

if [ "$dev_role" = "CTT_CONF" ];then
	echo "---set wan to 192.168.250.65---"
	uci set network.lan.ipaddr='192.165.100.65'
	uci set network.lan.netmask='255.255.255.0'
	uci set network.wan.ipaddr='192.168.250.65'
	uci set network.wan.netmask='255.255.255.0'
elif [ "$dev_role" = "CTT_STA" ];then
	echo "---set wan to 192.168.250.66---"
        uci set network.lan.ipaddr='192.165.100.66'
        uci set network.lan.netmask='255.255.255.0'
        uci set network.wan.ipaddr='192.168.250.66'
        uci set network.wan.netmask='255.255.255.0'
elif [ "$dev_role" = "DUT" ];then
        echo "---set lan to 192.165.100.70---"
        uci set network.lan.ipaddr='192.165.100.70'
        uci set network.lan.netmask='255.255.255.0'
        uci set network.wan.ipaddr='192.168.250.70'
        uci set network.wan.netmask='255.255.255.0'
elif [ "$dev_role" = "CTT_AP" ];then
        echo "---set lan to 192.165.100.71---"
        uci set network.lan.ipaddr='192.165.100.71'
        uci set network.lan.netmask='255.255.255.0'
        uci set network.wan.ipaddr='192.168.250.71'
        uci set network.wan.netmask='255.255.255.0'
else
	echo "---wrong device role parameter---"
fi

if [ "$dev_role" = "CTT_CONF" -o "$dev_role" = "CTT_STA" -o "$dev_role" = "DUT" -o "$dev_role" = "CTT_AP" ]; then
uci commit
fi

#set dpp_cfg_default.txt
echo "setting dpp role start..."
if [ "$dpp_role" = "conf" ];then
	echo "dpp_role=configurator
	" > /etc/dpp_cfg_default.txt
elif [ "$dpp_role" = "ap" ];then
	echo "dpp_role=ap
	" > /etc/dpp_cfg_default.txt
elif [ "$dpp_role" = "sta" ];then
	echo "dpp_role=sta
	" > /etc/dpp_cfg_default.txt
else
	echo "---wrong dpp role parameter---"
fi


echo "setting done"

