/*

	Title:		Chaotic LFO
	Author:		Satoshi "Chuck" Takagi
	Date:		Nov., 16, 2005
	Software:	WinAvr
	Target:		ATmega88

*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

//#define TEMPO(N) (65536-50-1024+(N))
#define TEMPO(N) (65536-50-(N))
#define DACDATA PORTD
#define DACCNTL PORTB
#define DACCS0x PB6
#define DACCS1x PB7
#define DACWRx 0xFE
#define DACAxB PB1
#define DAC0A 0xBD	// 10111101
#define DAC0B 0xBF	// 10111111
#define DAC1A 0x7D	// 10111101
#define DAC1B 0x7F	// 10111111

#define MAXX 19.492022
#define MINX -19.412596
#define MAXY 26.908178
#define MINY -26.754900
#define MAXZ 47.866940
#define MINZ 0.000000


typedef uint8_t bool;
#define true 1
#define false 0

volatile bool isNew = false;
volatile uint8_t gix, giy, giz, giw = 0;

volatile uint16_t gTempo = 977;
volatile uint16_t gADC;


SIGNAL (SIG_OVERFLOW1)
{
	TCNT1 = TEMPO(gTempo);

	if(!isNew)
		return;

	DACCNTL = DAC0A;
	DACDATA = gix;
	DACCNTL &= DACWRx;
	DACCNTL |= ~DACWRx;

	DACCNTL = DAC0B;
	DACDATA = giy;
	DACCNTL &= DACWRx;
	DACCNTL |= ~DACWRx;

	DACCNTL = DAC1A;
	DACDATA = giz;
	DACCNTL &= DACWRx;
	DACCNTL |= ~DACWRx;

	DACCNTL = DAC1B;
	DACDATA = giw++;
	DACCNTL &= DACWRx;
	DACCNTL |= ~DACWRx;
/*
	if(giw)	DACCNTL &= DACWRx;
	else	DACCNTL |= ~DACWRx;
	DACDATA = ~giw++;
*/
	isNew = false;
}

SIGNAL (SIG_ADC)
{
	if ((ADMUX & 0x0F)==0)
		gADC = ADC;
}

void init_io(void)
{
	DDRD = 0xFF;		// Port D output
	PORTD = 0;

	DDRB = 0xFF;		// Port B output
	PORTB = 0xFF;

	/* timer1 initialization */			// timer1 for Tempo Control
	TCNT1 = TEMPO(gTempo);
	TCCR1B = (1<<CS12)|(1<<CS10);		// timer1 prescaled by 1/1024
	TIMSK1 = (1<<TOIE1);				// timer1 interrupt enabled

	/* ADC initial state*/
	ADMUX = 0;
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	DIDR0 = (1<<ADC0D);

}

int main(void)
{
	float x = 0.0, y = 0.01, z = 0.0;
	float dx, dy, dz, dt = 1.0e-2;
	float tx, ty, tz;
	uint16_t lastADC;

	init_io();
	sei();				/* enable interrupts     */

	for (;;) {
		if(!(ADCSRA & (1<<ADSC)))
			ADCSRA |= (1<<ADSC);
		if(gADC != lastADC) {
			lastADC = gADC;
			gTempo = lastADC;
		}

		if (!isNew) {
			dx = (-10.0 * x + 10.0 * y) * dt;
			dy = (28.0 * x - y - x * z) * dt;
			dz = (-8.0 / 3.0 * z + x * y) * dt;
	
			x += dx;
			y += dy;
			z += dz;

			tx = (x - MINX) / (MAXX - MINX) * 255.;
			ty = (y - MINY) / (MAXY - MINY) * 255.;
			tz = (z - MINZ) / (MAXZ - MINZ) * 255.;
			if (tx < 0)		tx = 0;
			if (tx > 255)	tx = 255;
			if (ty < 0)		ty = 0;
			if (ty > 255)	ty = 255;
			if (tz < 0)		tz = 0;
			if (tz > 255)	tz = 255;
			gix = (int8_t)tx;
			giy = (int8_t)ty;
			giz = (int8_t)tz;
			
			isNew = true;
		}
	}
}
