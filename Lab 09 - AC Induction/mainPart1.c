// ECE 4550 Lab 9 Task 1
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

int32 counter = 0;
int32 positioncounter;
Uint16 datacounter = 0;
float32 ydat[1975];
float32 udat[1975];
int32 i = 0;

Uint32 state = 1;
float32 va = 750;
float32 vA = 0;
float32 vb = 750;
float32 vB = 0;
float32 vc = 750;
float32 vC = 0;

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

    if (state == 1)
    {
        vA = 16;
        vB = 8;
        vC = 12;
    }
    else if (state == 2)
    {
        vA = 12;
        vB = 16;
        vC = 8;
    }
    else if (state == 3)
    {
        vA = 8;
        vB = 12;
        vC = 16;
    }

    //compute and store next voltage value
    va = ((vA)/24.0) * 1500.0;
    vb = ((vB)/24.0) * 1500.0;
    vc = ((vC)/24.0) * 1500.0;

    //va = 1125; vb = 750; vc = 375;
    EPwm1Regs.CMPA.half.CMPA = va;
    EPwm2Regs.CMPA.half.CMPA = vb;
    EPwm3Regs.CMPA.half.CMPA = vc;

    //reinitialize interrupt registers
    PieCtrlRegs.PIEACK.all = 1;

    //display data
    if (datacounter < 1975 && counter >= 1000)
    {
        //add old result into u and y values
        //ydat[datacounter] = y;
        //udat[datacounter] = u;
        datacounter++;
    }

    counter++;
    if (counter >= 2000)
    {
        counter = 0;
    }
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
        //OSCCLK = 10Mhz from INTOSC1 = fsys
        SysCtrlRegs.PLLCR.bit.DIV = 0b01001; //set fclk = 90 MHz

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
    //ftimer = fclk / ((TDDR + 1)(PRD + 1)) --> PRD = 90MHz/5000 - 1
    CpuTimer0Regs.PRD.all = 17999; //sets value of timer frequency ftimer = (1/200E-6) = 5000 Hz
    CpuTimer0Regs.TCR.bit.TRB = 1; //load PRD value
    CpuTimer0Regs.TCR.bit.TIE = 1; //enable timer interrupt
}

void setupPWM(void)
{
    EALLOW;
    //access, select and enable EPWM1A, EPWM2A, and EPWM3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0b01;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0b01;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0b01;
    SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;
    SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;
    SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;

    //set up EPWM1B, 2B, 3B
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0; //enable output
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1; //select direction
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;
    GpioDataRegs.GPASET.bit.GPIO1 = 1; //set value
    GpioDataRegs.GPASET.bit.GPIO3 = 1;
    GpioDataRegs.GPASET.bit.GPIO5 = 1;

    //set mode to up-down-count mode
    // 00 = up count mode
    // 01 = down count mode
    EPwm1Regs.TBCTL.bit.CTRMODE = 0b10;
    EPwm2Regs.TBCTL.bit.CTRMODE = 0b10;
    EPwm3Regs.TBCTL.bit.CTRMODE = 0b10;

    //set counter increment for 32mV = 2*24V/TBPRD
    EPwm1Regs.TBPRD = 1500;
    EPwm2Regs.TBPRD = 1500;
    EPwm3Regs.TBPRD = 1500;

    //set PWM frequency - page 335 of ref manual
    //fpwm = fclk / [2(tbprd)(hspclkdiv)(clkdiv)] = 30kHz
    //fclk = 90 MHz, TBPRD = 1500 --> hspclkdiv = clkdiv = 1
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm3Regs.TBCTL.bit.CLKDIV = 0;

    //set output for EPWM1
    EPwm1Regs.AQCTLA.bit.CAU = 1; //falling edge
    EPwm1Regs.AQCTLA.bit.CAD = 2; //rising edge

    //set output for EPWM2
    EPwm2Regs.AQCTLA.bit.CAU = 1; //falling edge
    EPwm2Regs.AQCTLA.bit.CAD = 2; //rising edge

    //set output for EPWM3
    EPwm3Regs.AQCTLA.bit.CAU = 1; //falling edge
    EPwm3Regs.AQCTLA.bit.CAD = 2; //rising edge

    //enable time based clock
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;

    //initialize counter compare to 0 so no excitation occurs
    EPwm1Regs.CMPA.half.CMPA = 750;
    EPwm2Regs.CMPA.half.CMPA = 750;
    EPwm3Regs.CMPA.half.CMPA = 750;
}

void setupQEP(void)
{
    //select EQEP1A
    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0b01;
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0b01;

    //enable clock for EQEP1A
    SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;

    //set max counting value (given in instructions)
    EQep1Regs.QPOSMAX = 0xFFFFFFFF;

    //set initial counting value to 0
    EQep1Regs.QPOSINIT = 0;

    //enable and initialize counter
    EQep1Regs.QEPCTL.bit.QPEN = 1;
    EQep1Regs.QEPCTL.bit.SWI = 1;
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
