// Graphics LCD Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD/Keyboard Interface
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red Backlight LED:
//   PB5 drives an NPN transistor that powers the red LED
// Green Backlight LED:
//   PE5 drives an NPN transistor that powers the green LED
// Blue Backlight LED:
//   PE4 drives an NPN transistor that powers the blue LED
// ST7565R Graphics LCD Display Interface:
//   MOSI (SSI2Tx) on PB7
//   MISO (SSI2Rx) is not used by the LCD display but the pin is used for GPIO for A0
//   SCLK (SSI2Clk) on PB4
//   A0 connected to PB6
//   ~CS connected to PB1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"

#define RED_BL_LED   (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 5*4)))
#define GREEN_BL_LED (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 5*4)))
#define BLUE_BL_LED  (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*4)))

#define CS_NOT       (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 1*4)))
#define A0           (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 6*4)))

// Set pixel arguments
#define CLEAR  0
#define SET    1
#define INVERT 2

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

uint8_t  pixelMap[1024];
uint16_t txtIndex = 0;

// 96 character 5x7 bitmaps based on ISO-646 (BCT IRV extensions)
const uint8_t charGen[100][5] = {
    // Codes 32-127
    // Space ! " % $ % & ' ( ) * + , - . /
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x4F, 0x00, 0x00},
    {0x00, 0x07, 0x00, 0x07, 0x00},
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    {0x23, 0x13, 0x08, 0x64, 0x62},
    {0x36, 0x49, 0x55, 0x22, 0x40},
    {0x00, 0x05, 0x03, 0x00, 0x00},
    {0x00, 0x1C, 0x22, 0x41, 0x00},
    {0x00, 0x41, 0x22, 0x1C, 0x00},
    {0x14, 0x08, 0x3E, 0x08, 0x14},
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    {0x00, 0x50, 0x30, 0x00, 0x00},
    {0x08, 0x08, 0x08, 0x08, 0x08},
    {0x00, 0x60, 0x60, 0x00, 0x00},
    {0x20, 0x10, 0x08, 0x04, 0x02},
    // 0-9
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46},
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36},
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    // : ; < = > ? @
    {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x00, 0x56, 0x36, 0x00, 0x00},
    {0x08, 0x14, 0x22, 0x41, 0x00},
    {0x14, 0x14, 0x14, 0x14, 0x14},
    {0x00, 0x41, 0x22, 0x14, 0x08},
    {0x02, 0x01, 0x51, 0x09, 0x3E},
    {0x32, 0x49, 0x79, 0x41, 0x3E},
    // A-Z
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x3A},
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31},
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x3F, 0x40, 0x70, 0x40, 0x3F},
    {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07},
    {0x61, 0x51, 0x49, 0x45, 0x43},
    // [ \ ] ^ _ `
    {0x00, 0x7F, 0x41, 0x41, 0x00},
    {0x02, 0x04, 0x08, 0x10, 0x20},
    {0x00, 0x41, 0x41, 0x7F, 0x00},
    {0x04, 0x02, 0x01, 0x02, 0x04},
    {0x40, 0x40, 0x40, 0x40, 0x40},
    {0x00, 0x01, 0x02, 0x04, 0x00},
    // a-z
    {0x20, 0x54, 0x54, 0x54, 0x78},
    {0x7F, 0x44, 0x44, 0x44, 0x38},
    {0x38, 0x44, 0x44, 0x44, 0x20},
    {0x38, 0x44, 0x44, 0x48, 0x7F},
    {0x38, 0x54, 0x54, 0x54, 0x18},
    {0x08, 0x7E, 0x09, 0x01, 0x02},
    {0x0C, 0x52, 0x52, 0x52, 0x3E},
    {0x7F, 0x08, 0x04, 0x04, 0x78},
    {0x00, 0x44, 0x7D, 0x40, 0x00},
    {0x20, 0x40, 0x44, 0x3D, 0x00},
    {0x7F, 0x10, 0x28, 0x44, 0x00},
    {0x00, 0x41, 0x7F, 0x40, 0x00},
    {0x7C, 0x04, 0x18, 0x04, 0x78},
    {0x7C, 0x08, 0x04, 0x04, 0x78},
    {0x38, 0x44, 0x44, 0x44, 0x38},
    {0x7C, 0x14, 0x14, 0x14, 0x08},
    {0x08, 0x14, 0x14, 0x18, 0x7C},
    {0x7C, 0x08, 0x04, 0x04, 0x08},
    {0x48, 0x54, 0x54, 0x54, 0x20},
    {0x04, 0x3F, 0x44, 0x40, 0x20},
    {0x3C, 0x40, 0x40, 0x20, 0x7C},
    {0x1C, 0x20, 0x40, 0x20, 0x1C},
    {0x3C, 0x40, 0x20, 0x40, 0x3C},
    {0x44, 0x28, 0x10, 0x28, 0x44},
    {0x0C, 0x50, 0x50, 0x50, 0x3C},
    {0x44, 0x64, 0x54, 0x4C, 0x44},
    // { | } ~ cc
    {0x00, 0x08, 0x36, 0x41, 0x00},
    {0x00, 0x00, 0x7F, 0x00, 0x00},
    {0x00, 0x41, 0x36, 0x08, 0x00},
    {0x0C, 0x04, 0x1C, 0x10, 0x18},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // Custom assignments beyond ISO646
    // Codes 128+: right arrow, left arrow, degree sign
    {0x08, 0x08, 0x2A, 0x1C, 0x08},
    {0x08, 0x1C, 0x2A, 0x08, 0x08},
    {0x07, 0x05, 0x07, 0x00, 0x00},
};

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
	// Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable GPIO port B and E peripherals
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOB | SYSCTL_RCGC2_GPIOE;

    // Configure three backlight LEDs
    GPIO_PORTB_DIR_R |= 0x20;  // make bit5 an output
    GPIO_PORTB_DR2R_R |= 0x20; // set drive strength to 2mA
    GPIO_PORTB_DEN_R |= 0x20;  // enable bit5 for digital
    GPIO_PORTE_DIR_R |= 0x30;  // make bits 4 and 5 outputs
    GPIO_PORTE_DR2R_R |= 0x30; // set drive strength to 2mA
    GPIO_PORTE_DEN_R |= 0x30;  // enable bits 4 and 5 for digital

    // Configure A0 and ~CS for graphics LCD
    GPIO_PORTB_DIR_R |= 0x42;  // make bits 1 and 6 outputs
    GPIO_PORTB_DR2R_R |= 0x42; // set drive strength to 2mA
    GPIO_PORTB_DEN_R |= 0x42;  // enable bits 1 and 6 for digital

    // Configure SSI2 pins for SPI configuration
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R2;           // turn-on SSI2 clocking
    GPIO_PORTB_DIR_R |= 0x90;                        // make bits 4 and 7 outputs
    GPIO_PORTB_DR2R_R |= 0x90;                       // set drive strength to 2mA
	GPIO_PORTB_AFSEL_R |= 0x90;                      // select alternative functions for MOSI, SCLK pins
    GPIO_PORTB_PCTL_R = GPIO_PCTL_PB7_SSI2TX | GPIO_PCTL_PB4_SSI2CLK; // map alt fns to SSI2
    GPIO_PORTB_DEN_R |= 0x90;                        // enable digital operation on TX, CLK pins

    // Configure the SSI2 as a SPI master, mode 3, 8bit operation, 1 MHz bit rate
    SSI2_CR1_R &= ~SSI_CR1_SSE;                      // turn off SSI2 to allow re-configuration
    SSI2_CR1_R = 0;                                  // select master mode
    SSI2_CC_R = 0;                                   // select system clock as the clock source
    SSI2_CPSR_R = 40;                                // set bit rate to 1 MHz (if SR=0 in CR0)
    SSI2_CR0_R = SSI_CR0_SPH | SSI_CR0_SPO | SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8; // set SR=0, mode 3 (SPH=1, SPO=1), 8-bit
    SSI2_CR1_R |= SSI_CR1_SSE;                       // turn on SSI2
}

// Approximate busy waiting (in units of microseconds), given a 40 MHz system clock
void waitMicrosecond(uint32_t us)
{
	                                            // Approx clocks per us
	__asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*3
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             B    WMS_LOOP0");       // 1*3
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}

// Blocking function that writes data to the SPI bus and waits for the data to complete transmission
void sendGraphicsLcdCommand(uint8_t command)
{
	CS_NOT = 0;                        // assert chip select
	__asm (" NOP");                    // allow line to settle
	__asm (" NOP");
	__asm (" NOP");
	__asm (" NOP");
	A0 = 0;                            // clear A0 for commands
	SSI2_DR_R = command;               // write command
	while (SSI2_SR_R & SSI_SR_BSY);
	CS_NOT = 1;                        // de-assert chip select
}

// Blocking function that writes data to the SPI bus and waits for the data to complete transmission
void sendGraphicsLcdData(uint8_t data)
{
	CS_NOT = 0;                        // assert chip select
	__asm (" NOP");                    // allow line to settle
  	__asm (" NOP");
    __asm (" NOP");
	__asm (" NOP");
	A0 = 1;                            // set A0 for data
	SSI2_DR_R = data;                  // write data
	while (SSI2_SR_R & SSI_SR_BSY);    // wait for transmission to stop
	CS_NOT = 1;                        // de-assert chip select
}

void setGraphicsLcdPage(uint8_t page)
{
  sendGraphicsLcdCommand(0xB0 | page);
}

void setGraphicsLcdColumn(uint8_t x)
{
  sendGraphicsLcdCommand(0x10 | ((x >> 4) & 0x0F));
  sendGraphicsLcdCommand(0x00 | (x & 0x0F));
}

void refreshGraphicsLcd()
{
    uint8_t x, page;
    uint16_t i = 0;
    for (page = 0; page < 8; page ++)
    {
    	setGraphicsLcdPage(page);
        setGraphicsLcdColumn(0);
        for (x = 0; x < 128; x++)
    	    sendGraphicsLcdData(pixelMap[i++]);
    }
}

void clearGraphicsLcd()
{
    uint16_t i;
    // clear data memory pixel map
    for (i = 0; i < 1024; i++)
        pixelMap[i] = 0;
    // copy to display
    refreshGraphicsLcd();
}

void initGraphicsLcd()
{
    sendGraphicsLcdCommand(0x40); // set start line to 0
    sendGraphicsLcdCommand(0xA1); // reverse horizontal order
    sendGraphicsLcdCommand(0xC0); // normal vertical order
    sendGraphicsLcdCommand(0xA6); // normal pixel polarity
    sendGraphicsLcdCommand(0xA3); // set led bias to 1/9 (should be A2)
    sendGraphicsLcdCommand(0x2F); // turn on voltage booster and regulator
    sendGraphicsLcdCommand(0xF8); // set internal volt booster to 4x Vdd
    sendGraphicsLcdCommand(0x00);
    sendGraphicsLcdCommand(0x27); // set contrast
    sendGraphicsLcdCommand(0x81); // set LCD drive voltage
    sendGraphicsLcdCommand(0x04);
    sendGraphicsLcdCommand(0xAC); // no flashing indicator
    sendGraphicsLcdCommand(0x00);
    clearGraphicsLcd();           // clear display
    sendGraphicsLcdCommand(0xAF); // display on
}

void drawGraphicsLcdPixel(uint8_t x, uint8_t y, uint8_t op)
{
    uint8_t data, mask, page;
    uint16_t index;

    // determine pixel map entry
    page = y >> 3;

    // determine pixel map index
    index = page << 7 | x;

    // generate mask
    mask = 1 << (y & 7);

    // read pixel map
    data = pixelMap[index];

    // apply operator (0 = clear, 1 = set, 2 = xor)
    switch(op)
    {
        case 0: data &= ~mask; break;
        case 1: data |= mask; break;
        case 2: data ^= mask; break;
    }

    // write to pixel map
    pixelMap[index] = data;

    // write to display
    setGraphicsLcdPage(page);
    setGraphicsLcdColumn(x);
    sendGraphicsLcdData(data);
}

void drawGraphicsLcdRectangle(uint8_t xul, uint8_t yul, uint8_t dx, uint8_t dy, uint8_t op)
{
    uint8_t page, page_start, page_stop;
    uint8_t bit_index, bit_start, bit_stop;
    uint8_t mask, data;
    uint16_t index;
    uint8_t x;

    // determine pages for rectangle
    page_start = yul >> 3;
    page_stop = (yul + dy - 1) >> 3;

    // draw in pages from top to bottom within extent
    for (page = page_start; page <= page_stop; page++)
    {
        // calculate mask for this page
        if (page > page_start)
            bit_start = 0;
        else
            bit_start = yul & 7;
        if (page < page_stop)
            bit_stop = 7;
        else
            bit_stop = (yul + dy - 1) & 7;
        mask = 0;
        for (bit_index = bit_start; bit_index <= bit_stop; bit_index++)
            mask |= 1 << bit_index;

        // write page
        setGraphicsLcdPage(page);
        setGraphicsLcdColumn(xul);
        index = (page << 7) | xul;
        for (x = 0; x < dx; x++)
        {
            // read pixel map
            data = pixelMap[index];
            // apply operator (0 = clear, 1 = set, 2 = xor)
            switch(op)
            {
                case 0: data &= ~mask; break;
                case 1: data |= mask; break;
                case 2: data ^= mask; break;
            }
            // write to pixel map
            pixelMap[index++] = data;
            // write to display
            sendGraphicsLcdData(data);
        }
    }
}

void setGraphicsLcdTextPosition(uint8_t x, uint8_t page)
{
    txtIndex = (page << 7) + x;
    setGraphicsLcdPage(page);
    setGraphicsLcdColumn(x);
}

void putcGraphicsLcd(char c)
{
    uint8_t i, val;
    uint8_t uc;
    // convert to unsigned to access characters > 127
    uc = (uint8_t) c;
    for (i = 0; i < 5; i++)
    {
        val = charGen[uc-' '][i];
        pixelMap[txtIndex++] = val;
        sendGraphicsLcdData(val);
    }
    pixelMap[txtIndex++] = 0;
    sendGraphicsLcdData(0);
}

void putsGraphicsLcd(char str[])
{
    uint8_t i = 0;
    while (str[i] != 0)
        putcGraphicsLcd(str[i++]);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();

    // Turn-on all LEDs to create white backlight
    RED_BL_LED = 1;
    GREEN_BL_LED = 1;
    BLUE_BL_LED = 1;

    // Initialize graphics LCD
    initGraphicsLcd();

    // Draw X in left half of screen
    uint8_t i;
    for (i = 0; i < 64; i++)
        drawGraphicsLcdPixel(i, i, SET);
    for (i = 0; i < 64; i++)
        drawGraphicsLcdPixel(63-i, i, INVERT);

    // Draw text on screen
    setGraphicsLcdTextPosition(84, 5);
    putsGraphicsLcd("Text");

    // Draw flashing block around the text
    while(1)
    {
        drawGraphicsLcdRectangle(83, 39, 25, 9, INVERT);
        waitMicrosecond(500000);
    }
}
