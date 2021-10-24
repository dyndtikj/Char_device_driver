# Char_device_driver
Linux kernel module for symbolic devices (count can be defined in mychardev.c)

To make, insmod and test module write ./run.sh command, you will see an inteface for testing.

### Write command writes from current offset, writting to the opened file will add data, not rewritting, to rewrite close file

## Write to file

![Im1](./images/2.png)

## Read specific number of bytes
![Im2](./images/3.png)
 
## Read all that file contains till the end
![Im3](./images/4.png)
