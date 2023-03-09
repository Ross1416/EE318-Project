#include <msp430.h>
#include <driverlib.h>


// SERVO MOTOR VARIABLES
#define SERVO_MOTOR_PORT GPIO_PORT_P1
#define SERVO_MOTOR_PIN GPIO_PIN7

unsigned int current_angle = 45;
unsigned int angle_step_size = 5;

const int servo_lower_limit_count = 18; // 0 degrees
const int servo_upper_limit_count = 74; // 180 degrees

const unsigned int period = 640; // 50 Hz
volatile unsigned int pulse_width=18; // initially 0 degrees

// POTENTIOMETER VALUES
#define POT_MOTOR_PORT GPIO_PORT_P8
#define POT_MOTOR_PIN GPIO_PIN1

volatile unsigned int potValue=0;
unsigned int potAngle=0;

// ADC VARIABLES
unsigned int adc[10]={0};
unsigned int adc_count=9;

// CONFIGURATION VARIABLES
enum CONFIGURATION
{
  FIXED,
  MANUAL,
  TRACKING
};

enum CONFIGURATION config = FIXED;

// SELECT BUTTON VARIBLES
#define FIXED_BTN_PORT GPIO_PORT_P1
#define FIXED_BTN_PIN GPIO_PIN3
#define FIXED_BTN_INT P1IV_P1IFG3

#define MANUAL_BTN_PORT GPIO_PORT_P1
#define MANUAL_BTN_PIN GPIO_PIN4
#define MANUAL_BTN_INT P1IV_P1IFG4

#define TRACKING_BTN_PORT GPIO_PORT_P1
#define TRACKING_BTN_PIN GPIO_PIN5
#define TRACKING_BTN_INT P1IV_P1IFG5

// SELECT LED VARIABLES
#define FIXED_LED_PORT GPIO_PORT_P2
#define FIXED_LED_PIN GPIO_PIN5

#define MANUAL_LED_PORT GPIO_PORT_P8
#define MANUAL_LED_PIN GPIO_PIN2

#define TRACKING_LED_PORT GPIO_PORT_P8
#define TRACKING_LED_PIN GPIO_PIN3

// LDR VARIABLES
#define LDR_L_PORT8
#define LDR_L_PIN0

#define LDR_R_PORT1
#define LDR_R_PIN6

unsigned int ldr_r_val=0;
unsigned int ldr_l_val=0;


// ADC INTERRUPT
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
  {
    case ADCIV_ADCIFG:
        adc[adc_count]=ADCMEM0;
        if (adc_count <= 0)
            adc_count = 9;
        else
            adc_count--;
//      potValue=ADCMEM0;
//      __bis_SR_register_on_exit(LPM0_bits); // Low power mode select? - not low power mode?
      break;
  }
}

// PORT1 INTERRUPT
#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
  switch(P1IV)
  {
  case FIXED_BTN_INT:
    config=FIXED;
//    GPIO_clearInterrupt(FIXED_BTN_PORT, FIXED_LED_PIN); //not actually necessary
    break;
  case MANUAL_BTN_INT:
    config=MANUAL;
//    GPIO_clearInterrupt(MANUAL_BTN_PORT, MANUAL_BTN_PIN);
    break;
  case TRACKING_BTN_INT:
    config=TRACKING;
//    GPIO_clearInterrupt(TRACKING_BTN_PORT, TRACKING_BTN_PIN);
    break;
  default:
    break;
  }
}




//https://www.arduino.cc/reference/en/language/functions/math/map/
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



// SETUP SELECT BUTTONS AND LEDS
void init_select_btns_leds(void)
{
    // FIXED MODE SELECT BTN
    GPIO_selectInterruptEdge(FIXED_BTN_PORT, FIXED_BTN_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPin(FIXED_BTN_PORT, FIXED_BTN_PIN);
    GPIO_clearInterrupt(FIXED_BTN_PORT, FIXED_BTN_PIN);
    GPIO_enableInterrupt(FIXED_BTN_PORT, FIXED_BTN_PIN);

    // MANUAL MODE SELECT BTN
    GPIO_selectInterruptEdge(MANUAL_BTN_PORT, MANUAL_BTN_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPin(MANUAL_BTN_PORT, MANUAL_BTN_PIN);
    GPIO_clearInterrupt(MANUAL_BTN_PORT, MANUAL_BTN_PIN);
    GPIO_enableInterrupt(MANUAL_BTN_PORT, MANUAL_BTN_PIN);

    // TRACKING MODE SELECT BTN
    GPIO_selectInterruptEdge(TRACKING_BTN_PORT, TRACKING_BTN_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPin(TRACKING_BTN_PORT, TRACKING_BTN_PIN);
    GPIO_clearInterrupt(TRACKING_BTN_PORT, TRACKING_BTN_PIN);
    GPIO_enableInterrupt(TRACKING_BTN_PORT, TRACKING_BTN_PIN);

    // FIXED MODE LED
    GPIO_setAsOutputPin(FIXED_LED_PORT, FIXED_LED_PIN);
    // MANUAL MODE LED
    GPIO_setAsOutputPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
    // TRACKING MODE LED
    GPIO_setAsOutputPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
}

// SETUP ADC FOR MEASURING POTENTIOMETER
void init_POT_ADC(void)
{
  SYSCFG2 |= ADCPCTL9;// | ADCPCTL8 | ADCPCTL6; // Enable A9,A8,A6;

  ADCCTL0 |= ADCON | ADCSHT_2;// | ADCMSC; //Turn on ADC & setting sample and hold time as 16 ADCCLK cycles? set up multiple sample and conversion - could remove if not sampling anything for fixed
  ADCCTL1 |= ADCSHP; // sets the source of the sampling signal (SAMPCON) to the sampling timer? - or sets it to pulse sampling mode?
  ADCCTL2 |= ADCRES; //Set 10 bit resolution

  ADCMCTL0 |= ADCINCH_9; // Setting reference and input channel select to A9

  ADCCTL1 |= ADCCONSEQ_1;  //ADC conversion sequence mode select - 10b = Repeat-sequence-of-channels

//  ADCCTL2 |= ADCSR;

//  ADCCTL1 |= ADCSSEL_2 | ADCDIV_0;//aclk, divive by 1
//  ADCCTL2 |= ADCPDIV_0; // predivide by 0

//  ADCCTL1 |= ADCSSEL_1 | ADCDIV_7; // ACLK source, /8
//  ADCCTL2 |= ADCPDIV_2; //predivide by 64 => 62.5Hz clock signal?

  ADCIFG &= ~0x01; // clear interrupt - why this bit?
  ADCIE |= ADCIE0; // Enable interrupt;

}


// SETUP SERVO MOTOR TIMER
void init_timer_servo(void)
{
  GPIO_setAsOutputPin(SERVO_MOTOR_PORT, SERVO_MOTOR_PIN);
//  P1DIR |= BIT7; // set as output
  GPIO_setAsPeripheralModuleFunctionOutputPin(SERVO_MOTOR_PORT, SERVO_MOTOR_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
//  P1SEL0 |= BIT7; // set timer to output

  // not necessary
  TA0CTL = TACLR; //clear timer
  TA0R=0; // set initial counter value to just zero

  // necessary - better way to code this
  TA0CCR0 = period;
  TA0CCR1 = pulse_width; // for pin 1.7
//  TA0CCR2 = pulse_width; // for pin 1.6? - doesn't work for some reason

  TA0CTL |= TASSEL_1; // ACLK clock
  TA0CTL |= ID_0; // divide by 0
  TA0CTL |= MC_1; // up mode - 01b = Up mode: Timer counts up to TAxCCR0
  TA0CCTL1 |= OUTMOD_7; // Reset-set mode
}


// UPDATE SERVO ANGLE
void update_servo()
{
  pulse_width = map(current_angle, 0, 180, servo_lower_limit_count,servo_upper_limit_count);
  TA0CCR1 = pulse_width; // for pin 1.7
//  TA0CCR2 = pulse_width; // for pin 1.6? - doesnt work for some reason
}



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    // could replace with "PM5CTL0 &= ~LOCKLPM5;" if want to remove driverlib.h
    PMM_unlockLPM5();
    init_select_btns_leds();
    __enable_interrupt();

    init_timer_servo();
//    update_servo();
    init_POT_ADC();

    ADCCTL0 |= ADCENC | ADCSC; // enable conversion and start conversion

    while(1)
    {



      // CHECK WHICH CONFIGURATION IS SELECTED - FIXED BY DEFAULT
      switch(config)
      {
      case FIXED:
        current_angle=91;
        GPIO_setOutputHighOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
        GPIO_setOutputLowOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
        GPIO_setOutputLowOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
        update_servo();
        break;
      case MANUAL:
        ADCCTL0 |= ADCSC; // start conversion
        potValue=adc[9];
        potAngle = map(potValue,0,1023,0,180);
        current_angle=180-potAngle;
        GPIO_setOutputHighOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
        GPIO_setOutputLowOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
        GPIO_setOutputLowOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
        update_servo();
        break;
      case TRACKING:
        ADCCTL0 |= ADCSC; // start conversion
        ldr_r_val=adc[8];
        ldr_l_val=adc[6];
        GPIO_setOutputHighOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
        GPIO_setOutputLowOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
        GPIO_setOutputLowOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
        break;
      }


        __delay_cycles(2000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}
