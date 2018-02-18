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

#define RED_OFF() do{P1OUT|=0x20;}while(0)
#define RED_ON() do{P1OUT&=~0x20;}while(0)
#define RED_SWAP() do{P1OUT^=0x20;}while(0)

#define GREEN_OFF() do{P1OUT|=0x40;}while(0)
#define GREEN_ON() do{P1OUT&=~0x40;}while(0)
#define GREEN_SWAP() do{P1OUT^=0x40;}while(0)

#define IN1_HIGH() do{P1OUT|=0x10;}while(0)
#define IN1_LOW() do{P1OUT&=~0x10;}while(0)
#define IN2_HIGH() do{P1OUT|=0x08;}while(0)
#define IN2_LOW() do{P1OUT&=~0x08;}while(0)

#define CH2_M (1<<0)

#define THOLD 80
#define CENTER 1500
#define MAX 2200
#define MIN 800

#define PWM_MAX 1023

#define DIV_HIGH 7.5
#define DIV_LOW 4.7
#define ADC_REF 3.3
#define ADC_MAX 1023
#define ADC_SAMPLES 8

#define ADC_LEVEL(x) (uint16_t)(x/(DIV_HIGH+DIV_LOW)*DIV_LOW/ADC_REF*ADC_MAX*ADC_SAMPLES)

#define ADC_CHARGED ADC_LEVEL(8.0)
#define ADC_OK ADC_LEVEL(7.6)
#define ADC_CRITICAL ADC_LEVEL(6.6)

#define BLINK_SLOW 50000
#define BLINK_FAST 20000

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

    // init ADC
    ADC10CTL0 = ADC10SHT_2 + ADC10ON; // ADC10ON
    ADC10CTL1 = INCH_1; // input A1
    ADC10AE0 |= 0x02; // PA.1 ADC option select

    // leds on
    GREEN_ON();
    RED_ON();

    // switch motor off
    pwm = PWM_MAX;
    IN2_LOW();
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

// adc input
uint8_t use_value(uint16_t val)
{
    if (val<ADC_CRITICAL)
        return 0;
    if (val<ADC_OK)
        return 1;
    if (val<ADC_CHARGED)
        return 2;
    return 3;
}

bool blink_div(void) {
    static uint8_t div = 0;
    div ++;
    div &= 0x7;
    return (div==0);
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); // init dco and leds

    uint8_t in_last = 0;
    uint16_t ch2_start = P1IN;

    uint8_t adc_state = 0;
    uint16_t adc_value = 0;
    uint8_t adc_counter = 0;
    uint16_t adc_timer = 0;

    uint8_t level = 1;

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
            if (level>0) 
                ch2(now-ch2_start);

        // switch off when battery is low
        if (level==0) {
            pwm = PWM_MAX;
            IN2_LOW();
        }

        // do some pwm
        if (pwm<(now&PWM_MAX))
            IN1_HIGH();
        else
            IN1_LOW();

        // adc reading
        switch (adc_state) {
        case 0:
            if ((now-adc_timer) > 50000) {
                ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
                adc_timer = now;
                adc_state ++;
            }
            break;
        case 1:
            if (!(ADC10CTL1 & ADC10BUSY)) {
                adc_value += ADC10MEM;
                adc_counter ++;
                if (adc_counter >= ADC_SAMPLES)
                    adc_state++;
                else
                    adc_state=0;
            }
            break;
        case 2:
            level = use_value(adc_value);
            adc_counter = 0;
            adc_value = 0;
            adc_state = 0;
            // blinking
            switch (level) {
            case 0:
                GREEN_OFF();
                RED_SWAP();
                break;
            case 1:
                GREEN_OFF();
                RED_ON();
                break;
            case 2:
                GREEN_ON();
                RED_OFF();
                break;
            case 3:
                GREEN_SWAP();
                RED_OFF();
                break;
            }
            break;
        }

        // save input for next round
        in_last = in_now;
    }

	return -1;
}
