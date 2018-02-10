//******************************************************************************
// RC signal DC motor drive for MSP430 launchpad
//
//
// author: Ondrej Hejda
// date:   6.1.2018
//
// hardware: MSP430G2452 (launchpad)
//
//                MSP430G2452
//             -----------------
//         /|\|                 |
//          | |             P1.0|----> RED LED (not used)
//          --|RST              |
//            |             P1.3|<---- BUTTON
//            |                 |
//            |             P1.5|----> SERVO
//
//******************************************************************************

// include section
//#include <msp430g2553.h>
//#include <msp430g2231.h>
#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

#define LED_ON() do{P1OUT|=0x01;}while(0)
#define LED_OFF() do{P1OUT&=0xFE;}while(0)
#define LED_SWAP() do{P1OUT^=0x01;}while(0)

#define CENTER 1500
#define MAX 2000
#define MIN 1000
#define STEP 20

// leds and dco init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

    // led and motor outputs
	P1DIR |= 0x21; P1OUT &= ~0x21;

    // start timer
    CCTL0 = CCIE;                             // CCR0 interrupt enabled
    CCR0 = 1000;
    TACTL = TASSEL_2 + MC_2;                  // SMCLK, contmode

    P1REN |= 0x08;
    P1DIR &= ~0x08;

    LED_OFF();
}

int o = CENTER;
int lo = CENTER;

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); // init dco and leds

    while (1) {
        __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
    }

	return -1;
}

void do_change(void)
{
    static int s = 0;
    switch (s) {
    case 0:
        o = MAX;
        s++;
        break;
    case 1:
        o = CENTER;
        s++;
        break;
    case 2:
        o = MIN;
        s++;
        break;
    case 3:
        o = CENTER;
        s=0;
        break;
    default:
        s=0;
        break;
    }
}

// Timer A0 interrupt service routine
//void __attribute__ ((interrupt(TIMERA0_VECTOR))) Timer_A (void)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
{
    static int s = 0;

    switch(s) {
    case 0:
        P1OUT |= 0x20;
        if (o>lo)
            lo += STEP;
        else if (o<lo)
            lo -= STEP;
        CCR0 += lo;
        s++;
        break;
    case 1:
        P1OUT &= ~0x20;
        CCR0 += 20000-lo;
        s = 0;
        break;
    default:
        s = 0;
        break;
    }

    bool p = ((P1IN&0x08)!=0);
    static int pc = 0;
    static int ps = 0;

    if (s==0) {
        switch(ps) {
        case 0:
            if (p)
                pc = 0;
            else
                pc ++;
            if (pc>5) {
                ps ++;
                pc = 0;
                do_change();
            }
            break;
        case 1:
            if (p)
                pc ++;
            else
                p = 0;
            if (pc>5) {
                ps = 0;
                pc = 0;
            }
            break;
        default:
            pc = 0;
            ps = 0;
            break;
        }
    }
}
