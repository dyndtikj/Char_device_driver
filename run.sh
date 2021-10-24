#!/bin/bash

make clean
make

if [ -f mychrdev.ko ]
then
        # if the kernel module is already installed  
        lsmod | grep mychrdev >& /dev/null
        if [ $? -eq 0 ]
        then 
                sudo rmmod mychrdev
        fi
        sudo insmod mychrdev.ko

        gcc -o test.o test.c
        ./test.o
fi
