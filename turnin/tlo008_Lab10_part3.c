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
#define button (~PINA&0x04)

enum BuzzSM{startBuzz, play};
enum ThreeLEDsSM{start,s1};
enum BlinkingLEDSM{startBL,on,off};
enum CombineLEDsSM{startCOM,blink};

unsigned char led3;
unsigned char ledBl;
unsigned char tempBuzz = 0x00;
const unsigned long timePeriod = 2;

LED task[taskNum];

int Buzz(int state);
int tick3(int state);
int tickBL(int state);
int tickCOM(int state);

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF, PORTB = 0x00;
    unsigned char i = 0;

    task[i++] = makeTask(startBuzz,2,&Buzz);
    task[i++] = makeTask(start,300,&tick3);
    task[i++] = makeTask(startBL,1000,&tickBL);
    task[i]   = makeTask(startCOM,2,&tickCOM);
 
    TimerSet(timePeriod);
    TimerOn();

    /* Insert your solution below */
    while (1) {
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

int Buzz(int state){
  switch(state){
    case startBuzz:
	state = on;
	break;
   
    case play:
	if(button == 0x04)
	 state = play;
	else 
	 state = startBuzz;
	break;

    default:
	state = startBuzz;
	break;
    }	  

   switch(state){
    case startBuzz:
	tempBuzz = SetBit(tempBuzz,4,1);
        break;   
         
    case play:
	 tempBuzz = SetBit(tempBuzz,4,0);
        break;

    default:
        break;
    }     
    return state;
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



