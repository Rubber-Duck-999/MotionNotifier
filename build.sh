#!/bin/bash

cd build

cmake ..

make

sudo modprobe bcm2835-v4l2