#!/bin/bash
#OMO220715 server.sh by hhs

FServer="360_HHS_frontServer"
PServer="360_HHS_backServer"

if [ "$2" == "FServer" ];then
    echo "server start FServer "
    sudo /root/bin/360_HHS_frontServer.out
elif [ "$2" == "PServer" ];then
    echo "server start PServer "
    sudo /root/bin/360_HHS_backServer.out
elif [ "$1" == "stop" ];then
    echo "server stop ... "
    FPid=$(ps -ef | grep $FServer | grep -v grep | awk '{print $2}')
    PPid=$(ps -ef | grep $PServer | grep -v grep | awk '{print $2}')
    sudo kill -9 $FPid
    sudo kill -9 $PPid
    sudo ipcrm -a

else
    echo "OMO220715_server shell:"
    echo "args:"
    echo "    1.start: start FServer" 
    echo "    2.start: start PServer"
    echo "    3.stop: stop server"
fi

