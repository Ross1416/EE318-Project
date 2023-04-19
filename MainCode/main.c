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
unsigned int adcTest=0;

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
#define LDR_L_PORT GPIO_PORT_P8
#define LDR_L_PIN GPIO_PIN0

#define LDR_R_PORT GPIO_PORT_P1
#define LDR_R_PIN GPIO_PIN6

unsigned int ldr_r_val=0;
unsigned int ldr_l_val=0;

// MUX VARIABLES
#define MUX_A_PORT GPIO_PORT_P5
#define MUX_A_PIN GPIO_PIN0

#define MUX_B_PORT GPIO_PORT_P5
#define MUX_B_PIN GPIO_PIN2

#define MUX_C_PORT GPIO_PORT_P5
#define MUX_C_PIN GPIO_PIN3

#define MUX_INH_PORT GPIO_PORT_P5
#define MUX_INH_PIN GPIO_PIN1

// ADC INTERRUPT
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
  {
    case ADCIV_ADCIFG:
//        adc[adc_count]=ADCMEM0;
//        if (adc_count <= 0)
//            adc_count = 9;
//        else
//            adc_count--;
      potValue=ADCMEM0;
      ADCIFG &= ~0x01;
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
    GPIO_clearInterrupt(FIXED_BTN_PORT, FIXED_LED_PIN);
    break;
  case MANUAL_BTN_INT:
    config=MANUAL;
    GPIO_clearInterrupt(MANUAL_BTN_PORT, MANUAL_BTN_PIN);
    break;
  case TRACKING_BTN_INT:
    config=TRACKING;
    GPIO_clearInterrupt(TRACKING_BTN_PORT, TRACKING_BTN_PIN);
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

    // SET ADC INPUTS AS INPUTS????

}

void select_led(unsigned int led) {
    GPIO_setOutputLowOnPin(MUX_A_PORT, MUX_A_PIN);
    GPIO_setOutputLowOnPin(MUX_B_PORT, MUX_B_PORT);
    GPIO_setOutputLowOnPin(MUX_C_PORT, MUX_C_PORT);
}

void init_MUX(void)
{
    GPIO_setAsOutputPin(MUX_A_PORT, MUX_A_PIN);
    GPIO_setAsOutputPin(MUX_B_PORT, MUX_B_PIN);
    GPIO_setAsOutputPin(MUX_C_PORT, MUX_C_PIN);
    GPIO_setAsOutputPin(MUX_INH_PORT, MUX_INH_PIN);

    GPIO_setOutputLowOnPin(MUX_A_PORT, MUX_A_PIN);
    GPIO_setOutputLowOnPin(MUX_B_PORT, MUX_B_PIN);
    GPIO_setOutputLowOnPin(MUX_C_PORT, MUX_C_PIN);
    GPIO_setOutputLowOnPin(MUX_INH_PORT, MUX_INH_PIN);

}

// SETUP ADC FOR MEASURING POTENTIOMETER
void init_POT_ADC(void)
{
    // makes no damn difference
  SYSCFG2 |= ADCPCTL9 | ADCPCTL8 | ADCPCTL6; // Enable A9,A8,A6;
  SYSCFG2 &= ~ADCPCTL7;
//    SYSCFG2 |= 0x0340;
//    SYSCFG2 |= 0x0000;
  ADCCTL0 |= ADCON | ADCSHT_2;// | ADCMSC; //Turn on ADC & setting sample and hold time as 16 ADCCLK cycles? set up multiple sample and conversion - could remove if not sampling anything for fixed
  ADCCTL1 |= ADCSHP; // sets the source of the sampling signal (SAMPCON) to the sampling timer? - or sets it to pulse sampling mode?
  ADCCTL2 |= ADCRES; //Set 10 bit resolution

  ADCMCTL0 |= ADCINCH_9; // Setting reference and input channel select to A9


  ADCCTL1 |= ADCCONSEQ_1;  //ADC conversion sequence mode select - 10b = Repeat-sequence-of-channels

  ADCCTL1 |= ADCSSEL_2;
//  ADCCTL2 |= ADCSR;

//  ADCCTL1 |= ADCSSEL_2 | ADCDIV_0;//aclk, divive by 1
//  ADCCTL2 |= ADCPDIV_0; // predivide by 0

//  ADCCTL1 |= ADCSSEL_1 | ADCDIV_7; // ACLK source, /8
//  ADCCTL2 |= ADCPDIV_2; //predivide by 64 => 62.5Hz clock signal?

  ADCIFG &= ~0x01; // clear interrupt - why this bit?
//  ADCIE |= ADCIE0; // Enable interrupt;

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

//  TA0CCTL0 |= CM_1; //capture on rising edge
//  TA0CCTL0 |= CCIS_0; // use CCIOA
//  TA0CCTL0 |= SCS; // Synchronise capture
//  TA0CCTL0 |=
//
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
    config = MANUAL;
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
//    CSCTL1 |= DCORSEL_5; // made no difference
    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    // could replace with "PM5CTL0 &= ~LOCKLPM5;" if want to remove driverlib.h
    PMM_unlockLPM5();

    __enable_interrupt();
    init_select_btns_leds();

    init_timer_servo();
//    update_servo();
    init_POT_ADC();
    init_MUX();

    ADCCTL0 |= ADCENC | ADCSC;// | ADCMSC; // enable conversion and start conversion

    while(1)
    {

        // ##### TEST THIS ######
//      while (ADCCTL0 & ADCBUSY){
//      }
      // CHECK WHICH CONFIGURATION IS SELECTED - FIXED BY DEFAULT
      switch(config)
      {
      case FIXED:
        current_angle=91;
        ADCCTL0 &= ~(ADCENC);
//        ADCCTL0 |= ADCSC; // start conversio
        GPIO_setOutputHighOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
        GPIO_setOutputLowOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
        GPIO_setOutputLowOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
        update_servo();
        break;
      case MANUAL:
          // this seems to sort of work but it adds a slight delay
          ADCCTL0 |= ADCENC | ADCSC;// | ADCMSC;
//          if (adcTest > 20)
//          {
//              ADCCTL0 |= ADCSC; // start conversion
//              adcTest = 0;
//          }
//          else
//              adcTest++;

          // none of this seems to work
//          while (ADCCTL1 & ADCBUSY)
//          {
//              adc[adc_count]=ADCMEM0;
//              if (adc_count <= 0)
//                  adc_count = 9;
//              else
//                  adc_count--;
//          }
//          while (ADCIFG0 == 0);
//        ADCCTL0 |= ADCSC; // start conversio
//        potValue=adc[9];
        potAngle = map(potValue,0,1023,0,180);
        current_angle=180-potAngle;
        GPIO_setOutputHighOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
        GPIO_setOutputLowOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
        GPIO_setOutputLowOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
        update_servo();
        break;
      case TRACKING:
//        ADCCTL0 |= ADCSC; // start conversion
//        ldr_r_val=adc[8];
//        ldr_l_val=adc[6];
        GPIO_setOutputHighOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
        GPIO_setOutputLowOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
        GPIO_setOutputLowOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
        break;
      }


//        __delay_cycles(5000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}
