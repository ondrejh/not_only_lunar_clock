//******************************************************************************
// LiPo voltage guard using MSP430 launchpad
//
//
// author: Ondrej Hejda
// date:   8.2.2018
//
// hardware: MSP430G2231 (launchpad)
//
//                MSP430G2231
//             -----------------
//         /|\|                 |
//          | |             P1.0|----> RED LED
//          --|RST          P1.6|----> GREEN LED
//            |                 |
//            |             P1.4|----> BEEP +
//            |             P1.5|----> BEEP -
//            |                 |
//
//******************************************************************************

// include section
//#include <msp430g2553.h>
#include <msp430g2231.h>
//#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

#define GREEN_ON() do{P1OUT|=0x40;}while(0)
#define GREEN_OFF() do{P1OUT&=~0x40;}while(0)
#define RED_ON() do{P1OUT|=0x01;}while(0)
#define RED_OFF() do{P1OUT&=~0x01;}while(0)

#define BEEP_OFF() do{P1OUT&=~0x30;}while(0)
#define BEEP_P() do{BEEP_OFF();P1OUT|=0x10;}while(0)
#define BEEP_N() do{BEEP_OFF();P1OUT|=0x20;}while(0);

#define BEEP_PERIOD 133

// leds and dco init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

    // led and motor outputs
	P1DIR |= 0x71; P1OUT &= 0x00;

    // start timer
    TACTL = TASSEL_2 + MC_2; // SMCLK, continuous
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); // init dco and leds

    uint16_t last = 0;
    while (1) {
        uint16_t now = TAR;
        if ((now-last)>=BEEP_PERIOD) {
            static int cnt;
            cnt ++;
            cnt &= 3;
            switch (cnt) {
            case 0:
                BEEP_OFF();
                break;
            case 1:
                BEEP_P();
                break;
            case 2:
                BEEP_OFF();
                break;
            case 3:
                BEEP_N();
                break;
            }
            last += BEEP_PERIOD;
            P1OUT ^= 0x10;
        }
    }

	return -1;
}
