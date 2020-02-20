/*    Author: Chris Lo
 *  Partner(s) Name:
 *    Lab Section:
 *    Assignment: Lab # 10  Exercise # 1
 *    Exercise Description: [optional - include for your own benefit]
 *
 *    I acknowledge all content contained herein, excluding template or example
 *    code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#define button (~PINA&0x07)

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;			
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;	
	TIMSK1 = 0x02;	
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;	//0x80: 10000000
}

void TimerOff() {
	TCCR1B = 0x00; //bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;	
	if (_avr_timer_cntcurr == 0) { 
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
    return (b ? (x | (0x01 << k)) : (x & ~(0x01 << k)));
}

typedef struct
{
    int state;
    unsigned long period;
    unsigned long eTime;
    int (*Tick)(int);
}LED;

LED makeTask(int state, unsigned long period, int(*Tick)(int)){
    LED curr;
    curr.state = state;
    curr.period = period;
    curr.eTime = 0;
    curr.Tick = Tick;
    return curr;
}

#define taskNum 4
#define MAX 4
enum BuzzSM{startBuzz, waitBuzz, play, offBuzz};
enum ThreeLEDsSM{start,s1};
enum BlinkingLEDSM{startBL,on,off};
enum CombineLEDsSM{startCOM,blink};
enum pulse{startPulse, up, down, wait} pulses;
unsigned char led3;
unsigned char ledBl;
unsigned char tempBuzz = 0x00;
unsigned long pulse=0;
unsigned long timePeriod = 2;

LED task[taskNum];

int Buzz(int state);
int tick3(int state);
int tickBL(int state);
int tickCOM(int state);
void TickPulse();

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF, PORTC = 0x00;
    DDRB = 0xFF, PORTB = 0x00;
    DDRA = 0x00, PORTA = 0xFF;
    pulses = startPulse;
    task[0] = makeTask(start,300,&tick3);
    task[1] = makeTask(startBL,1000,&tickBL);
    task[2] = makeTask(startBuzz,1,&Buzz); 
    task[3] = makeTask(startCOM,1,&tickCOM);

    /* Insert your solution below */
    while (1) {
        TickPulse();
	TimerSet(timePeriod);
        TimerOn();
	for (unsigned char i = 0; i < taskNum; i++) {
	   if (task[i].eTime >= task[i].period) {
	      task[i].state = task[i].Tick(task[i].state);
	      task[i].eTime = 0;
	   }
	   task[i].eTime += timePeriod;
	}
	while(!TimerFlag);
        TimerFlag = 0; 
    }
  return 0;
}

void TickPulse(){
  unsigned char press;
  unsigned char index;
  index = timePeriod; 
  switch(pulses){
   case startPulse:
	pulses = wait;
        press = 0;
	break;
   case wait:
	if(button == 0x01){
	  pulses = up;
	  press = 1;
	}
	else if(button == 0x02){
	  pulses = down;
	  press = 1;
	}
	else
	  pulses = wait;
   	break;
   case up:
	if(button == 0x01)
	  pulses = up;
	else 
	  pulses = wait;
	break;
   case down:
	if(button == 0x02)
          pulses = down;
	else 
	  pulses = wait;
	break;
   default: pulses = startPulse; break;
   }

   switch(pulses){
   case startPulse:
        break;
   case wait:
        PORTC = 0;
	break;
   case up:
        if((button == 0x01)&&press){
          index++;
	  timePeriod = index;
	  press = 0;
	  PORTC = 0x40;
	}
        break;
   case down:
        if((button == 0x02)&&press){
          index--;
          timePeriod = index;
	  press = 0;
	  PORTC = 0x20;
	}
	   if(timePeriod <= 0x01)
	     timePeriod = 0x01;
        break;
   default: break;
  }


}
int tick3(int state){
  switch(state){

    case start:
      state = s1;
      led3 = MAX;
      break;
    

    case s1:
      state = s1;
      break;
    
    default:
      state = state;
      break;
  }

  switch(state){
        case start:
       break;

        case s1:
         if(led3 < MAX)
	  led3 <<= 1;
	 else 
	  led3 = 0x01; 
       break;

       default:
       break;
  }
  return state;
}

int tickBL(int state){
  switch(state){
    case startBL:
         state = on;
         ledBl = SetBit(ledBl,3,1);	 
    break;

    case on:
         state = off;
    break;

    case off:
         state = on;
    break;

    default:
         state = startBL;
       break;
  }

  switch(state){
    case startBL:
       break;

    case on:
       ledBl = SetBit(ledBl,3,1);
       break;

    case off:
       ledBl = SetBit(ledBl,3,0);
       break;

    default:
       break;
  }
  return state;
}
int Buzz(int state){
  switch(state){
    case startBuzz:
        state = offBuzz;
        pulse = 0;
	break;

    case waitBuzz:
        if(button == 0x04)
         state = play;
        else
         state = offBuzz;
        break;
   
    case play:
        if(button == 0x04)
         state = waitBuzz;
        else 
         state = offBuzz;
        break;

    case offBuzz:
        if(button == 0x04)
         state = play;
        else
         state = waitBuzz;
        break;

    default:
        state = startBuzz;
        break;
    }     
  switch(state){
    case startBuzz:
        break;   
         

    case waitBuzz:
         tempBuzz = SetBit(ledBl,4,0);
        break;

    case play:
         tempBuzz = SetBit(ledBl,4,1);
  
        break;
    
    case offBuzz:
        tempBuzz = SetBit(ledBl,4,0);
        break;

    default:
        break;
    }
    return state;
}

int tickCOM(int state){
  switch(state){
    case startCOM:
       state = blink;
       PORTB = ledBl | led3 | tempBuzz;
       break;

    case blink:
       state = blink;
       break;

    default:
       state = startCOM;
       break;

  }
  switch(state){
    case startCOM:
       break;

    case blink:
       PORTB = ledBl | led3 | tempBuzz;
       break;

    default:
       break;
  }
  return state;
}



