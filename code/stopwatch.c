/*
 * stopwatch.c
 *
 *  Created on: Sep 11, 2024
 *      Author: Aziza Abdul Rahman Zamel
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define COUNT_UP   0
#define COUNT_DOWN 1
#define HOURS_DEC_BT PB0
#define HOURS_INC_BT PB1
#define MIN_DEC_BT PB3
#define MIN_INC_BT PB4
#define SEC_DEC_BT PB5
#define SEC_INC_BT PB6
#define TOGGLE_BT PB7
#define RED PD4
#define YELLOW PD5
#define BUZZER PD0
#define STOP_BUZZER PA6

unsigned char seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 0;
unsigned char timer1_flag = 0;
unsigned char stop_buzzer_flag = 0;

/**********************         function prototypes         **********************/

void TIMER1_CTC_MODE_INIT(void);
void INT0_INIT(void);
void INT1_INIT(void);
void INT2_INIT(void);

void printFirstDigit(char num);
void printSecondDigit(char num);

void increment_seconds(void);
void decrement_seconds(void);
void increment_minutes(void);
void decrement_minutes(void);

/***********************************************************************************/

int main(void) {

	unsigned char count_mode = COUNT_UP;
	unsigned char buzzer_on_flag = 0;
	// buttons flags
	unsigned char hours_inc_flag = 1, hours_dec_flag = 1, min_inc_flag = 1,
			min_dec_flag = 1, sec_inc_flag = 1, sec_dec_flag = 1;
	unsigned char toggle_flag = 1;

	/********** 7-segment Initialization **********/
	// configure first four pins of PORTC as output pins
	DDRC |= 0x0F;
	// configure first six pins of PORTA as output pins to enable/disable the six 7-segments.
	DDRA |= 0x3F;

	/********** Global Interrupt enable **********/
	SREG |= (1 << 7);

	/********** Timer 1 Initialization **********/
	TIMER1_CTC_MODE_INIT();

	/********** External interrupts initialization **********/
	// Enable external INT0
	INT0_INIT();

	// Enable external INT1
	INT1_INIT();

	// Enable external INT2
	INT2_INIT();

	/********** switches initialization **********/
	// Configure PB0, PB1 ,PB3 ,PB4 ,PB5 ,PB6 ,PB7  as input pin
	DDRB &= 0x04;
	// Activate the internal pull up resistor at PB0, PB1 ,PB3 ,PB4 ,PB5 ,PB6 ,PB7
	PORTB |= ~0x04;

	/********** LEDs initialization **********/
	// Configure PD4 and PD5 as output pins
	DDRD |= (1 << RED) | (1 << YELLOW);
	// initially RED LED is ON because the count up mode is the default
	PORTD |= (1 << RED);
	PORTD &= ~(1 << YELLOW);

	/********** Buzzer initialization **********/
	// Configure PD0 as output pin
	DDRD |= (1 << BUZZER);
	// turn off buzzer
	PORTD &= ~(1 << BUZZER);
	// Configure PA6 as output pin
	DDRA &= ~(1 << STOP_BUZZER);
	// Activate the internal pull up resistor at PA6
	PORTA |= (1 << STOP_BUZZER);

	for (;;) {

		/********************* Print Digits *********************/

		/****** print seconds ******/

		//first digit
		PORTA = (PORTA & (~0x3F)) | (1 << PA5);   //enable 1st 7-segment
		printFirstDigit(seconds);

		_delay_ms(2);

		//second digit
		PORTA = (PORTA & (~0x3F)) | (1 << PA4);   //enable 2nd 7-segment
		printSecondDigit(seconds);

		_delay_ms(2);

		/****** print minutes ******/

		//first digit
		PORTA = (PORTA & (~0x3F)) | (1 << PA3);    //enable 3rd 7-segment
		printFirstDigit(minutes);

		_delay_ms(2);

		//second digit
		PORTA = (PORTA & (~0x3F)) | (1 << PA2);   //enable 4th 7-segment
		printSecondDigit(minutes);

		_delay_ms(2);

		/****** print hours ******/

		//first digit
		PORTA = (PORTA & (~0x3F)) | (1 << PA1);    //enable 5th 7-segment
		printFirstDigit(hours);

		_delay_ms(2);

		//second digit
		PORTA = (PORTA & (~0x3F)) | (1 << PA0);    //enable 6th 7-segment
		printSecondDigit(hours);

		_delay_ms(2);

		// check timer1 interrupt flag
		if (timer1_flag) {
			// check if the count mode is COUNT UP or COUNT DOWN mode
			if (count_mode == COUNT_UP) {
				increment_seconds();
			} else {
				decrement_seconds();
				if ((!seconds) && (!minutes) && (!hours)
						&& (!stop_buzzer_flag)) {
					buzzer_on_flag = 1;
				} else {
					buzzer_on_flag = 0;
				}
			}
			// reset flag value to 0 to not enter here until the interrupt occurs again
			timer1_flag = 0;
		}

		// check if the toggle mode button pressed
		if (!(PINB & (1 << TOGGLE_BT))) {
			stop_buzzer_flag = 0;
			if (toggle_flag) {
				// toggle mode
				count_mode ^= 1;
				if (count_mode == COUNT_DOWN) {
					// turn off the red led and turn on the yellow led
					PORTD = (PORTD & ~(1 << RED)) | (1 << YELLOW);
				} else {
					// turn off the yellow led and turn on the red led
					PORTD = (PORTD & ~(1 << YELLOW)) | (1 << RED);
				}
				// Reset the button flag value to 0 to not enter here again until the button is released.
				toggle_flag = 0;
			}
		} else {

			// button is released set the button flag to value 1 again.
			toggle_flag = 1;
		}

		// check if hours increment button pressed
		if (!(PINB & (1 << HOURS_INC_BT))) {
			if (hours_inc_flag) {
				if (hours < 23) {
					hours++;
					// Reset the button flag value to 0 to not enter here again until the button is released.
					hours_inc_flag = 0;
				}
			}
		} else {

			// button is released set the button flag to value 1 again.
			hours_inc_flag = 1;
		}

		// check if hours decrement button pressed
		if (!(PINB & (1 << HOURS_DEC_BT))) {
			if (hours_dec_flag) {
				if (hours) {
					hours--;
					// Reset the button flag value to 0 to not enter here again until the button is released.
					hours_dec_flag = 0;
				}
			}
		} else {

			// button is released set the button flag to value 1 again.
			hours_dec_flag = 1;
		}

		// check if minutes increment button pressed
		if (!(PINB & (1 << MIN_INC_BT))) {
			if (min_inc_flag) {
				increment_minutes();
				// Reset the button flag value to 0 to not enter here again until the button is released.
				min_inc_flag = 0;
			}
		} else {

			// button is released set the button flag to value 1 again.
			min_inc_flag = 1;
		}

		// check if minutes decrement button pressed
		if (!(PINB & (1 << MIN_DEC_BT))) {
			if (min_dec_flag) {
				decrement_minutes();
				// Reset the button flag value to 0 to not enter here again until the button is released.
				min_dec_flag = 0;
			}
		} else {

			// button is released set the button flag to value 1 again.
			min_dec_flag = 1;
		}

		// check if seconds increment button pressed
		if (!(PINB & (1 << SEC_INC_BT))) {
			if (sec_inc_flag) {
				increment_seconds();
				// Reset the button flag value to 0 to not enter here again until the button is released.
				sec_inc_flag = 0;
			}
		} else {

			// button is released set the button flag to value 1 again.
			sec_inc_flag = 1;
		}

		// check if seconds decrement button pressed
		if (!(PINB & (1 << SEC_DEC_BT))) {
			if (sec_dec_flag) {
				decrement_seconds();
				// Reset the button flag value to 0 to not enter here again until the button is released.
				sec_dec_flag = 0;
			}
		} else {

			// button is released set the button flag to value 1 again.
			sec_dec_flag = 1;
		}

		// check if stop buzzer button pressed
		if (!(PINA & (1 << STOP_BUZZER))) {
			stop_buzzer_flag = 1;
		}

		// check if count down reaches zero
		if (buzzer_on_flag && !stop_buzzer_flag) {
			// turn on buzzer
			PORTD |= (1 << BUZZER);
		} else {
			// turn off buzzer
			PORTD &= ~(1 << BUZZER);
		}

	}

	return 0;
}

/*
 * For System clock = 16Mhz and prescale F_CPU/1024.
 * Timer frequency will be around 15Khz, Ttimer = 64us
 * So we just need 15625 counts to get 1s period.
 * Compare interrupt will be generated every 1s.
 */
void TIMER1_CTC_MODE_INIT(void) {

	// Set timer1 initial count to zero
	TCNT1 = 0;
	// Set the Compare value to 15625
	OCR1A = 15625;

	// Enable global interrupts in MC
	TIMSK |= (1 << OCIE1A);

	/* Configure timer control register TCCR1A
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4)
	 */
	TCCR1A = (1 << FOC1A);

	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Prescaler = F_CPU/1024  CS10=1 CS11=0 CS12=1
	 */
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);
}

/* Interrupt Service Routine for timer1 compare mode */
ISR(TIMER1_COMPA_vect) {
	timer1_flag = 1;
}

/* Function to print the first digit of a number on 7-segment */
void printFirstDigit(char num) {
	// get first digit
	num %= 10;
	// Display the digit on the 7-segment
	PORTC = (PORTC & 0xF0) | (num & 0x0F);
}

/* Function to print the second digit of a number on 7-segment */
void printSecondDigit(char num) {
	// get second digit
	num /= 10;
	// Display the digit on the 7-segment
	PORTC = (PORTC & 0xF0) | (num & 0x0F);
}

/* External INT0 enable and configuration function */
void INT0_INIT(void) {
	// Configure INT0/PD2 as input pin
	DDRD &= ~(1 << PD2);

	// Activate the internal pull up resistor at PD2
	PORTD |= (1 << PD2);

	// Trigger INT0 with the falling edge
	MCUCR |= (1 << ISC01);
	MCUCR &= ~(1 << ISC00);

	// Enable external interrupt pin INT0
	GICR |= (1 << INT0);
}

/* External INT0 Interrupt Service Routine */
// Resets the stopwatch
ISR(INT0_vect) {
	seconds = 0;
	minutes = 0;
	hours = 0;
	TCNT1 = 0;
}

/* External INT1 enable and configuration function */
void INT1_INIT(void) {
	// Configure INT1/PD3 as input pin
	DDRD &= ~(1 << PD3);

	// Trigger INT1 with the rising edge
	MCUCR |= (1 << ISC11) | (1 << ISC10);

	// Enable external interrupt pin INT1
	GICR |= (1 << INT1);
}

/* External INT1 Interrupt Service Routine */
// pauses the stopwatch
ISR(INT1_vect) {
	// No clock, stop counter
	TCCR1B &= ~0X07;
	stop_buzzer_flag = 0;
}

/* External INT2 enable and configuration function */
void INT2_INIT(void) {
	// Configure INT2/PB2 as input pin
	DDRB &= ~(1 << PB2);

	// Activate the internal pull up resistor at PB2
	PORTB |= (1 << PB2);

	// Trigger INT2 with the falling edge
	MCUCSR &= ~(1 << ISC2);

	// Enable external interrupt pin INT0
	GICR |= (1 << INT2);
}

/* External INT2 Interrupt Service Routine */
// Resumes the stopwatch
ISR(INT2_vect) {
	TCCR1B |= (1 << CS10) | (1 << CS12);
}

// Function increments seconds
void increment_seconds(void) {
	if (seconds < 59) {
		seconds++;
		return;
	}
	increment_minutes();
	seconds = 0;
}

// Function decrements seconds
void decrement_seconds(void) {
	if (seconds) {
		seconds--;
		return;
	}
	if ((!minutes) && (!hours)) {
		return;
	}
	decrement_minutes();
	seconds = 59;
}

// Function increments minutes
void increment_minutes(void) {
	if (minutes < 59) {
		minutes++;
		return;
	}
	if (hours < 23) {
		hours++;
		minutes = 0;
	} else {
		hours = 0;
		minutes = 0;
		seconds = 0;
	}
}

// Function decrements seconds
void decrement_minutes(void) {
	if (minutes) {
		minutes--;
		return;
	}
	if (hours) {
		hours--;
		minutes = 59;
	}

}

