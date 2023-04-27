#include <msp430.h>
#include <driverlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "LedDriver.h"

int dir = 1;
int dir2 = 1;
bool SW_interruptFlag_ = 0;
volatile unsigned int speed = 0;
float voltage1=0.0;
int voltage2=0;
int voltage3=0;
int voltagedecima=0;
char buffer[18];

#define RESISTOR_1 18000
#define RESISTOR_2 18000
int power=0;
int powerdecima=0;

// Define word access definitions to LCD memories
#define LCDMEMW ((int*)LCDMEM)

// Workaround LCDBMEM definition bug in IAR header file
#ifdef __IAR_SYSTEMS_ICC__
#define LCDBMEMW ((int*)&LCDM32)
#else
#define LCDBMEMW ((int*)LCDBMEM)
#endif



//---------------------------------------------------||----------------------------------------------------\\
//                                           LCD Configuration
//---------------------------------------------------||----------------------------------------------------\\

#define pos1 4                                               // Digit A1 - L4
#define pos2 6                                               // Digit A2 - L6
#define pos3 8                                               // Digit A3 - L8
#define pos4 10                                              // Digit A4 - L10
#define pos5 2                                               // Digit A5 - L2
#define pos6 18                                              // Digit A6 - L18


// LCD memory map for numeric digits
const char digit2[10][2] =
{
    {0xFC, 0x28},  /* "0" LCD segments a+b+c+d+e+f+k+q */
    {0x60, 0x20},  /* "1" */
    {0xDB, 0x00},  /* "2" */
    {0xF3, 0x00},  /* "3" */
    {0x67, 0x00},  /* "4" */
    {0xB7, 0x00},  /* "5" */
    {0xBF, 0x00},  /* "6" */
    {0xE4, 0x00},  /* "7" */
    {0xFF, 0x00},  /* "8" */
    {0xF7, 0x00}   /* "9" */
};

// LCD memory map for uppercase letters
const char alphabetBig[26][2] =
{
    {0xEF, 0x00},  /* "A" LCD segments a+b+c+e+f+g+m */
    {0xF1, 0x50},  /* "B" */
    {0x9C, 0x00},  /* "C" */
    {0xF0, 0x50},  /* "D" */
    {0x9F, 0x00},  /* "E" */
    {0x8F, 0x00},  /* "F" */
    {0xBD, 0x00},  /* "G" */
    {0x6F, 0x00},  /* "H" */
    {0x90, 0x50},  /* "I" */
    {0x78, 0x00},  /* "J" */
    {0x0E, 0x22},  /* "K" */
    {0x1C, 0x00},  /* "L" */
    {0x6C, 0xA0},  /* "M" */
    {0x6C, 0x82},  /* "N" */
    {0xFC, 0x00},  /* "O" */
    {0xCF, 0x00},  /* "P" */
    {0xFC, 0x02},  /* "Q" */
    {0xCF, 0x02},  /* "R" */
    {0xB7, 0x00},  /* "S" */
    {0x80, 0x50},  /* "T" */
    {0x7C, 0x00},  /* "U" */
    {0x0C, 0x28},  /* "V" */
    {0x6C, 0x0A},  /* "W" */
    {0x00, 0xAA},  /* "X" */
    {0x00, 0xB0},  /* "Y" */
    {0x90, 0x28}   /* "Z" */
};


const char digit[10] =
{
    0xFC,   // "0"
    0x60,   // "1"
    0xDB,   // "2"
    0xF3,   // "3"
    0x67,   // "4"
    0xB7,   // "5"
    0xBF,   // "6"
    0xE4,   // "7"
    0xFF,   // "8"
    0xF7    // "9"
};

//---------------------------------------------------||----------------------------------------------------\\

volatile unsigned char * Seconds = &BAKMEM0_L;               // Store seconds in the backup RAM module
volatile unsigned char * Minutes = &BAKMEM0_H;               // Store minutes in the backup RAM module
volatile unsigned char * Hours = &BAKMEM1_L;                 // Store hours in the backup RAM module

//---------------------------------------------------||----------------------------------------------------\\

void Init_GPIO(void);
void Init_LCD(void);
void displayScrollText(char*);
void displayText2(char *msg);
void showChar(char, int);
void clearLCD(void);

//---------------------------------------------------||----------------------------------------------------\\

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
  switch(P1IV)
  {
  case P1IV_P1IFG3:
    if(dir2 == 1)
      dir2 = 0;
    else
      dir2 = 1;
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
    break;
  default:
    break;
  }
}

#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
{
  switch(ADCIV)
  {
    case ADCIV_ADCIFG:
      speed = 1+(ADCMEM0)/10;

      break;
  }
}

//---------------------------------------------------||----------------------------------------------------\\

void init_ADC(void)
{

  // Using other way
  GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P8, GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);

  ADC_init(ADC_BASE,
           ADC_SAMPLEHOLDSOURCE_SC,
           ADC_CLOCKSOURCE_ADCOSC,
           //ADC_CLOCKDIVIDER_1);
           ADC_CLOCKDIVIDER_512);


  ADC_enable(ADC_BASE);

  ADC_configureMemory(ADC_BASE,
                      ADC_INPUT_A9,
                      ADC_VREFPOS_AVCC,
                      ADC_VREFNEG_AVSS);

  ADC_setupSamplingTimer(ADC_BASE,
                         ADC_CYCLEHOLD_4_CYCLES,
                         //ADC_MULTIPLESAMPLESDISABLE);
                         ADC_MULTIPLESAMPLESENABLE);

  ADC_clearInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);
  ADC_enableInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT);

}

//---------------------------------------------------||----------------------------------------------------\\

void init_interrupts(void)
{
    // Setup SW1 as interrupt
  GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN3, GPIO_HIGH_TO_LOW_TRANSITION);
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN3);


  GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
  GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN3); // enable SW1 interrupt

}

//---------------------------------------------------||----------------------------------------------------\\

int main( void )
{
    WDTCTL = WDTPW | WDTHOLD;                               // Stop watchdog timer

        unsigned char dialValue = 0x01;
      //

        // Configure XT1 oscillator
        P4SEL0 |= BIT1 | BIT2;                              // P4.2~P4.1: crystal pins
        do
        {
            CSCTL7 &= ~(XT1OFFG | DCOFFG);                  // Clear XT1 and DCO fault flag
            SFRIFG1 &= ~OFIFG;
        }while (SFRIFG1 & OFIFG);                           // Test oscillator fault flag
        CSCTL6 = (CSCTL6 & ~(XT1DRIVE_3)) | XT1DRIVE_2;     // Higher drive strength and current consumption for XT1 oscillator

        // Disable the GPIO power-on default high-impedance mode
        // to activate previously configured port settings
        PM5CTL0 &= ~LOCKLPM5;
        init_ADC();

        P1DIR &= ~BIT2;               // P1.1 (PUSH1) -> Input
         P1REN |= BIT2;                // P1.1 Pull Up/Down Enable
         P1OUT |= BIT2;              // P1.1 Pull Up Enable

       // Disable the GPIO power-on default high-impedance mode
       // to activate previously configured port settings
       PMM_unlockLPM5();
       initialiseLedDial();
       init_interrupts();

        // Configure RTC
        RTCCTL |= RTCSS__XT1CLK | RTCIE;                    // Initialize RTC to use XT1 and enable RTC interrupt
        RTCMOD = 32768;                                     // Set RTC modulo to 32768 to trigger interrupt each second

        // Configure LCD pins
        SYSCFG2 |= LCDPCTL;                                 // R13/R23/R33/LCDCAP0/LCDCAP1 pins selected

        LCDPCTL0 = 0xFFFF;
        LCDPCTL1 = 0x07FF;
        LCDPCTL2 = 0x00F0;                                  // L0~L26 & L36~L39 pins selected

        LCDCTL0 = LCDSSEL_0 | LCDDIV_7;                     // flcd ref freq is xtclk

        // LCD Operation - Mode 3, internal 3.08v, charge pump 256Hz
        LCDVCTL = LCDCPEN | LCDREFEN | VLCD_6 | (LCDCPFSEL0 | LCDCPFSEL1 | LCDCPFSEL2 | LCDCPFSEL3);

        LCDMEMCTL |= LCDCLRM;                               // Clear LCD memory

        LCDCSSEL0 = 0x000F;                                 // Configure COMs and SEGs
        LCDCSSEL1 = 0x0000;                                 // L0, L1, L2, L3: COM pins
        LCDCSSEL2 = 0x0000;

        LCDM0 = 0x21;                                       // L0 = COM0, L1 = COM1
        LCDM1 = 0x84;                                       // L2 = COM2, L3 = COM3

        LCDCTL0 |= LCD4MUX | LCDON;                         // Turn on LCD, 4-mux selected (LCD4MUX also includes LCDSON)


        while (1)
        {
            //    ADCCTL0 |= ADCSC; // start conversion

                  ADCCTL0 |= ADCENC | ADCSC; // Sampling and conversion start
                  while (ADCCTL0 & ADCBUSY); // Wait for conversion to complete
                  voltage1 =(((ADCMEM0) *3.3) / 1023); // 12bits - 4095 // 10bits - 1023
                  voltage2 =(((ADCMEM0) *3.3) / 1023)*(10); // Unidad: Voltios*10
                  voltage3 =(((ADCMEM0) *3.3) / 1023); // Unidad: DeciVoltios

               if (dir)
                {
                  dialValue = dialValue * 0x02;
                  if(0x00 == dialValue)
                   dialValue = 0x01;
                }
                else
                {
                  if(0x01 == dialValue)
                   dialValue = 0x80;
                  else
                    dialValue = dialValue / 0x02;

                }

               if (voltage1>=0.5)
               {
                //Set value
                setLedDial(dialValue);
               }
               else if (voltage1<0.5)
               {

                setLedDial(0);
              }
                //for(i = 0; i < 10; i++)
                refreshLedDial(speed);

                int SquaredVoltage = voltage2 * voltage2;
                int Rtotal = 3600*10;

                // Convierte uno de los operandos a float para evitar la división entera
                float Division = SquaredVoltage / (float)Rtotal;

                // Multiplica por 1000000 y convierte el resultado a int
                int power = (int)(Division * 100000);

                if(!(P1IN & BIT2))         // If PUSH1 is pressed
                           {

                    if (dir2==0)
                    {
                    dir2=1;
                    }
                    else if (dir2==1)
                    {
                        dir2=0;
                    }
                           while (!(P1IN & BIT2))
                               {
                               __delay_cycles(1048576); // DELAY 1S
                               }
                           }


                if (dir2==0)
                {
                    sprintf(buffer, "P%d    ",power);
                }
                else if (dir2==1)
                {
                sprintf(buffer, "V%d    ",voltage2);
                }


                displayText2(buffer);

        PMMCTL0_H = PMMPW_H;                                // Open PMM Registers for write
        PMMCTL0_L |= PMMREGOFF_L;                           // and set PMMREGOFF
        }
}

//---------------------------------------------------||----------------------------------------------------\\

void Init_GPIO()
{
    // Configure all GPIO to Output Low
    P1OUT = 0x00;P2OUT = 0x00;P3OUT = 0x00;P4OUT = 0x00;
    P5OUT = 0x00;P6OUT = 0x00;P7OUT = 0x00;P8OUT = 0x00;

    P1DIR = 0xFF;P2DIR = 0xFF;P3DIR = 0xFF;P4DIR = 0xFF;
    P5DIR = 0xFF;P6DIR = 0xFF;P7DIR = 0xFF;P8DIR = 0xFF;
}

//---------------------------------------------------||----------------------------------------------------\\

void Init_LCD()
{
    // L0~L26 & L36~L39 pins selected
    LCD_E_setPinAsLCDFunctionEx(LCD_E_BASE, LCD_E_SEGMENT_LINE_0, LCD_E_SEGMENT_LINE_26);
    LCD_E_setPinAsLCDFunctionEx(LCD_E_BASE, LCD_E_SEGMENT_LINE_36, LCD_E_SEGMENT_LINE_39);

    LCD_E_initParam initParams = LCD_E_INIT_PARAM;
    initParams.clockDivider = LCD_E_CLOCKDIVIDER_3;
    initParams.muxRate = LCD_E_4_MUX;
    initParams.segments = LCD_E_SEGMENTS_ENABLED;

    // Init LCD as 4-mux mode
    LCD_E_init(LCD_E_BASE, &initParams);

    // LCD Operation - Mode 3, internal 3.02v, charge pump 256Hz
    LCD_E_setVLCDSource(LCD_E_BASE, LCD_E_INTERNAL_REFERENCE_VOLTAGE, LCD_E_EXTERNAL_SUPPLY_VOLTAGE);
    LCD_E_setVLCDVoltage(LCD_E_BASE, LCD_E_REFERENCE_VOLTAGE_2_96V);

    LCD_E_enableChargePump(LCD_E_BASE);
    LCD_E_setChargePumpFreq(LCD_E_BASE, LCD_E_CHARGEPUMP_FREQ_16);

    // Clear LCD memory
    LCD_E_clearAllMemory(LCD_E_BASE);

    // Configure COMs and SEGs
    // L0 = COM0, L1 = COM1, L2 = COM2, L3 = COM3
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_0, LCD_E_MEMORY_COM0);
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_1, LCD_E_MEMORY_COM1);
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_2, LCD_E_MEMORY_COM2);
    LCD_E_setPinAsCOM(LCD_E_BASE, LCD_E_SEGMENT_LINE_3, LCD_E_MEMORY_COM3);

    // Select to display main LCD memory
    LCD_E_selectDisplayMemory(LCD_E_BASE, LCD_E_DISPLAYSOURCE_MEMORY);

    // Turn on LCD
    LCD_E_on(LCD_E_BASE);
}

//---------------------------------------------------||----------------------------------------------------\\

/*
 * Scrolls input string across LCD screen from left to right
 */
void displayScrollText(char *msg)
{
    int length = strlen(msg);
    int i;
    int s = 5;
    char buffer[6] = "      ";
    for (i=0; i<length+7; i++)
    {
        int t;
        for (t=0; t<6; t++)
            buffer[t] = ' ';
        int j;
        for (j=0; j<length; j++)
        {
            if (((s+j) >= 0) && ((s+j) < 6))
                buffer[s+j] = msg[j];
        }
        s--;

        showChar(buffer[0], pos1);
        showChar(buffer[1], pos2);
        showChar(buffer[2], pos3);
        showChar(buffer[3], pos4);
        showChar(buffer[4], pos5);
        showChar(buffer[5], pos6);

        __delay_cycles(20000);
    }
}

//---------------------------------------------------||----------------------------------------------------\\

void displayText2(char *msg)
{
    int length = strlen(msg);
    char buffer[6] = "      ";

    int j;

    for (j=0; j<length; j++)
            {
                    buffer[j] = msg[j];
            }

    showChar(buffer[0], pos1);
    showChar(buffer[1], pos2);
    showChar(buffer[2], pos3);
    showChar(buffer[3], pos4);
    showChar(buffer[4], pos5);
    showChar(buffer[5], pos6);


}

//---------------------------------------------------||----------------------------------------------------\\

/*
 * Displays input character at given LCD digit/position
 * Only spaces, numeric digits, and uppercase letters are accepted characters
 */
void showChar(char c, int position)
{
    if (c == ' ')
    {
        // Display space
        LCDMEMW[position/2] = 0;
    }
    else if (c >= '0' && c <= '9')
    {
        // Display digit
        LCDMEMW[position/2] = digit2[c-48][0] | (digit2[c-48][1] << 8);
    }
    else if (c >= 'A' && c <= 'Z')
    {
        // Display alphabet
        LCDMEMW[position/2] = alphabetBig[c-65][0] | (alphabetBig[c-65][1] << 8);
    }
    else
    {
        // Turn all segments on if character is not a space, digit, or uppercase letter
        LCDMEMW[position/2] = 0xFFFF;
    }
}

//---------------------------------------------------||----------------------------------------------------\\

/*
 * Clears memories to all 6 digits on the LCD
 */
void clearLCD()
{
    LCDMEMW[pos1/2] = 0;
    LCDMEMW[pos2/2] = 0;
    LCDMEMW[pos3/2] = 0;
    LCDMEMW[pos4/2] = 0;
    LCDMEMW[pos5/2] = 0;
    LCDMEMW[pos6/2] = 0;
    LCDMEM[12] = LCDMEM[13] = 0;
}

//---------------------------------------------------||----------------------------------------------------\\
//---------------------------------------------------||----------------------------------------------------\\
//---------------------------------------------------||----------------------------------------------------\\
//---------------------------------------------------||----------------------------------------------------\\
//---------------------------------------------------||----------------------------------------------------\\


