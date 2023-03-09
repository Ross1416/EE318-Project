#include <msp430.h>
#include <driverlib.h>

unsigned int current_angle = 45;
unsigned int angle_step_size = 5;

int servo_lower_limit_count = 18; // 0 degrees
int servo_upper_limit_count = 74; // 180 degrees

// For Testing
int LDR1=560;
int LDR2=450;

int LDRdiff=0;

volatile unsigned int period = 640; // 50 Hz
volatile unsigned int pulse_width=18; // initially 0 degrees

volatile unsigned int potAngle=0;

enum CONFIGURATION
{
  FIXED,
  MANUAL,
  TRACKING
};

enum CONFIGURATION config = MANUAL;

//https://www.arduino.cc/reference/en/language/functions/math/map/
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch(ADCIV)
  {
    case ADCIV_ADCIFG:

      potAngle = map(ADCMEM0,0,1023,0,180);
//      __bis_SR_register_on_exit(LPM0_bits); // Low power mode select? - not low power mode?
      break;
  }
}




void init_timer_servo(void)
{ 
  P1DIR |= BIT7; // set as output 
  P1SEL0 |= BIT7; // set timer to output
  
  // not necessary 
  TA0CTL = TACLR; //clear timer
  TA0R=0; // set initial counter value to just zero

  // necessary
  TA0CCR0 = period; 
  TA0CCR1 = pulse_width;
  
  TA0CTL |= TASSEL_1; // ACLK clock
  TA0CTL |= ID_0; // divide by 0
  TA0CTL |= MC_1; // up mode - 01b = Up mode: Timer counts up to TAxCCR0
  TA0CCTL1 |= OUTMOD_7;
}

void init_POT_ADC(void)
{    
  SYSCFG2 |= ADCPCTL9; // Enable A9;
  
  ADCCTL0 |= ADCON | ADCSHT_2;// | ADCMSC; //Turn on ADC & setting sample and hold time as 16 ADCCLK cycles?
  ADCCTL1 |= ADCSHP; // sets the source of the sampling signal (SAMPCON) to the sampling timer? - or sets it to pulse sampling mode?
  ADCCTL2 |= ADCRES; //Set 10 bit resolution 
  
  ADCMCTL0 |= ADCINCH_9; // Setting reference and input channel select to A1
  
  ADCCTL1 |= ADCCONSEQ_2;  //ADC conversion sequence mode select - 10b = Repeat-sequence-of-channels
  
  ADCIFG &= ~0x01; // clear interrupt - why this bit?
  ADCIE |= ADCIE0; // Enable interrupt;
  
}



void update_servo()
{
  pulse_width = map(current_angle, 0, 180, servo_lower_limit_count,servo_upper_limit_count);
  TA0CCR1 = pulse_width;
}



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    __enable_interrupt();
    
    init_timer_servo();
    LDRdiff= LDR1-LDR2;
//    __delay_cycles(2000000);
    update_servo();
    init_POT_ADC();

    ADCCTL0 |= ADCENC | ADCSC; // enable conversion and start conversion

    while(1)
    {
      ADCCTL0 |= ADCSC; // start conversion
      switch(config)
      {
      case FIXED:
        current_angle=90;
        update_servo();
        break;
      case MANUAL:
        current_angle=180-potAngle;
        update_servo();
        break;
      case TRACKING:
        break;
      }
          
          
        __delay_cycles(2000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}
