//******************************************************************************
// Semaphor light timer for MSP430 launchpad
//
//
// author: Ondrej Hejda
// date:   25.12.2017
//
// hardware: MSP430G2231 (launchpad)
//
//                MSP4302231
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST          P1.5|---|<---------| RED
//            |             P1.6|--------------|
//            |             P1.7|---o-----|<---o YELLOW
//            |                 |    \---->|--/  GREEN  
//            |                 |
//
//******************************************************************************

// include section
//#include <msp430g2553.h>
#include <msp430g2231.h>
//#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

#define LED_INIT() do{P1DIR|=0xE0;P1OUT&=~0xE0;}while(0)
#define OFF() do{P1OUT&=~0xE0;}while(0)
#define RED() do{OFF();P1OUT|=0xC0;}while(0)
#define YELLOW() do{OFF();P1OUT|=0x60;}while(0)
#define GREEN() do{OFF();P1OUT|=0x80;}while(0)
#define RED_YELLOW() do{OFF();P1OUT|=0x40;}while(0)

#define YELLOW_PAUSE 4
#define PAUSE 20

#define TICK_TIME 62500

#define SLEEP(x) do{sleep_time=x;__bis_SR_register(CPUOFF + GIE);}while(0)
#define WAKE() do{__bic_SR_register_on_exit(CPUOFF);}while(0)

volatile uint16_t sleep_time = 0;

// leds and dco init
void init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

    // led output
	LED_INIT();

    // timer    
    CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = TICK_TIME;
    TACTL = TASSEL_2 + MC_2 + ID_3; // SMCLK, contmode
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	init();

	while(1)
	{
  	    RED();
		SLEEP(PAUSE);
        RED_YELLOW();
        SLEEP(YELLOW_PAUSE);
        GREEN();
        SLEEP(PAUSE);
        YELLOW();
        SLEEP(YELLOW_PAUSE);
	}

	return -1;
}

/** interrupt section **/

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	CCR0 += TICK_TIME; // Add Offset to CCR0
	if (sleep_time>0)
		sleep_time--;
	if (sleep_time==0)
		WAKE();
}
