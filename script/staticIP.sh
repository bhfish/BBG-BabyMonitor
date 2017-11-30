#!/bin/bash

#####################################################################################################################
# This script configures the static IP of eth0 for the baby monitoring system                                       #
#                                                                                                                   #
#     For Baby's BBG, modify targetType variable to 1                                                               #
#     For Parent's BBG, modify targetType variable to 2                                                             #
#     then,                                                                                                         #
#     1) $scp staticIP.sh.sh root@<BBG's IP>:/root/                                                                 #
#     2) $scp babyMonitorIP.service root@<BBG's IP>:/lib/systemd/system/                                            #
#     3) #systemctl enable babyMonitorIP.service                                                                    #
#####################################################################################################################

targetType=1

if [[ $targetType -eq 1 ]]; then
    staticEth0IP='192.168.3.2'
else
    staticEth0IP='192.168.3.1'
fi

/sbin/ifconfig eth0 $staticEth0IP

exit 0




