// ECE 4550 Lab 3 Task 1
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

Uint16 LED1 = 0;
Uint16 LED2 = 0;
Uint16 LED3 = 0;
Uint16 LED4 = 0;

interrupt void YourISR(void);

void main(void)
{
    //disable watchdog
    EALLOW;
    SysCtrlRegs.WDCR = 0b0000000001101000;

    GpioDataRegs.GPACLEAR.bit.GPIO9;
    GpioDataRegs.GPACLEAR.bit.GPIO11;
    GpioDataRegs.GPBCLEAR.bit.GPIO34;
    GpioDataRegs.GPBCLEAR.bit.GPIO41;

    //enable mux as default

    //assign signal directions
    //inputs (HEX): 12, 13, 14, 15
    //outputs (LED): 9, 11, 34, 41 --> 1
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO41 = 1;

    //pull up resistors
    GpioCtrlRegs.GPAPUD.bit.GPIO9 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO41 = 0;

    //initialize system clock
    if (SysCtrlRegs.PLLSTS.bit.MCLKSTS != 1)
    {
        if (SysCtrlRegs.PLLSTS.bit.DIVSEL ==2 || SysCtrlRegs.PLLSTS.bit.DIVSEL == 3)
        { SysCtrlRegs.PLLSTS.bit.DIVSEL = 0; }

        SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;

        //change freq
        //OSCCLK = 10Mhz from INTOSC1
        SysCtrlRegs.PLLCR.bit.DIV = 0b00001;

        while(SysCtrlRegs.PLLSTS.bit.PLLLOCKS != 1)
        {
                ;
        }

        SysCtrlRegs.PLLSTS.bit.MCLKOFF = 0;
        SysCtrlRegs.PLLSTS.bit.DIVSEL = 3;
    }

    //set timer frequency
    CpuTimer0Regs.TPR.bit.TDDR = 0;
    CpuTimer0Regs.TPRH.bit.TDDRH = 0;
    CpuTimer0Regs.TCR.bit.TSS = 1; //stop timer to set up interrupt
    CpuTimer0Regs.PRD.all = 4999999; //sets value of timer frequency (PRD = 5MHz - 1)
    CpuTimer0Regs.TCR.bit.TRB = 1; //load PRD value
    CpuTimer0Regs.TCR.bit.TIE = 1; //enable timer interrupts

    //initialize interrupt system registers
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.TINT0 = &YourISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEACK.all = 1;

    //enable interrupts at CPU level
    EINT;
    IER = M_INT1;

    //enable watchdog
    SysCtrlRegs.WDCR = 0b0000000000101000;

    //start timer
    CpuTimer0Regs.TCR.bit.TSS = 0;

    EDIS;
    //service watchdog
    while(1)
    {
        //reset watchdog
        EALLOW;
        SysCtrlRegs.WDKEY = 0x55;
        SysCtrlRegs.WDKEY = 0xAA;
        EDIS;
    }

}

interrupt void YourISR(void)
{
// Put here the code that runs each time the interrupt is serviced.
// To get data in and out of subroutine YourISR, use global variables.
// Acknowledge the interrupt before returning from subroutine YourISR.

    //read pins
    //LED1 = GpioDataRegs.GPADAT.bit.GPIO9;
    //LED2 = GpioDataRegs.GPADAT.bit.GPIO11;
    //LED3 = GpioDataRegs.GPBDAT.bit.GPIO34;
    //LED4 = GpioDataRegs.GPBDAT.bit.GPIO41;

    GpioDataRegs.GPATOGGLE.bit.GPIO9 = 1;
    PieCtrlRegs.PIEACK.all = 1;
}
