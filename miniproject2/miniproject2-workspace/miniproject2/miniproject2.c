/*
 * miniproject2.c
 *
 *  Created on: Sep 16, 2021
 *  Author: Toka Mohamed
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include<util/delay.h>

unsigned char flag1 = 0;
unsigned char arr[6] = { 0, 1, 2, 3, 4, 5 };   //6 seven-segment displays
unsigned char count[6] = { 0, 0, 0, 0, 0, 0 }; //counts of each seven-segment display

ISR(TIMER1_COMPA_vect) { //when compare match occurs the stop watch increments by 1 second
	flag1 = 1;
}

ISR(INT0_vect)         //reset the stop watch if push button(PD2) is pressed
{
	for (unsigned char i = 0; i < 6; i++) {
		count[i] = 0;
	}
}

ISR(INT1_vect)         //pause the stop watch if push button(PD3) is pressed
{
	TCCR1B &= ~(1 << CS12) & ~(1 << CS10);   //no clock source
}

ISR(INT2_vect)        //resume the stop watch if push button(PB2) is pressed
{
	TCCR1B |= (1 << CS12) | (1 << CS10); //return the clock source with prescaler Fcpu/1024
}

void INT0_Init(void) {
	SREG &= ~(1 << 7);                   // Disable interrupts by clearing I-bit
	DDRD &= (~(1 << PD2));               // Configure INT0/PD2 as input pin
	PORTD |= (1 << PD2);                  //enable internal pull up resistor
	GICR |= (1 << INT0);                 // Enable external interrupt pin INT0
	MCUCR |= (1 << ISC01);                // Trigger INT0 with the falling edge
	SREG |= (1 << 7);                    // Enable interrupts by setting I-bit
}

void INT1_Init(void) {
	SREG &= ~(1 << 7);                   // Disable interrupts by clearing I-bit
	DDRD &= (~(1 << PD3));               // Configure INT1/PD3 as input pin
	GICR |= (1 << INT1);                 // Enable external interrupt pin INT1
	MCUCR |= (1 << ISC11) | (1 << ISC10);   //Trigger INT1 with the rising edge
	SREG |= (1 << 7);                    // Enable interrupts by setting I-bit
}

void INT2_Init(void) {
	SREG &= ~(1 << 7);                  // Disable interrupts by clearing I-bit
	DDRB &= (~(1 << PB2));              // Configure INT2/PB2 as input pin
	PORTB |= (1 << PB2);                   //enable internal pull up resistor
	GICR |= (1 << INT2);	            // Enable external interrupt pin INT2
	MCUCSR &= ~(1 << ISC2);               // Trigger INT2 with the falling edge
	SREG |= (1 << 7);                   // Enable interrupts by setting I-bit
}

void Timer1_CTC_Init(void) {
	TCNT1 = 0;                          // Set timer1 initial count to zero

	OCR1A = 1000;                     //Set the Compare value to 1000 (1 second)

	TIMSK |= (1 << OCIE1A);             //Enable Timer1 Compare A Interrupt

	TCCR1A = (1 << FOC1A);              //non pwm

	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); //CTC Mode and prescaler Fcpu/1024
}

int main(void) {
	DDRA |= 0X3F;  //the first 6 pins in portA are outputs
	PORTA |= 0X3F; //initially enable all the 7-segment displays
	DDRC |= 0X0F;  //the first 4 pins in portC are outputs
	PORTC &= 0XF0; //initially display 0

	SREG |= (1 << 7);      // Enable interrupts by setting I-bit
	INT0_Init();
	INT1_Init();
	INT2_Init();
	Timer1_CTC_Init();

	while (1) {
		if (flag1 == 1) {
			flag1 = 0; //reset the flag so when the next interrupt occurs it becomes 1 again

			if (count[0] == 9) {
				count[0] = 0;
				if (count[1] == 5) {
					count[1] = 0;
					if (count[2] == 9) {
						count[2] = 0;
						if (count[3] == 5) {
							count[3] = 0;
							if (count[4] == 9) {
								count[4] = 0;
								if (count[5] == 9) {
									count[5] = 0;
								} else
									count[5]++;
							} else
								count[4]++;
						} else
							count[3]++;
					} else
						count[2]++;
				} else
					count[1]++;
			} else
				count[0]++;

			while (1) {
				for (unsigned char i = 0; i < 6; i++) { /*at each interrupt loop on the 6 7-segments to display the counts with
				 delay 100us between them to appear as if all of them are enabled at the
				 same time */
					PORTA &= ~0X3F;
					PORTA |= (1 << arr[i]);
					PORTC = (PORTC & 0xF0) | (count[i] & 0x0F);
					_delay_us(100);
				}

				if (flag1 == 1) //at a new interrupt leave while(1) and increment the counter then enter while(1) again to display the new counts
					break;
			}
		}

	}
}
