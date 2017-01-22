// Stop Go C/ASM Mix Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// Pushbutton:
//   SW1 pulls pin PF4 low (internal pull-up is used)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))
#define PUSH_BUTTON  (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4)))

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Blocking function that returns only when SW1 is pressed
extern void waitPbPress();

// Initialize Hardware
void init_hw()
{
	// Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable GPIO port F peripherals
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

    // Configure LED and pushbutton pins
    GPIO_PORTF_DIR_R = 0x0A;  // bits 1 and 3 are outputs, other pins are inputs
    GPIO_PORTF_DR2R_R = 0x0A; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R = 0x1A;  // enable LEDs and pushbuttons
    GPIO_PORTF_PUR_R = 0x10;  // enable internal pull-up for push button
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
	// Initialize hardware
	init_hw();

    // Turn on red LED, turn off green LED
    RED_LED = 1;
    GREEN_LED = 0;

    // Wait for PB press
    waitPbPress();

    // Turn off red LED
    RED_LED = 0;

    // Turn on green LED
    __asm("            PUSH {R0, R1}");
    __asm("            B    next");
    __asm("GREEN_LED:  .field 0x424A7F8C");
    __asm("next:       LDR R0, GREEN_LED");
    __asm("            MOV R1, #1");
    __asm("            STR R1, [R0]");
    __asm("            POP {R0, R1}");

    // Endless loop
    while(1);
}
