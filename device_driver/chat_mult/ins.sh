#!/bin/sh

make clean
make
rmmod chat
insmod chat.ko
