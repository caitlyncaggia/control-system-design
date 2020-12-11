// ECE 4550 Lab 1
// Section: L04
// Author: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"
#define pi 3.14159

Uint32 i = 0;
Uint32 j = 1;
Uint32 counter = 0;
float32 mathcounter = 0;
float32 x[300];
float32 y[300];

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

    while(1)
    {

        if (j == 2)
        {
            EALLOW;
            SysCtrlRegs.WDKEY = 0x55;
            SysCtrlRegs.WDKEY = 0xAA;
            EDIS;
        }

        if (mathcounter < 300)
        {
            x[counter] = (3*mathcounter)/300;
            y[counter] = 3*cos(2*pi*x[counter]) - cos(6*pi*x[counter]);
            mathcounter++;
        }

        counter++;

        if (counter == 10000)
        {
            i++;
            counter = 0;
        }

    }

}
