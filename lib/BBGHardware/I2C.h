/*
    I2C.h
    Module to provide I2C related functionalities
*/

#ifndef I2C_H
#define I2C_H

// initializes the I2C device specified by the bus number
int I2C_init(int bus, int address);

// write specified memory to the I2C device's register
_Bool I2C_writeReg(int I2CFd, unsigned char regAddr, unsigned char memoryVal);

// read a specified register's memory content by the given address
_Bool I2C_readSingleReg(int I2CFd, unsigned char regAddr, char *memoryContent);

// read a sequence of registers' memory content from the starting register
_Bool I2C_readSequentialReg(int I2CFd, unsigned char startRegAddr, int numOfReg, char *memoryContent);

#endif