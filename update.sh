#!/bin/sh

cd $HOME/Documents/HouseGuard-CameraMonitor

git pull

if [ -f exeCameraMonitor.py ];
then
    echo "CM File found"
    if [ -f $HOME/Documents/Temp/exeCameraMonitor.py ];
    then
        echo "CM old removed"
        rm -f $HOME/Documents/Temp/exeCameraMonitor.py
    fi
    cp exeCameraMonitor.py $HOME/Documents/Temp/exeCameraMonitor.py
fi