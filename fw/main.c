//******************************************************************************
// RC signal DC motor drive with voltage monitoring for MSP430 launchpad
//
//
// author: Ondrej Hejda
// date:   18.2.2018
//
// hardware: MSP430G2231 (launchpad)
//
//                MSP430G2231
//             -----------------
//         /|\|                 |
//          | |             P1.6|----> GREEN LED
//          --|RST          P1.5|----> RED LED
//            |                 |
//            |             P1.4|----> MOTOR IN1
//            |             P1.3|----> MOTOR IN2
//            |                 |
//            |             P1.1|<---- ADC (input voltage)
//            |             P1.0|<---- CH2 (throtle)
//            |                 |
//
//******************************************************************************

// include section
//#include <msp430g2553.h>
#include <msp430g2231.h>
//#include <msp430g2452.h>

#include <stdint.h>
#include <stdbool.h>

#define RED_LED_OFF() do{P1OUT|=0x20;}while(0)
#define RED_LED_ON() do{P1OUT&=~0x20;}while(0)

#define GREEN_LED_OFF() do{P1OUT|=0x40;}while(0)
#define GREEN_LED_ON() do{P1OUT&=~0x40;}while(0)

#define IN1_HIGH() do{P1OUT|=0x10;}while(0)
#define IN1_LOW() do{P1OUT&=~0x10;}while(0)
#define IN2_HIGH() do{P1OUT|=0x08;}while(0)
#define IN2_LOW() do{P1OUT&=~0x08;}while(0)

#define CH2_M (1<<0)

#define THOLD 70
#define CENTER 1500
#define MAX 2200
#define MIN 800

#define PWM_MAX 1023

uint16_t pwm;

// leds and dco init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

    // led and motor outputs
	P1DIR |= 0x78; P1OUT &= ~0x78;
    // servo signal input
    P1DIR &= ~0x01;

    // start timer
    TACTL = TASSEL_2 + MC_2; // SMCLK, continuous

    GREEN_LED_ON();
    RED_LED_ON();
}

// channel 2
void ch2(uint16_t v)
{
    if ((v<MIN) | (v>MAX)) return;

    uint16_t p;

    if (v < (CENTER - THOLD)) {
        p = CENTER - v;
        p <<= 1;
        pwm = p;
        IN2_HIGH();
    }
    else if (v > (CENTER + THOLD)) {
        p = v - CENTER;
        p <<= 1;
        pwm = PWM_MAX - p;
        IN2_LOW();
    }
    else {
        pwm = PWM_MAX;
        IN2_LOW();
    }
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); // init dco and leds

    while (1) {
        // wait servo input is low
        while (P1IN&(CH2_M)) {};

        RED_LED_OFF();
        GREEN_LED_ON();

        uint8_t in_last = 0;

        uint16_t ch2_start = P1IN;

	    while(1) {
            uint16_t now = TAR;
            uint8_t in_now = P1IN;
            
            uint8_t in_changes = in_now^in_last;
            uint8_t in_goes_up = in_changes&in_now;
            uint8_t in_goes_down = in_changes&in_last;
            
            // get channel 2 value
            if (in_goes_up & CH2_M)
                ch2_start = now;
            else if (in_goes_down & CH2_M)
                ch2(now-ch2_start);

            // do some pwm
            if (pwm<(now&PWM_MAX))
                IN1_HIGH();
            else
                IN1_LOW();

            // save input for next round
            in_last = in_now;

            // if no servo signal switch off
            if ((now-ch2_start) > 50000)
                break;
	    }

        GREEN_LED_OFF();

        IN1_LOW();
        IN2_LOW();

        ch2_start = TAR;

        while (!(P1IN&CH2_M)) {
        }
    }

	return -1;
}
