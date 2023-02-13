#include <msp430.h>
#include <driverlib.h>

volatile unsigned int period = 640;
volatile unsigned int duty_cycle = 320; //50% duty cycle of 50 Hz signal?

void init_timer_LED_flashing(void)
{ 
  P4DIR |= BIT0; // set as output 
  P4SEL0 |= BIT0; // set timer to output
  
  // not necessary 
  TA1CTL = TACLR; //clear timer
  TA1R=0; // set initial counter value to just zero

  // necessary
  TA1CCR0 = period;
  TA1CCR1 = duty_cycle;
  
  TA1CTL |= TASSEL_1; // ACLK clock
  TA1CTL |= ID_0; // divide by 0
  TA1CTL |= MC_1; // up mode - 01b = Up mode: Timer counts up to TAxCCR0
  TA1CCTL0 |= OUTMOD_7;
}

void init_timer_servo(void)
{ 
  P1DIR |= BIT7; // set as output 
  P1SEL0 |= BIT7; // set timer to output
  
  // not necessary 
  TA1CTL = TACLR; //clear timer
  TA1R=0; // set initial counter value to just zero

  // necessary
  TA1CCR0 = period; 
  TA1CCR1 = duty_cycle;
  
  TA1CTL |= TASSEL_1; // ACLK clock
  TA1CTL |= ID_0; // divide by 0
  TA1CTL |= MC_1; // up mode - 01b = Up mode: Timer counts up to TAxCCR0
  TA1CCTL0 |= OUTMOD_7;
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    __enable_interrupt();
    
    init_timer_servo();

    while(1)
    {
        
        __delay_cycles(2000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}
