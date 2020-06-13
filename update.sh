#!/bin/sh

cd $HOME/Documents/HouseGuard-CameraMonitor

git pull

if [ -f exeCameraMonitor.py ];
then
    echo "CM File found"
    if [ -f $HOME/Documents/Deploy/exeCameraMonitor.py ];
    then
        echo "CM old removed"
        rm -f $HOME/Documents/Deploy/exeCameraMonitor.py
    fi
    cp exeCameraMonitor.py $HOME/Documents/Deploy/exeCameraMonitor.py
fi