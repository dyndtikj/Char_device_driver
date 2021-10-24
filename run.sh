#!/bin/bash

make clean
make

# if compilation has produced a loadable kernel module
if [ -f mychrdev.ko ]
then
        # if the kernel module is already installed  
        lsmod | grep mychrdev >& /dev/null
        if [ $? -eq 0 ]
        then
                # uninstall the existing kernel module  
        sudo rmmod mychrdev
        fi

        # install the new version of the kernel module
        sudo insmod mychrdev.ko

        # # remove user's executable if it exists
        # [ -x main ] && rm user_src/main

        # compile and execute the user code which uses the module
        gcc -o test.o test.c
        ./test.o
fi
