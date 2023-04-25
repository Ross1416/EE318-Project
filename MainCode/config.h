/*
 * setup.h
 *
 *  Created on: 19 Apr 2023
 *      Author: ringl
 */

#ifndef CONFIG_H_
#define CONFIG_H_

// SERVO MOTOR VARIABLES
#define SERVO_MOTOR_PORT GPIO_PORT_P1
#define SERVO_MOTOR_PIN GPIO_PIN7


unsigned int angle_step_size = 1;

const int servo_lower_limit_count = 18; // 0 degrees
const int servo_upper_limit_count = 74; // 180 degrees

const unsigned int period = 640; // 50 Hz
volatile unsigned int pulse_width=18; // initially 0 degrees

//// POTENTIOMETER VALUES
//#define POT_PORT GPIO_PORT_P1
//#define POT_PIN GPIO_PIN6



// ADC VARIABLES
unsigned int adc[7]={0};
unsigned int adc_count=6;
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
#define FIXED_BTN_PIN GPIO_PIN1
#define FIXED_BTN_INT P1IV_P1IFG1

#define MANUAL_BTN_PORT GPIO_PORT_P2
#define MANUAL_BTN_PIN GPIO_PIN7
#define MANUAL_BTN_INT P2IV_P2IFG7

#define TRACKING_BTN_PORT GPIO_PORT_P2
#define TRACKING_BTN_PIN GPIO_PIN5
#define TRACKING_BTN_INT P2IV_P2IFG5

// SELECT LED VARIABLES
#define FIXED_LED_PORT GPIO_PORT_P5
#define FIXED_LED_PIN GPIO_PIN0

#define MANUAL_LED_PORT GPIO_PORT_P5
#define MANUAL_LED_PIN GPIO_PIN2

#define TRACKING_LED_PORT GPIO_PORT_P5
#define TRACKING_LED_PIN GPIO_PIN3

// LDR VARIABLES
#define LDR_L_PORT GPIO_PORT_P1
#define LDR_L_PIN GPIO_PIN3

#define LDR_R_PORT GPIO_PORT_P1
#define LDR_R_PIN GPIO_PIN4

int LDR_INDICATOR_TOLERANCES[] = {10,20,30};

// MUX VARIABLES
#define MUX_A_PORT GPIO_PORT_P8
#define MUX_A_PIN GPIO_PIN3

#define MUX_B_PORT GPIO_PORT_P8
#define MUX_B_PIN GPIO_PIN2

#define MUX_C_PORT GPIO_PORT_P5
#define MUX_C_PIN GPIO_PIN1

#define MUX_INH_PORT GPIO_PORT_P8
#define MUX_INH_PIN GPIO_PIN0



#endif /* CONFIG_H_ */
