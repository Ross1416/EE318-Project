#include <config.h>
#include <msp430.h>
#include <driverlib.h>

// INIT VARIABLES
bool switched_mode;


// ADC INTERRUPT
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
  {
    case ADCIV_ADCIFG:
        adc[adc_count]=ADCMEM0;
        if (adc_count <= 0)
            adc_count = 6;
        else
            adc_count--;
//      potValue=ADCMEM0;
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
    switched_mode = true;
    GPIO_clearInterrupt(FIXED_BTN_PORT, FIXED_LED_PIN);
    break;
  }
}

// PORT2 INTERRUPT
#pragma vector = PORT2_VECTOR
__interrupt void P2_ISR(void)
{
  switch(P2IV)
  {
  case MANUAL_BTN_INT:
    config=MANUAL;
    switched_mode = true;
    GPIO_clearInterrupt(MANUAL_BTN_PORT, MANUAL_BTN_PIN);
    break;
  case TRACKING_BTN_INT:
    config=TRACKING;
    switched_mode = true;
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
void init_btns(void)
{
    // FIXED MODE SELECT BTN
    GPIO_selectInterruptEdge(FIXED_BTN_PORT, FIXED_BTN_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPin(FIXED_BTN_PORT, FIXED_BTN_PIN);                              // FOR VEROBOARD VERSION
//    GPIO_setAsInputPinWithPullDownResistor(FIXED_BTN_PORT, FIXED_BTN_PIN);        // FOR PCB VERSION
    GPIO_clearInterrupt(FIXED_BTN_PORT, FIXED_BTN_PIN);
    GPIO_enableInterrupt(FIXED_BTN_PORT, FIXED_BTN_PIN);

    // MANUAL MODE SELECT BTN
    GPIO_selectInterruptEdge(MANUAL_BTN_PORT, MANUAL_BTN_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPin(MANUAL_BTN_PORT, MANUAL_BTN_PIN);                            // FOR VEROBOARD VERSION
//    GPIO_setAsInputPinWithPullDownResistor(MANUAL_BTN_PORT, MANUAL_BTN_PIN);      // FOR PCB VERSION
    GPIO_clearInterrupt(MANUAL_BTN_PORT, MANUAL_BTN_PIN);
    GPIO_enableInterrupt(MANUAL_BTN_PORT, MANUAL_BTN_PIN);

    // TRACKING MODE SELECT BTN
    GPIO_selectInterruptEdge(TRACKING_BTN_PORT, TRACKING_BTN_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPin(TRACKING_BTN_PORT, TRACKING_BTN_PIN);                        // FOR VEROBOARD VERSION
//    GPIO_setAsInputPinWithPullDownResistor(TRACKING_BTN_PORT, TRACKING_BTN_PIN);  // FOR PCB VERSION
    GPIO_clearInterrupt(TRACKING_BTN_PORT, TRACKING_BTN_PIN);
    GPIO_enableInterrupt(TRACKING_BTN_PORT, TRACKING_BTN_PIN);
}

void init_leds(void)
{
    // FIXED MODE LED
    GPIO_setAsOutputPin(FIXED_LED_PORT, FIXED_LED_PIN);
    // MANUAL MODE LED
    GPIO_setAsOutputPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
    // TRACKING MODE LED
    GPIO_setAsOutputPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
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

void select_led(unsigned int led) {
    GPIO_setOutputLowOnPin(MUX_A_PORT, MUX_A_PIN);
    GPIO_setOutputLowOnPin(MUX_B_PORT, MUX_B_PORT);
    GPIO_setOutputLowOnPin(MUX_C_PORT, MUX_C_PORT);
}



// SETUP ADC FOR MEASURING POTENTIOMETER
void init_ADC(void)
{
    // makes no damn difference
  SYSCFG2 |= ADCPCTL6 | ADCPCTL5 | ADCPCTL4; // Enable A9,A8,A6;

  ADCCTL0 |= ADCON | ADCSHT_2;// | ADCMSC; //Turn on ADC & setting sample and hold time as 16 ADCCLK cycles? set up multiple sample and conversion - could remove if not sampling anything for fixed
  ADCCTL1 |= ADCSHP; // sets the source of the sampling signal (SAMPCON) to the sampling timer? - or sets it to pulse sampling mode?
  ADCCTL2 |= ADCRES; //Set 10 bit resolution

  ADCMCTL0 |= ADCINCH_6; // Setting reference and input channel select to A9

  ADCCTL1 |= ADCCONSEQ_1;  //ADC conversion sequence mode select - 10b = Repeat-sequence-of-channels

//  ADCCTL1 |= ADCSSEL_2;
//  ADCCTL2 |= ADCSR;

//  ADCCTL1 |= ADCSSEL_2 | ADCDIV_0;//aclk, divive by 1
//  ADCCTL2 |= ADCPDIV_0; // predivide by 0

//  ADCCTL1 |= ADCSSEL_1 | ADCDIV_7; // ACLK source, /8
//  ADCCTL2 |= ADCPDIV_2; //predivide by 64 => 62.5Hz clock signal?

  ADCIFG &= ~0x01; // clear interrupt - why this bit?
  ADCIE |= ADCIE0; // Enable interrupt;

}


// SETUP SERVO MOTOR TIMER
void init_servo_timer(void)
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
    config = FIXED;
    switched_mode = true;
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
//    CSCTL1 |= DCORSEL_5; // made no difference
    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    // could replace with "PM5CTL0 &= ~LOCKLPM5;" if want to remove driverlib.h
    PMM_unlockLPM5();

    __enable_interrupt();
    init_btns();
    init_leds();

    init_servo_timer();
//    update_servo();
    init_ADC();
    init_MUX();

    ADCCTL0 |= ADCENC | ADCSC;// | ADCMSC; // enable conversion and start conversion
//    ADCCTL0 |= ADCENC | ADCSC | ADCMSC;
    while(1)
    {

      switch(config)
      {
      case FIXED:
          if (switched_mode)
          {
              // Update servo to 90 degress
              current_angle=91;
              update_servo();
              // Disable ADC
              ADCCTL0 &= ~(ADCENC);

              // Update Mode LEDs
              GPIO_setOutputHighOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
              GPIO_setOutputLowOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
              GPIO_setOutputLowOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);

              switched_mode = false;
          }
        break;
      case MANUAL:
          if (switched_mode)
          {
              // Enable ADC
              ADCCTL0 |= ADCENC;

              // Updated Mode LEDs
              GPIO_setOutputHighOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
              GPIO_setOutputLowOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
              GPIO_setOutputLowOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);

              switched_mode = false;
          }
          else{
              // Update servo with respect to potentiometer
              //         | ADCSC;
              potValue=adc[6];
              potAngle = map(potValue,0,1023,0,180);
              current_angle=potAngle;//180-potAngle;

              update_servo();
          }


        break;
      case TRACKING:
          if (switched_mode)
          {
              // Update servo with respect to potentiometer
              GPIO_setOutputHighOnPin(TRACKING_LED_PORT, TRACKING_LED_PIN);
              GPIO_setOutputLowOnPin(FIXED_LED_PORT, FIXED_LED_PIN);
              GPIO_setOutputLowOnPin(MANUAL_LED_PORT, MANUAL_LED_PIN);
          }
          else{

          }
//        ADCCTL0 |= ADCSC; // start conversion
//        ldr_r_val=adc[8];
//        ldr_l_val=adc[6];

        break;
      }



    }
}
