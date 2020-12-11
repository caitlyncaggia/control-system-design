// ECE 4550 Lab 5 Task 1
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

int32 counter = 0;
float32 voltagehigh = 20;
float32 voltagelow = -20;
Uint16 zero = 750;
Uint16 high = 1375;
Uint16 low = 125;
Uint16 k = 0;

interrupt void TimerISR(void);

void setupClock(void);
void setupPWM(void);
void setupQEP(void);
void setupInterrupts(void);

void main(void)
{
    //disable watchdog
    EALLOW;
    SysCtrlRegs.WDCR = 0x68;

    setupClock();
    setupPWM();
    setupQEP();
    setupInterrupts();

    //start timer
    CpuTimer0Regs.TCR.bit.TSS = 0;

    //enable watchdog
    SysCtrlRegs.WDCR = 0x28;

    //service watchdog
    while(1)
    {
        //reset watchdog
        SysCtrlRegs.WDKEY = 0x55;
        SysCtrlRegs.WDKEY = 0xAA;
    }
}


interrupt void TimerISR(void)
{
    // Put here the code that runs each time the interrupt is serviced.
    // To get data in and out of subroutine YourISR, use global variables.
    // Acknowledge the interrupt before returning from subroutine YourISR.

    //map and write duty cycle
    if (counter < 200) // 200 ms in 0.2 seconds
    {
        //Vavg = +20 V
        EPwm1Regs.CMPA.half.CMPA = high;
        EPwm2Regs.CMPA.half.CMPA = high;
    }
    else if (counter >= 200 && counter < 400)
    {
        //Vavg = 0 V
        EPwm1Regs.CMPA.half.CMPA = zero;
        EPwm2Regs.CMPA.half.CMPA = zero;
    }
    else if(counter >= 400 && counter < 600)
    {
        //Vavg = -20 V
        EPwm1Regs.CMPA.half.CMPA = low;
        EPwm2Regs.CMPA.half.CMPA = low;
    }
    else if(counter >= 600 && counter < 800)
    {
        //Vavg = 0 V
        EPwm1Regs.CMPA.half.CMPA = zero;
        EPwm2Regs.CMPA.half.CMPA = zero;
    }
    else
    {
        counter = 0;
    }

    //pulls value from counter for current angular position
    // ? = EQep1Regs.QPOSCNT;
    //int32         Uint32

    //compute and store next voltage value
    //v = (va - vb) / 2
    //d(k) = 0.5 * ( 1 + v(k)/vdc );
    //vdc = 24 V,
    //d1 = .9166666666 at +20V
    //d2 = 0.5 at 0 V
    //d3 = 0.083333333 at -20V

    //reinitialize interrupt registers
    PieCtrlRegs.PIEACK.all = 1;

    counter++;
}


void setupClock(void)
{
    EALLOW;
    //initialize system clock
    if (SysCtrlRegs.PLLSTS.bit.MCLKSTS != 1)
    {
        if (SysCtrlRegs.PLLSTS.bit.DIVSEL == 2 || SysCtrlRegs.PLLSTS.bit.DIVSEL == 3)
        { SysCtrlRegs.PLLSTS.bit.DIVSEL = 0; }

        SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;

        //change freq
        //OSCCLK = 10Mhz from INTOSC1
        SysCtrlRegs.PLLCR.bit.DIV = 0b01001;

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
    CpuTimer0Regs.PRD.all = 89999; //sets value of timer frequency (PRD = 90kHz - 1)
    CpuTimer0Regs.TCR.bit.TRB = 1; //load PRD value
    CpuTimer0Regs.TCR.bit.TIE = 1; //enable timer interrupt
}

void setupPWM(void)
{
    EALLOW;
    //access, select and enable EPWM1A and EPWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0b01;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0b01;
    SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;
    SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;

    //enable output
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;
    GpioDataRegs.GPASET.bit.GPIO1 = 1;
    GpioDataRegs.GPASET.bit.GPIO3 = 1;

    //set mode to up-down-count mode
       // 00 = up count mode
       // 01 = down count mode
    EPwm1Regs.TBCTL.bit.CTRMODE = 0b10;
    EPwm2Regs.TBCTL.bit.CTRMODE = 0b10;

    //set counter increment for 32mV = 2*24V/TBPRD
    EPwm1Regs.TBPRD = 1500;
    EPwm2Regs.TBPRD = 1500;

    //set PWM frequency - page 335 of ref manual
    //fpwm = fclk / [2(tbprd)(hspclkdiv)(clkdiv)] = 30kHz
    //fclk = 90 MHz, TBPRD = 1500 --> hspclkdiv = clkdiv = 1
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;

    //set output for EPWM1
    EPwm1Regs.AQCTLA.bit.CAU = 1; //falling edge
    EPwm1Regs.AQCTLA.bit.CAD = 2; //rising edge

    //set output for EPWM2
    EPwm2Regs.AQCTLA.bit.CAU = 2; //rising edge
    EPwm2Regs.AQCTLA.bit.CAD = 1; //falling edge

    //enable time based clock
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
}

void setupQEP(void)
{
    //select EQEP1A
    GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 0b01;

    //enable clock for EQEP1A
    SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;

    //set max counting value (given in instructions)
    EQep1Regs.QPOSMAX = 0xFFFFFFFF;

    //set initial counting value to 0
    EQep1Regs.QPOSINIT = 0;

    //enable and initialize counter
    EQep1Regs.QEPCTL.bit.QPEN = 1;
    EQep1Regs.QEPCTL.bit.SWI = 1;

    //initialize counter compare to 0 so no excitation occurs
    EPwm1Regs.CMPA.half.CMPA = 0;
    EPwm1Regs.CMPB = 0;
}

void setupInterrupts(void)
{
    EALLOW;
    //Initialize system interrupt registers
    PieCtrlRegs.PIEACK.all = 1;

    //initialize interrupt system registers
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.TINT0 = &TimerISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    //enable interrupts at CPU level
    EINT;
    IER = M_INT1;
}
