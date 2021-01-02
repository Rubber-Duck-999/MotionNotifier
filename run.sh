#!/bin/sh

##
echo 'Run CM'
pwd
cd /home/simon/Documents/HouseGuard-CameraMonitor
./update.sh
if [ -f "log/cameramonitor.log" ];
then
    echo "Log exists, removing"
    rm -rf logs/cameramonitor.log
fi
sleep 5
./exeCameraMonitor.py -c conf.json
