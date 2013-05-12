#!/usr/bin/env bash
gcc -std=c99 -pedantic -Wall src/trixiekeys.c src/config.c -o trixiekeys
#sudo ./trixiekeys "Microsoft Comfort Curve Keyboard 2000"
#sudo ./trixiekeys "USB Keyboard"
sudo ./trixiekeys "Apple Inc. Apple Internal Keyboard / Trackpad"
#sudo ./trixiekeys /dev/input/event0
