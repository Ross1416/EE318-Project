#include <msp430.h>
#include <driverlib.h>
#include "hal_LCD.h"

volatile unsigned int val = 0;

#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch(ADCIV)
  {
    case ADCIV_ADCIFG:
      val = ADCMEM0;
      break;
  }
}

void init_ADC(void)
{    
  SYSCFG2 |= ADCPCTL0 | ADCPCTL1; // Enable A6;
  
  ADCCTL0 |= ADCON | ADCSHT_2; //Turn on ADC & setting sample and hold time as 16 ADCCLK cycles?
  ADCCTL1 |= ADCSHP; // sets the source of the sampling signal (SAMPCON) to the sampling timer? - or sets it to pulse sampling mode?
  ADCCTL2 |= ADCRES; //Set 10 bit resolution 
  
  ADCMCTL0 |= ADCINCH_1; // Setting reference and input channel select
  
  ADCCTL1 |= ADCCONSEQ_3;  //doesnt really seem to change anything - ADC conversion sequence mode select - 10b = Repeat-single-channel
  
  ADCIFG &= ~0x01; // clear interrupt - why this bit?
  ADCIE |= ADCIE0; // Enable interrupt;
  
}

void intToChar(char out[4], int value)
{
    out[0] = (value / 1000);
    out[1] = ((value - out[0] * 1000) / 100);
    out[2] = ((value - out[0] * 1000 - out[1] * 100) / 10);
    out[3] = (value - out[0] * 1000 - out[1] * 100 - out[2] * 10);

    out[0] = out[0] + '0';
    out[1] = out[1] + '0';
    out[2] = out[2] + '0';
    out[3] = out[3] + '0';
//    out[4] = '\0';
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
    
    char valStr[4];
    
    Init_LCD();
    init_ADC();
    __enable_interrupt();
    
    ADCCTL0 |= ADCENC | ADCSC; // enable conversion and start conversion

    while(1)
    {
      ADCCTL0 |= ADCSC; // start conversion
      
      intToChar(valStr, val);
      
      showChar(valStr[0],pos1);
      showChar(valStr[1],pos2);
      showChar(valStr[2],pos3);
      showChar(valStr[3],pos4);
      
        __delay_cycles(2000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}
