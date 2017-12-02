//******************************************************************************
// Pomodoro timer for MSP430 launchpad
//
//
// author:          Ondrej Hejda
// date (started):  28.2.2013
//
// hardware: MSP430G2553 (launchpad)
//
//                MSP4302553
//             -----------------
//         /|\|                 |
//          | |             XTAL|<---> 32.768kHz quartz (realtime clock)
//          --|RST              |
//            |                 |
//            |             P1.4|----> COIL
//            |             P1.5|----> COIL
//            |                 |
//
//******************************************************************************

// include section
#include <msp430g2553.h>
//#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

// board (leds)
#define LED_INIT() {P1DIR|=0x41;P1OUT&=~0x41;}
#define LED_RED_ON() {P1OUT|=0x01;}
#define LED_RED_OFF() {P1OUT&=~0x01;}
#define LED_RED_SWAP() {P1OUT^=0x01;}
#define LED_GREEN_ON() {P1OUT|=0x40;}
#define LED_GREEN_OFF() {P1OUT&=~0x40;}
#define LED_GREEN_SWAP() {P1OUT^=0x40;}

#define COIL_INIT() do{P1DIR|=0x30;P1OUT&=~0x30;}while(0)
#define COIL_A_ON() do{P1OUT|=0x10;}while(0)
#define COIL_B_ON() do{P1OUT|=0x20;}while(0)
#define COIL_OFF() do{P1OUT&=~0x30;}while(0)

#define TICKS_MIN 384
#define TICKS_AVG 448
#define TICKS_MAX 512

void rtc_timer_init(void)
{
	CCTL0 = CCIE; // CCR0 interrupt enabled
	CCR0 = 31;	  // f = 32768 / 8(ID_3) / 32(CCR0+1) = 128Hz
	//CCR0 = 63;	  // f = 32768 / 8(ID_3) / 64(CCR0+1) = 64Hz
	//CCR0 = 127;	  // f = 32768 / 8(ID_3) / 128(CCR0+1) = 32Hz
	//CCR0 = 511;	  // f = 32768 / 8(ID_3) / 512(CCR0+1) = 8Hz
	//CCR0 = 1023;	  // f = 32768 / 8(ID_3) / 1024(CCR0+1) = 4Hz
	//CCR0 = 2047;	  // f = 32768 / 8(ID_3) / 2048(CCR0+1) = 2Hz
	TACTL = TASSEL_1 + ID_3 + MC_1; // ACLK, /8, upmode
}

// leds and dco init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

	LED_INIT();
	COIL_INIT();
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); // init dco and leds
	rtc_timer_init(); // init 32kHz clock timer

	while(1)
	{
        __bis_SR_register(CPUOFF + GIE); // enter sleep mode
	}

	return -1;
}

/** interrupt section **/

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    static uint16_t cnt = 0;
    static uint16_t coil_on = 0;

    /*// standard clock (16Hz tick)
	if ((cnt&0x07)==0) {
		if (cnt&0x08)
			COIL_A_ON();
		else
			COIL_B_ON();
        coil_on = 2;
	}*/
    
    // lunar clock
    static uint16_t next = TICKS_AVG;
    if (cnt==next) {
        static bool A = true;
        if (A)
            COIL_A_ON();
        else
            COIL_B_ON();
        A = !A;
        coil_on = 2;
        next+=TICKS_AVG;
    }
    
    // coil zeroing
	else {
        if (coil_on) {
            coil_on--;
            if (coil_on==0)
                COIL_OFF();
        }   
	}

	cnt++;
}
