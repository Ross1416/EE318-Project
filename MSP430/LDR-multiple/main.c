#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include <driverlib.h>
#include "hal_LCD.h"

volatile unsigned int val1 = 0;
volatile unsigned int val2 = 0;
int current_ADC=1;

int buttonValue; 
bool selected=0;


#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch(ADCIV)
  {
    case ADCIV_ADCIFG:
      if (current_ADC == 1)
      {
        val1 = ADCMEM0;
        
        current_ADC = 0;
      }
      else if (current_ADC == 0)
      {
        val2 = ADCMEM0;
        current_ADC = 1;
      }
      __bis_SR_register_on_exit(LPM0_bits); // Low power mode select? - not low power mode
      break;
  }
}
void init_ADC(void)
{    
  SYSCFG2 |= ADCPCTL6 | ADCPCTL7; // Enable A0 & A1;
  
  ADCCTL0 = 0;
  ADCCTL0 |= ADCON | ADCSHT_2 | ADCMSC; //Turn on ADC & setting sample and hold time as 16 ADCCLK cycles?
  ADCCTL1 |= ADCSHP; // sets the source of the sampling signal (SAMPCON) to the sampling timer? - or sets it to pulse sampling mode?
  ADCCTL2 |= ADCRES; //Set 10 bit resolution 
  
  ADCMCTL0 |= ADCINCH_7; // Setting reference and input channel select to A1
  
  ADCCTL1 |= ADCCONSEQ_3;  //ADC conversion sequence mode select - 10b = Repeat-sequence-of-channels
  
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
    
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN3);

    
    ADCCTL0 |= ADCENC | ADCSC; // enable conversion and start conversion

    while(1)
    {
      ADCCTL0 |= ADCSC; // start conversion
//      printf("LDR1: %u, LDR2: %u \n", val1, val2);
      
      buttonValue = GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3);
      
      if (!buttonValue)
        selected=!selected;
        
      if (!selected) 
      {
        intToChar(valStr, val1);
        showChar('1',pos1);
//        showChar('',pos2);
      }else
      {
        showChar('2',pos1);
//        showChar('',pos2);
        intToChar(valStr, val2);
      }
      
      
      showChar(valStr[0],pos3);
      showChar(valStr[1],pos4);
      showChar(valStr[2],pos5);
      showChar(valStr[3],pos6);
      
      __delay_cycles(50000);             // Delay for 100000*(1/MCLK)=0.1s
      
    }
}
