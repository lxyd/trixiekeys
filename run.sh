#!/usr/bin/env bash
gcc -std=c99 -pedantic -Wall main.c config.c -o trixiekeys
#sudo ./trixiekeys "Microsoft Comfort Curve Keyboard 2000"
sudo ./trixiekeys /dev/input/event15
#sudo ./trixiekeys /dev/input/event0
