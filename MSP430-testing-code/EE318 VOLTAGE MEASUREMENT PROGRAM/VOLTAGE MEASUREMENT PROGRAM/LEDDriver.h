/**************************************************
 * LEDDriver.h
 * Drives LED display
 *
 * Copyright 2015 University of Strathclyde
 *
 *
 **************************************************/
#include <msp430.h>
#include <driverlib.h>

extern void initialiseLedDial();  //Initialise LED Dial, setting GPIO parameters
extern void setLedDial(unsigned char value); //Set dial value
extern void refreshLedDial(); //Refresh the display
