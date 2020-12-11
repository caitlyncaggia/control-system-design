// ECE 4550 Lab 4 Task 1
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

Uint16 Result = 0x0000;

Uint16 LED1 = 0;
Uint16 LED2 = 0;
Uint16 LED3 = 0;
Uint16 LED4 = 0;
Uint16 mathcounter = 0;
Uint16 x[500];
Uint16 y[500];
Uint16 j = 0;

interrupt void ADCISR(void);
interrupt void TimerISR(void);

void main(void)
{
    //disable watchdog
    EALLOW;
    SysCtrlRegs.WDCR = 0x68;

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
        if (SysCtrlRegs.PLLSTS.bit.DIVSEL == 2 || SysCtrlRegs.PLLSTS.bit.DIVSEL == 3)
        { SysCtrlRegs.PLLSTS.bit.DIVSEL = 0; }

        SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;

        //change freq
        //OSCCLK = 10Mhz from INTOSC1
        SysCtrlRegs.PLLCR.bit.DIV = 4;

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
    CpuTimer0Regs.PRD.all = 3999; //sets value of timer frequency (PRD = 45MHz - 1)
    CpuTimer0Regs.TCR.bit.TRB = 1; //load PRD value
    CpuTimer0Regs.TCR.bit.TIE = 1; //enable timer interrupt


    //ADC enable
    SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;

    //Set ADC clock frequency as 1/4 *fclk
    AdcRegs.ADCCTL2.bit.CLKDIV2EN = 1;
    AdcRegs.ADCCTL2.bit.CLKDIV4EN = 1;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;

    //Turn power on and enable ADC
    AdcRegs.ADCCTL1.bit.ADCPWDN = 1;
    AdcRegs.ADCCTL1.bit.ADCBGPWD = 1;
    AdcRegs.ADCCTL1.bit.ADCREFPWD = 1;
    AdcRegs.ADCCTL1.bit.ADCENABLE = 1;

    //wait 1 ms
    for( j = 0; j < 40000; j++ )
    {
        ;
    }

    //ties channel ADC-A2 to SOC0 with the fastest time for sample window and CPU0 timer as the trigger
    AdcRegs.ADCSOC0CTL.bit.CHSEL = 2; //chosen by physical pin ADC-A2 on board
    AdcRegs.ADCSOC0CTL.bit.ACQPS = 6;
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 1; //use CPU timer 0

    //Select and enable source 0 for EOC for Interrupt Select 1 and 2 Register
    //pulse generation occurs 1 cycle prior to ADC result latching to its result register
    AdcRegs.INTSEL1N2.bit.INT1SEL = 0;
    AdcRegs.INTSEL1N2.bit.INT1E = 1;
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;


    //Initialize system interrupt registers
    PieCtrlRegs.PIEACK.all = 1;

    //initialize interrupt system registers
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.TINT0 = &TimerISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    //=====================================================?????????????????????????????????????????????????????
    //register and enable CPU interrupt interfaces
    PieVectTable.ADCINT1 = &ADCISR;
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;
    PieCtrlRegs.PIEACK.all = 1;

    //enable interrupts at CPU level
    EINT;
    IER = M_INT1;


    //start timer
    CpuTimer0Regs.TCR.bit.TSS = 0;

    //enable watchdog
    SysCtrlRegs.WDCR = 0x28;


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


interrupt void TimerISR(void)
{
// Put here the code that runs each time the interrupt is serviced.
// To get data in and out of subroutine YourISR, use global variables.
// Acknowledge the interrupt before returning from subroutine YourISR.

    //initialize ADC based on AdcRegs

    //reinitialize interrupt registers
    PieCtrlRegs.PIEACK.all = 1;
}

interrupt void ADCISR(void)
{
// Put here the code that runs each time the interrupt is serviced.
// To get data in and out of subroutine YourISR, use global variables.
// Acknowledge the interrupt before returning from subroutine YourISR.

    //put old result into x and y values
    if (mathcounter < 500)
    {
        x[mathcounter] = mathcounter;
        y[mathcounter] = Result;
        mathcounter++;
    }

    //read new value of register 1 and clear register 1
    Result = AdcResult.ADCRESULT0;
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    //reinitialize interrupt registers
    PieCtrlRegs.PIEACK.all = 1;
}
