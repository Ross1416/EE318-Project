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
  
  // Using other way
  GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P8, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);
  
  ADC_init(ADC_BASE, 
           ADC_SAMPLEHOLDSOURCE_SC,
           ADC_CLOCKSOURCE_ADCOSC,
           //ADC_CLOCKDIVIDER_1);
           ADC_CLOCKDIVIDER_512);
  

  ADC_enable(ADC_BASE);
  
  ADC_configureMemory(ADC_BASE,
                      ADC_INPUT_A8,
                      ADC_VREFPOS_AVCC,
                      ADC_VREFNEG_AVSS);
  
  ADC_setupSamplingTimer(ADC_BASE,
                         ADC_CYCLEHOLD_4_CYCLES,
                         //ADC_MULTIPLESAMPLESDISABLE);
                         ADC_MULTIPLESAMPLESENABLE);
  
  ADC_clearInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);
  ADC_enableInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);
  
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
    ADC_startConversion(ADC_BASE,ADC_REPEATED_SINGLECHANNEL); //ADC_SINGLECHANNEL);

    while(1)
    {
      intToChar(valStr, val);
      
      showChar(valStr[0],pos1);
      showChar(valStr[1],pos2);
      showChar(valStr[2],pos3);
      showChar(valStr[3],pos4);
      
        __delay_cycles(2000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}
