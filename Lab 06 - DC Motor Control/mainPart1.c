// ECE 4550 Lab 6 Task 1
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

float32 uprev = 0; //average voltage
float32 yprev = 0; //measured position in rads
float32 sigmaprev = 0;
float32 xhat1prev = 0;
float32 xhat2prev = 0;
float32 u = 0; //average voltage
float32 y = 0; //measured position in rads
float32 ustar = 0;
float32 sigma = 0;
float32 xhat1 = 0;
float32 xhat2 = 0;
float32 unext = 0; //average voltage
float32 ynext = 0; //measured position in rads
float32 sigmanext = 0;
float32 xhat1next = 0;
float32 xhat2next = 0;

float32 Umax = 22.8;
float32 adjvolt = 0;

float32 alpha = 47.3270;
float32 beta = 132.7271;
float32 lambdar = 35;
float32 lambdae = 0;
float32 K11 = 0;
float32 K12 = 0;
float32 K2 = 0;
float32 L1 = 0;
float32 L2 = 0;
float32 T = 0.001;
float32 r = 0;

interrupt void TimerISR(void);

void defineParameters(void);
void setupClock(void);
void setupPWM(void);
void setupQEP(void);
void setupInterrupts(void);

void main(void)
{
    //disable watchdog
    EALLOW;
    SysCtrlRegs.WDCR = 0x68;

    defineParameters();
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
    //save old values
    yprev = y;
    uprev = u;
    sigmaprev = sigma;
    xhat1prev = xhat1;
    xhat2prev = xhat2;
    sigma = sigmanext;
    xhat1 = xhat1next;
    xhat2 = xhat2next;

    //pull value from counter for current angular position
    positioncounter = (int32)EQep1Regs.QPOSCNT;
    //int32                  Uint32

    //change integer position value to angular position
    y = positioncounter * 2.0 * 3.14 / 1000.0;
    //float32

    //perform calculations to get next values
    ustar = -K11*xhat1prev - K12*xhat2prev - K2*sigmaprev;
    if (ustar > Umax)
    {
        u = Umax;
        //sigmanext = sigma;
    }
    else if(ustar < -Umax)
    {
        u = -Umax;
        //sigmanext = sigma;
    }
    else
    {
        u = ustar;
        //sigmanext = sigma + T*(y - r);
    }

    xhat1next = xhat1 + T*xhat2 - T*L1*(xhat1 - y);
    xhat2next = xhat2 - T*alpha*xhat2 + T*beta*u - T*L2*(xhat1 - y);
    sigmanext = sigma + T*(y - r);

    //determine r (desired position)
    if (counter < 500)
    {
        r = 0;
    }
    else if (counter >= 500 && counter < 1000)
    {
        r = 2*3.14;
    }

    //compute and store next voltage value
    adjvolt = 750 + (u/24.0) * 750.0; //adjust so 750 = 0

    EPwm1Regs.CMPA.half.CMPA = adjvolt;
    EPwm2Regs.CMPA.half.CMPA = adjvolt;

    //reinitialize interrupt registers
    PieCtrlRegs.PIEACK.all = 1;

    //display data
    if (datacounter < 1975)
    {
        //add old result into u and y values
        ydat[datacounter] = y;
        udat[datacounter] = u;
        datacounter++;
    }

    counter++;
    if (counter >= 1000)
    {
        counter = 0;
    }
}


void defineParameters(void)
{
    lambdae = 4*lambdar;
    K11 = 1/beta * 3 * lambdar*lambdar;
    K12 = 1/beta * (3*lambdar - alpha);
    K2 = 1/beta * pow(lambdar,3);
    L1 = 2*lambdae - alpha;
    L2 = lambdae*lambdae - 2*alpha*lambdae + alpha*alpha;
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

    //initialize counter compare to 0 so no excitation occurs
    EPwm1Regs.CMPA.half.CMPA = 750;
    EPwm2Regs.CMPA.half.CMPA = 750;
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
