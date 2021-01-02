#!/bin/sh

cd $HOME/Documents/HouseGuard-CameraMonitor

git pull

if [ -f exeCameraMonitor.py && -f Messenger.py ];
then
    echo "CM File found"
    if [ -f $HOME/Documents/Deploy/exeCameraMonitor.py && -f Messenger.py ];
    then
        echo "CM old removed"
        rm -f $HOME/Documents/Deploy/exeCameraMonitor.py
	rm -f $HOME/documents/Deploy/Messenger.py
    fi
    cp exeCameraMonitor.py $HOME/Deploy/
    cp Messenger.py $HOME/Deploy/
fi
