//******************************************************************************
// Police light timer for MSP430 launchpad
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
//          --|RST          P1.6|----> LED
//            |                 |
//
//******************************************************************************

// include section
//#include <msp430g2553.h>
#include <msp430g2231.h>
//#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

#define LED_INIT() do{P1DIR|=0x40;P1OUT&=~0x40;}while(0)
#define LED_ON() do{P1OUT|=0x40;}while(0)
#define LED_OFF() do{P1OUT&=~0x40;}while(0)

#define FLASHES 6
#define FLASH_ON_TIME 7
#define FLASH_OFF_TIME 7
#define PAUSE 40

#define TICK_TIME 10000

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
    TACTL = TASSEL_2 + MC_2; // SMCLK, contmode
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	init();

	while(1)
	{
  	    LED_ON();
		SLEEP(FLASH_ON_TIME);
		
		int i;
		for (i=1;i<FLASHES;i++) {
			LED_OFF();
			SLEEP(FLASH_OFF_TIME);
		
			LED_ON();
			SLEEP(FLASH_ON_TIME);
		}
		
		LED_OFF();
        SLEEP(PAUSE);
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
