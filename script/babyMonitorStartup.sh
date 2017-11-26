#!/bin/bash

#####################################################################################################################
# This script automates the startup procedures of the baby monitoring program in *BOTH* baby and parent's BBG.      #
# In order to let this script to be executed upon each board reset, user has to do the following at lease once:     #
#                                                                                                                   #
#     For Baby's BBG, modify runType variable to 1                                                                  #
#     For Parent's BBG, modify runType variable to 2                                                                #
#     then,                                                                                                         #
#     1) $scp babyMonitorStartup.sh root@<BBG's IP>:/root/                                                          #
#     2) $scp babyMonitor.service root@<BBG's IP>:/lib/systemd/system/                                              #
#     3) #systemctl enable babyMonitor.service                                                                      #
#####################################################################################################################

runType=1
monitoringExePath='/root'
nodejsServerExePath='/root/web/server.js'

if [[ $runType -eq 1 ]]; then
    monitoringExeName='babyMonitor'
    staticEth0IP='192.168.3.2'
else
    monitoringExeName='parentMonitor'
    staticEth0IP='192.168.3.1'
fi

sleep 10

/sbin/ifconfig eth0 $staticEth0IP

sleep 1

if [[ $deploymentType -eq 1 ]]; then
    ${monitoringExePath}/${monitoringExeName} &

    sleep 1

    /usr/bin/nodejs --harmony $nodejsServerExePath &
else
    ${monitoringExePath}/${monitoringExeName} &
fi




