// ECE 4550 Lab 8 Task 2
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

Uint32 counter = 0;
int32 positioncounter = 0;
//int32 lastpositioncounter = 1;
Uint16 datacounter = 0;
float32 ydat[1975];
float32 udat[1975];
Uint32 i = 0;

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
Uint32 settled = 0;
float32 va = 750;
float32 vA = 0;
float32 vb = 750;
float32 vB = 0;
float32 vc = 750;
float32 vC = 0;
float32 Vmax = 25.488;

float32 alpha = 0;
float32 beta = 0;
float32 lambdar = 125;
float32 lambdae = 0;
float32 K11 = 0;
float32 K12 = 0;
float32 K2 = 0;
float32 L1 = 0;
float32 L2 = 0;
float32 T = 0.0002;
float32 r = 0;
float32 N = 4;
float32 alphav = 0;
float32 phiv = 0;
float32 K = 0.04;
float32 F = 0.00005;
float32 J = 0.0000048;
float32 R = 1.2;

interrupt void TimerISR(void);

void defineParameters(void);
void setupClock(void);
void setupPWM(void);
void setupQEP(void);
void setupInterrupts(void);

void main(void)
{
    //disable watchdoggo
    EALLOW;
    SysCtrlRegs.WDCR = 0x68;

    defineParameters();
    setupClock();
    setupPWM();
    setupQEP();
    setupInterrupts();

    //start-up procedure
    //set calibration voltages

    va = (13.2/24.0) * 1500.0;
    vb = (10.8/24.0) * 1500.0;
    vc = (12.0/24.0) * 1500.0;

    EPwm1Regs.CMPA.half.CMPA = va;
    EPwm2Regs.CMPA.half.CMPA = vb;
    EPwm3Regs.CMPA.half.CMPA = vc;

    //wait 1s
    for(i = 0; i < 400000; i++)
    {
        ;
    }
    //reset position counter
    EQep1Regs.QEPCTL.bit.SWI = 1;
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
    y = positioncounter * 2.0 * 3.14 / 4000.0;
    //float32

    //perform calculations to get next values
    ustar = -K11*xhat1prev - K12*xhat2prev - K2*sigmaprev;
    if (ustar > Vmax)
    {
        u = Vmax;
    }
    else if(ustar < -Vmax)
    {
        u = -Vmax;
    }
    else
    {
        u = ustar;
    }

    xhat1next = xhat1 + T*xhat2 - T*L1*(xhat1 - y);
    xhat2next = xhat2 - T*alpha*xhat2 + T*beta*u - T*L2*(xhat1 - y);
    sigmanext = sigma + T*(y - r);

    //determine r (desired position)
    if (counter < 2500) //enter timer ISR every 1e-3sec, want requests every 0.5 sec
    {
        r = 0;
    }
    else if (counter >= 2500 && counter < 5000)
    {
        r = 2*3.14;
    }

    //determine alphav
    if (fabs(u) <= Vmax)
    {
        alphav = fabs(u);
    }
    else
    {
        alphav = Vmax;
    }

    //determine phiv
    if (u >= 0)
    {
        phiv = 3.14/2;
    }
    else
    {
        phiv = -3.14/2;
    }

    //calculate voltages
    vA = 0.5*24 + sqrt(2.0/9.0)*alphav*cos(N*y + phiv - 3.14/6.0);
    vB = 0.5*24 + sqrt(2.0/9.0)*alphav*cos(N*y + phiv - 3.14/6.0 - 2.0*3.14/3.0);
    vC = 0.5*24 + sqrt(2.0/9.0)*alphav*cos(N*y + phiv - 3.14/6.0 + 2.0*3.14/3.0);

    //compute and store next voltage value
    va = (vA/24.0) * 1500.0;
    vb = (vB/24.0) * 1500.0;
    vc = (vC/24.0) * 1500.0;

    EPwm1Regs.CMPA.half.CMPA = va;
    EPwm2Regs.CMPA.half.CMPA = vb;
    EPwm3Regs.CMPA.half.CMPA = vc;

    //reinitialize interrupt registers
    PieCtrlRegs.PIEACK.all = 1;

    //display data
    if (datacounter < 1975)
    {
        //add old result into u and y values
        ydat[datacounter] = y;
        udat[datacounter] = u;

        if (fmod(counter,10) == 0)
        {
            datacounter++;
        }
    }

    counter++;
    if (counter >= 5000)
    {
        counter = 0;
    }
}


void defineParameters(void)
{
    alpha = (K*K + F*R)/(J*R);
    beta = K/(J*R);
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
    //ftimer = fclk / ((TDDR + 1)(PRD + 1)) --> PRD = 90MHz/1000 - 1
    CpuTimer0Regs.PRD.all = 17999; //sets value of timer frequency to 1/1e-3 = 1000Hz
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
