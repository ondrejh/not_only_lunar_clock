//******************************************************************************
// Lunar clock timer for MSP430 launchpad
//
//
// author: Ondrej Hejda
// date:   19.12.2017
//
// hardware: MSP430G2231 (launchpad)
//
//                MSP4302231
//             -----------------
//         /|\|                 |
//          | |             P1.0|----> MOTOR BW
//          --|RST          P1.1|----> MOTOR FW
//            |                 |
//            |             P1.2|----> LED_L
//            |             P1.3|----> LED_R
//            |                 |
//            |             P1.4|<---- CH2 (throtle)
//            |             P1.5|<---- CH3 (light)
//            |                 |
//
//******************************************************************************

// include section
//#include <msp430g2553.h>
#include <msp430g2231.h>
//#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

#define MOTOR_FW() do{P1OUT=(P1OUT&0xFC)|0x02;}while(0)
#define MOTOR_BW() do{P1OUT=(P1OUT&0xFC)|0x01;}while(0)
#define MOTOR_STOP() do{P1OUT&=0xFC;}while(0)
#define LED_ON() do{P1OUT|=0x04;}while(0)
#define LED_OFF() do{P1OUT&=0xF3;}while(0)

// leds and dco init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

    // led and motor outputs
	P1DIR |= 0x0F; P1OUT &= 0xF0;
    // servo signal input
    P1DIR &= 0xCF;

    LED_ON();
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); // init dco and leds

	while(1)
	{
        __bis_SR_register(CPUOFF + GIE); // enter sleep mode
	}

	return -1;
}
