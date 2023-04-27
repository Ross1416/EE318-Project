/**************************************************
 * LEDDriver.c
 * Drives LED display
 *
 * Copyright 2015 University of Strathclyde
 *
 *
 **************************************************/

#include "LedDriver.h"

//Current dial value
unsigned char ledValue_ = 0;

/**
Initialise LED Dial, setting GPIO parameters
*/
void initialiseLedDial()
{
  //GPIO 2.7
  GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);
    
  //GPIO 5.1, 5.2, 5.3
  GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3);
    
  //GPIO 8.0
  GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0);


}

/** 
Refresh the display
*/
void refreshLedDial(int speed)
{
  //Refresh lower bank (LEDs 1-4)
  if (ledValue_ <  0x10)
  {
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
    
    if (ledValue_ & 0x01)
      GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0);
    if (ledValue_ & 0x02)
      GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);
    if (ledValue_ & 0x04)
      GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
    if (ledValue_ & 0x08)
      GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN3);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN3);
    
  }
  else{
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
    
    if (!(ledValue_ & 0x80))
      GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0);
    if (!(ledValue_ & 0x40))
      GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);
    if (!(ledValue_ & 0x20))
      GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
    if (!(ledValue_ & 0x10))
      GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN3);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN3);
    
  }
  
  //speed range from 1000 to 20000
  //0-1024
  int i=0;
  for (i = 0; i<speed; i++)
    __delay_cycles(10000);
}

/** 
Set dial value
*/
void setLedDial(unsigned char value)
{
  ledValue_ = value;
}
