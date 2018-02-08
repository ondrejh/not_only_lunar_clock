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
//          | |             XTAL|<---> 32.768kHz quartz (realtime clock)
//          --|RST              |
//            |                 |
//            |             P1.6|----> COIL
//            |             P1.7|----> COIL
//            |                 |
//            |             P1.3|-------  POTENTIOMETER (10k)
//            |                 |      \/
//            |             P1.4|----[XXXX]----
//            |             P1.5|--------------
//            |                 |
//
//******************************************************************************

// include section
//#include <msp430g2553.h>
#include <msp430g2231.h>
//#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

#define COIL_INIT() do{P1DIR|=0xC0;P1OUT&=~0xC0;}while(0)
#define COIL_A_ON() do{P1OUT|=0x40;}while(0)
#define COIL_B_ON() do{P1OUT|=0x80;}while(0)
#define COIL_OFF() do{P1OUT&=~0xC0;}while(0)

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

void analog_init(void)
{
    P1DIR |= 0x30; P1OUT&=~0x30;
    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
    ADC10CTL1 = INCH_3;                         // input A3
    ADC10AE0 |= 0x08;                           // PA.3 ADC option select
}

// leds and dco init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

	COIL_INIT();
}

volatile uint16_t ticks = TICKS_AVG;

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); // init dco and leds
	rtc_timer_init(); // init 32kHz clock timer
    analog_init();

	while(1)
	{
        __bis_SR_register(CPUOFF + GIE); // enter sleep mode
        // switch on potentiometer
        P1OUT |= 0x20;
        __delay_cycles(200);
        // start conversion
        ADC10CTL0 |= ENC + ADC10SC;
        __bis_SR_register(CPUOFF + GIE);
        // get value
        ticks = TICKS_MIN + (ADC10MEM>>3);
        // switch it off
        P1OUT &= ~0x20;
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
        //LED_GREEN_ON();
        A = !A;
        coil_on = 2;
        next += ticks;
    }
    
    // coil zeroing
	else {
        if (coil_on) {
            //LED_GREEN_OFF();
            coil_on--;
            if (coil_on==0)
                COIL_OFF();
        }   
	}

    if (cnt==0)
        __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)

	cnt++;
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}
