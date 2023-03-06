#include <msp430.h>
//#include <driverlib.h>

volatile unsigned int period = 640;
volatile unsigned int duty_cycle = 74; //50% duty cycle of 50 Hz signal?

volatile unsigned int period_led = 32000;
volatile unsigned int duty_cycle_led = 16000; //50% duty cycle of 50 Hz signal?


//https://www.arduino.cc/reference/en/language/functions/math/map/
volatile long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

unsigned int current_angle = 45;
unsigned int angle_step_size = 5;

int servo_lower_limit_count=18;
int servo_upper_limit_count=74;
// For Testing
int LDR1=560;
int LDR2=450;

int LDRdiff=0;


volatile unsigned int pulse_width=18;

void init_timer_LED_flashing(void)
{ 
  P4DIR |= BIT0; // set as output 
  P4SEL0 |= BIT0; // set timer to output
  
  // not necessary 
  TA1CTL = TACLR; //clear timer
  TA1R=0; // set initial counter value to just zero

  // necessary
  TA1CCR0 = period_led;
  TA1CCR1 = duty_cycle_led;
  
  TA1CTL |= TASSEL_1; // ACLK clock
  TA1CTL |= ID_0; // divide by 0
  TA1CTL |= MC_1; // up mode - 01b = Up mode: Timer counts up to TAxCCR0
  TA1CCTL1 |= OUTMOD_7;
}

void init_timer_servo(void)
{ 
  P1DIR |= BIT7; // set as output 
  P1SEL0 |= BIT7; // set timer to output
  
  // not necessary 
//  TA0CTL = TACLR; //clear timer
//  TA0R=0; // set initial counter value to just zero

  // necessary
  TA0CCR0 = period; 
  TA0CCR1 = duty_cycle;
  
  TA0CTL |= TASSEL_1; // ACLK clock
  TA0CTL |= ID_0; // divide by 0
  TA0CTL |= MC_1; // up mode - 01b = Up mode: Timer counts up to TAxCCR0
  TA0CCTL1 |= OUTMOD_7;
}



//void servo_control (signed int ldr_diff)
//{
//  pulse_width = map(current_angle, 0, 180, servo_lower_limit_count,servo_upper_limit_count);
//  TA0CCR1 = pulse_width;
//
//}

void update_servo()
{
  
  pulse_width = map(current_angle, 0, 180, servo_lower_limit_count,servo_upper_limit_count);
  TA0CCR1 = pulse_width;
//  pulse_width=18;
//  TA0CTL |= TACLR;
//  __delay_cycles(2000);  
//  TA0CCR1 = pulse_width;
//  TA1CCR1=1600;
}



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    __enable_interrupt();
    
    init_timer_servo();
//    init_timer_LED_flashing();
    LDRdiff= LDR1-LDR2;
    __delay_cycles(5000000);
//    servo_control(LDRdiff);
    update_servo();
    
    while(1)
    {
        
        __delay_cycles(2000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}
