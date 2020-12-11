// ECE 4550 Lab 4 Task 2
// Section: L04
// Authors: Caitlyn Caggia & Marco Ricci

#include "F2806x_Device.h"
#include "math.h"

Uint16 Result1 = 0x0000;
Uint16 Result2 = 0x0000;
float32 A = 0;
float32 B = 0;

Uint16 mathcounter = 0;
float32 x[100];
float32 y1[100];
float32 y2[100];
Uint16 i = 0;
Uint16 j = 0;

interrupt void ADCISR(void);
interrupt void TimerISR(void);

void setupClock(void);
void setupADC(void);
void setupInterrupts(void);

void main(void)
{
	//disable watchdog
	EALLOW;
	SysCtrlRegs.WDCR = 0x68;

	setupClock();
	setupADC();
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

	//initialize ADC based on AdcRegs

	//reinitialize interrupt registers
	PieCtrlRegs.PIEACK.all = 1;
}

interrupt void ADCISR(void)
{
	// Put here the code that runs each time the interrupt is serviced.
	// To get data in and out of subroutine YourISR, use global variables.
	// Acknowledge the interrupt before returning from subroutine YourISR.

    EALLOW;
	//add old result into x and y values
	if (mathcounter < 100)
	{
		x[mathcounter] = mathcounter;
		y1[mathcounter] = A;
		y2[mathcounter] = B;
		mathcounter++;
	}
	else
	{
	    //delete oldest value in x and y
	    for(i=0; i<99; i++)
	    {
	        y1[i] = y1[i+1];
	        y2[i] = y2[i+1];
	    }
	    //add newest result to end of x and y
	    y1[99] = A;
	    y2[99] = B;
	}

	//read new value of register 1 and clear register 1
	Result1 = AdcResult.ADCRESULT0;
	Result2 = AdcResult.ADCRESULT1;
	AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

	//normalize values to be between 0 and 3.3
	A = Result1*3.3 / 4095.0;
	B = Result2*3.3 / 4095.0;

	//reinitialize interrupt registers
	PieCtrlRegs.PIEACK.all = 1;
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
		SysCtrlRegs.PLLCR.bit.DIV = 1;

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
	CpuTimer0Regs.PRD.all = 99999; //sets value of timer frequency (PRD = 10MHz - 1)
	CpuTimer0Regs.TCR.bit.TRB = 1; //load PRD value
	CpuTimer0Regs.TCR.bit.TIE = 1; //enable timer interrupt
}

void setupADC(void)
{
	EALLOW;
    //ADC enable
	SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;

	//Set ADC clock frequency as 1/2 *fclk
	AdcRegs.ADCCTL2.bit.CLKDIV2EN = 1;
	AdcRegs.ADCCTL2.bit.CLKDIV4EN = 0;
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

	//ties channel ADC-A0 and A1 to SOC0 and 1 with CPU0 timer as the trigger
	AdcRegs.ADCSOC0CTL.bit.CHSEL = 0; //chosen by physical pin ADC-A0 on board
	AdcRegs.ADCSOC0CTL.bit.ACQPS = 6;
	AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 1; //use CPU timer 0
	AdcRegs.ADCSOC1CTL.bit.CHSEL = 1; //chosen by physical pin ADC-A1 on board
	AdcRegs.ADCSOC1CTL.bit.ACQPS = 6;
	AdcRegs.ADCSOC1CTL.bit.TRIGSEL = 1; //use CPU timer 0

	//Select and enable source 0 for EOC for Interrupt Select 1 and 2 Register
	//pulse generation occurs 1 cycle prior to ADC result latching to its result register
	AdcRegs.INTSEL1N2.bit.INT1SEL = 1;
	AdcRegs.INTSEL1N2.bit.INT1E = 1;
	AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;
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

	//register and enable CPU interrupt interfaces
	PieVectTable.ADCINT1 = &ADCISR;
	PieCtrlRegs.PIEIER1.bit.INTx1 = 1;
	PieCtrlRegs.PIEACK.all = 1;

	//enable interrupts at CPU level
	EINT;
	IER = M_INT1;
}
