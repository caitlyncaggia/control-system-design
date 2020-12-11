
// ECE 4550 Lab 2 Task 1 - RAM
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

Uint32 i = 0;
Uint32 j = 2;

Uint16 LED1 = 0;
Uint16 LED2 = 0;
Uint16 LED3 = 0;
Uint16 LED4 = 0;

Uint16 HEX0 = 0;
Uint16 HEX1 = 0;
Uint16 HEX2 = 0;
Uint16 HEX3 = 0;

Uint16 one = 1;
Uint16 zero = 0;


void main(void)
{
    switch (j)
    {
        case 0 :
            break;
        case 1 :
            EALLOW;
            SysCtrlRegs.WDCR = 0b0000000001101000;
            EDIS;
        case 2 :
            EALLOW;
            SysCtrlRegs.WDCR = 0b0000000000101000;
            EDIS;
    }

    GpioDataRegs.GPACLEAR.bit.GPIO9;
    GpioDataRegs.GPACLEAR.bit.GPIO11;
    GpioDataRegs.GPBCLEAR.bit.GPIO34;
    GpioDataRegs.GPBCLEAR.bit.GPIO41;

    //enable mux as default

    //assign signal directions
    //inputs (HEX): 12, 13, 14, 15
    //outputs (LED): 9, 11, 34, 41 --> 1
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO41 = 1;
    EDIS;

    //pull up resistors
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO9 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO41 = 0;
    EDIS;

    while(1)
    {
        //reset watchdog
        if (j == 2)
        {
            EALLOW;
            SysCtrlRegs.WDKEY = 0x55;
            SysCtrlRegs.WDKEY = 0xAA;
            EDIS;
        }

        //read pins
        LED1 = GpioDataRegs.GPADAT.bit.GPIO9;
        LED2 = GpioDataRegs.GPADAT.bit.GPIO11;
        LED3 = GpioDataRegs.GPBDAT.bit.GPIO34;
        LED4 = GpioDataRegs.GPBDAT.bit.GPIO41;

        HEX0 = GpioDataRegs.GPADAT.bit.GPIO12;
        HEX1 = GpioDataRegs.GPADAT.bit.GPIO13;
        HEX2 = GpioDataRegs.GPADAT.bit.GPIO14;
        HEX3 = GpioDataRegs.GPADAT.bit.GPIO15;

        if (HEX3 == one)
        {
            GpioDataRegs.GPASET.bit.GPIO9 = 1;
        }
        else
        {
            GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
        }

        if (HEX2 == one)
        {
            GpioDataRegs.GPASET.bit.GPIO11 = 1;
        }
        else
        {
            GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;
        }

        if (HEX1 == one)
        {
            GpioDataRegs.GPBSET.bit.GPIO34 = 1;
        }
        else
        {
            GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
        }

        if (HEX0 == one)
        {
            GpioDataRegs.GPBSET.bit.GPIO41 = 1;
        }
        else
        {
            GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1;
        }


    }

}
