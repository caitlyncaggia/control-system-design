// ECE 4550 Lab 7 Task 2
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

struct ECAN_REGS ECanaCopy;

float32 r = 0;
Uint32 t = 0;
float32 R1 = 0;
float32 R2 = 31.415;
float32 Ac = 3000;
float32 Sc = 125;
float32 ta = 0;
float32 ts = 0;
float32 t1 = 0;
float32 t2 = 0;
float32 r1 = 0;
float32 r2 = 0;
float32 ac = 0;
float32 sc = 0;
float32 tt = 0;
float32 arg = 0;

Uint32 counter = 0;
Uint32 Rtx = 0;
Uint32 m = 1000;
Uint32 b = 0;
float32 rdat[1975];
Uint16 datacounter = 0;

Uint16 PB1 = 1;
Uint16 PB2 = 1;
Uint16 HEX0 = 0;
Uint16 HEX1 = 0;
Uint16 HEX2 = 0;
Uint16 HEX3 = 0;
Uint16 hex = 0;
Uint16 flag = 0;

interrupt void TimerISR(void);

void setupClock(void);
void setupCAN(void);
void setupInterrupts(void);

void main(void)
{
    //disable watchdog
    EALLOW;
    SysCtrlRegs.WDCR = 0x68;

    ta = Sc/Ac;
    ts = (R2 - R1)/Sc - ta;
    flag = 1;

    setupClock();
    setupCAN();
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
    t = counter*0.001;

    //calculate r
    if ( counter % 1000 < 500 )
    {
        r1 = R1;
        r2 = R2;
        ac = Ac;
        sc = Sc;
        t1 = 0;
    }
    else
    {
        r1 = R2;
        r2 = R1;
        ac = -Ac;
        sc = -Sc;
        t1 = 0.5;
    }

    t2 = t1+ta+ts+ta;
    tt = counter*0.001;
    if (tt >= t1 && tt < t1+ta)
    {
        r = r1+0.5*ac*(tt-t1)*(tt-t1);
    }
    else if (tt >= t1+ta && tt < t2-ta)
    {
        r = 0.5*(r1+r2)+sc*(tt-0.5*(t1+t2));
    }
    else if (tt >= t2-ta && tt < t2)
    {
        r = r2-0.5*ac*(t2-tt)*(t2-tt);
    }
    else
    {
        r = r2;
    }

    Rtx = (int)(m * r + b);

    //from Task 1
    PB1 = GpioDataRegs.GPADAT.bit.GPIO17;
    PB2 = GpioDataRegs.GPBDAT.bit.GPIO40;
    HEX0 = GpioDataRegs.GPADAT.bit.GPIO12; //least significant
    HEX1 = GpioDataRegs.GPADAT.bit.GPIO13;
    HEX2 = GpioDataRegs.GPADAT.bit.GPIO14;
    HEX3 = GpioDataRegs.GPADAT.bit.GPIO15; //most significant
    hex = 8*HEX3 + 4*HEX2 + 2*HEX1 + HEX0;

    //store values to transmit
    //lower 32 bits contain pushbutton and hex info
    ECanaMboxes.MBOX0.MDL.byte.BYTE0 = PB1;
    ECanaMboxes.MBOX0.MDL.byte.BYTE1 = PB2;
    ECanaMboxes.MBOX0.MDL.byte.BYTE2 = hex;
    ECanaMboxes.MBOX0.MDL.byte.BYTE3 = flag;
    //upper 32 bits contain position values
    ECanaMboxes.MBOX0.MDH.all = Rtx;
    ECanaCopy.CANTRS.all = 0;
    ECanaCopy.CANTRS.bit.TRS0 = 1; //transmit request
    ECanaRegs.CANTRS.all = ECanaCopy.CANTRS.all;

    //display data
    if (datacounter < 1975)
    {
        //add old result into u and y values
        rdat[datacounter] = r;
        datacounter++;
    }

    //reinitialize interrupt registers
    PieCtrlRegs.PIEACK.all = 1;

    counter++;
    if (counter >= 1000)
    { counter = 0; }
}

void setupCAN(void)
{
    GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 01; //set pin mux for CANRXA(I) – pg 125
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 01; //set pin mux for CANTXA(O)
    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1; //clock ecan-a at sysclkout/2

    //enable pin functions (transmit and receive)
    ECanaCopy.CANTIOC.all = ECanaRegs.CANTIOC.all;
    ECanaCopy.CANTIOC.bit.TXFUNC = 1; //enable transmit – pg 1079
    ECanaRegs.CANTIOC.all = ECanaCopy.CANTIOC.all;
    ECanaCopy.CANRIOC.all = ECanaRegs.CANRIOC.all;
    ECanaCopy.CANRIOC.bit.RXFUNC = 1; //enable receive
    ECanaRegs.CANRIOC.all = ECanaCopy.CANRIOC.all;

    //set timing parameters – pg 1067; use TQ = 15
    ECanaCopy.CANBTC.all = ECanaRegs.CANBTC.all;
    ECanaCopy.CANBTC.bit.TSEG1REG = 8; //15 = 1 +TS1 + TS2
    ECanaCopy.CANBTC.bit.TSEG2REG = 6; // TS2 < TS1
    ECanaCopy.CANBTC.bit.BRPREG = 5;  //(45e6/500e3)/15 - 1= x = 5
    ECanaRegs.CANBTC.all = ECanaCopy.CANBTC.all;

    //activate normal operation
    ECanaCopy.CANMC.all = ECanaRegs.CANMC.all;
    ECanaCopy.CANMC.bit.CCR = 0; //EALLOW protected – pg 1063
    ECanaRegs.CANMC.all = ECanaCopy.CANMC.all;
    do ECanaCopy.CANES.all = ECanaRegs.CANES.all;
    while (ECanaCopy.CANES.bit.CCE != 1); // 0 = denied access - pg 1069

    //configure mailboxes
    ECanaMboxes.MBOX0.MSGCTRL.all = 0; //pg 1089
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8; //how many bytes are tx/rx
    ECanaMboxes.MBOX0.MSGID.all = 100; //message ID
    ECanaCopy.CANMD.all = ECanaRegs.CANMD.all;
    ECanaCopy.CANMD.bit.MD0 = 0; // mailbox direction - pg 1053 0 = tx, 1 = rx
    ECanaRegs.CANMD.all = ECanaCopy.CANMD.all;
    ECanaCopy.CANME.all = ECanaRegs.CANME.all;
    ECanaCopy.CANME.bit.ME0 = 1; //enable mailbox - pg 1052
    ECanaRegs.CANME.all = ECanaCopy.CANME.all;
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
