
#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
void TimerOn()
{
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff()
{
	TCCR1B = 0x00;
}
void TimerISR()
{
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0)
	{
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TickThree_LEDS();
void TickBL();
void TickCOM();
enum Three_States {Start, LED_1, LED_2, LED_3} Three_state;
enum Blinking_States{BL_Start, ON, OFF} BL_state;
enum COMBINE_States{COM_Start, OUTPUT} Combine_State;
unsigned char Blink = 0x00;
unsigned char Three = 0x00;
unsigned char temp = 0x00;
unsigned short count = 0x00;
unsigned short cnt = 0x00;

int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	Three_state = Start;
	BL_state = BL_Start;
	Combine_State = COM_Start;
	cnt = 0;
	count = 0;
	
	TimerSet(1);
	TimerOn();
    while(1)
    {
		TickThree_LEDS();
		TickBL();
		TickCOM();
        while (!TimerFlag);
        TimerFlag = 0;
    }
}

void TickThree_LEDS()
{
	switch(Three_state) // Transitions
	{
		case Start:
		{
			Three_state = LED_1;
			break;
		}
		case LED_1:
		{
			if(cnt < 1000)
			{
				Three_state = LED_1;
				++cnt;
			}
			else
			{
				Three_state = LED_2;
				cnt = 0;
			}
			break;
		}
		case LED_2:
		{
			if(cnt < 1000)
			{
				Three_state = LED_2;
				++cnt;
			}
			else
			{
				Three_state = LED_3;
				cnt = 0;
			}
			break;
		}
		case LED_3:
		{
			if(cnt < 1000)
			{
				Three_state = LED_3;
				++cnt;
			}
			else
			{
				Three_state = LED_1;
				cnt = 0;
			}
			break;
		}
		default:
			break;	
	}
	
	switch(Three_state) // State Actions
	{
		case Start:
			break;
		case LED_1:
		{
			Three = 0x01;
			break;
		}
		case LED_2:
		{
			Three = 0x02;
			break;
		}
		case LED_3:
		{
			Three = 0x04;
			break;
		}
		default:
			break;
	}
}
void TickBL()
{
	switch(BL_state) // Transitions
	{
		case BL_Start:
		{
			BL_state = ON;
			break;
		}
		case ON:
		{
			if(count < 1000)
			{
				BL_state = ON;
				++count;
			}
			else
			{
				BL_state = OFF;
				count = 0;
			}
			break;
		}
		case OFF:
		{
			if(count < 1000)
			{
				BL_state = OFF;
				++count;
			}
			else
			{
				BL_state = ON;
				count = 0;
			}
			break;
		}
		default:
			break;
	}
	switch(BL_state) // State Actions
	{
		case BL_Start:
			break;
		case ON:
		{
			Blink = 0x08;
			break;
		}
		case OFF:
		{
			Blink = 0x00;
		}
		default:
			break;
	}
}
void TickCOM()
{
	switch(Combine_State) // Transitions
	{
		case COM_Start:
		{
			Combine_State = OUTPUT;
			break;
		}
		case OUTPUT:
			break;
		default:
			break;
	}
	switch(Combine_State) // State Actions
	{
		case COM_Start:
			break;
		case OUTPUT:
		{
			temp = Blink | Three;
			PORTB = temp;
			break;
		}
	}
}
