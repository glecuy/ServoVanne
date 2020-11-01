/*
	timer0.c

	Copyright 2014-2020 GLecuyer <glecuyer207@gmail.com>


 */


#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>


// Two Phases Bipolar Stepper Motor Control

#define PH_PORT PORTC
#define PH_DDR  DDRC

#define PH1P_PIN PINC1
#define PH1M_PIN PINC3
#define PH0P_PIN PINC4
#define PH0M_PIN PINC5

// (25 / 0.8) * (360/1.8) =
#define MAX_STEPS_VALUE  16000 // 64 mm <=> 16000/(360/1.8) ⋅ .8
#define LIMIT_SWITCH_PORT PIND
#define LIMIT_SWITCH_PIN  PIND6

volatile unsigned short  StepsTarget;
volatile unsigned short  StepsCounter;
volatile unsigned char Phases;
volatile unsigned char InitDone;


inline void setPhasesPins( void ){
    unsigned char pPH;
    pPH = PH_PORT & ~( (1 << PH1P_PIN) | (1 << PH1M_PIN) | (1 << PH0P_PIN) | (1 << PH0M_PIN) );
    PORTC =  pPH | Phases;
}

void setPhasesPinsIdle (void){    
    Phases = (1 << PH1P_PIN) | (1 << PH1M_PIN) | (1 << PH0P_PIN) | (1 << PH0M_PIN);    
    setPhasesPins();
}




/*
 * PWM generator
 */
void timerO_Stepper_Init(unsigned char Val){
    
    // Set lines as output
	PH_DDR |= (1 << PH1P_PIN) | (1 << PH1M_PIN) | (1 << PH0P_PIN) | (1 << PH0M_PIN);

    InitDone = 0;
    StepsCounter = (2 * MAX_STEPS_VALUE);
    StepsTarget = 0;

    // disable stepper motor
    setPhasesPinsIdle();
    
    // *** timer 0 interrupt setup  ***
    TCCR0A = 0;  // set entire TCCR2A register to 0
    TCCR0B = 0;  // same for TCCR2B
    TCNT0 = 0;   //initialize counter value to 0
    
    // set compare match register for frequency generation
    OCR0A = 90;  // TODO fine tune
    
    // turn on CTC mode (Table 15-8)
    TCCR0A |= (1 << WGM01) | (0 << WGM00);   
    // Set CS01 and CS00 bits for 256 prescaler
    TCCR0B |= (1 << CS02) | (0 << CS01) | (0 << CS00);
    // enable timer compare interrupt
    TIMSK0 = (1 << OCIE0A);    
    
}

// Val unit is millimeter
void timerO_Stepper_SetValue(unsigned char Val)
{
    if (InitDone != 0){
        StepsTarget = Val * (MAX_STEPS_VALUE / 64) ;
    }
}

ISR(TIMER0_COMPA_vect)
{
    // PH_PORT ^= (1<<PH0M_PIN);  // test only
    if ( StepsCounter > StepsTarget ){
        StepsCounter--;
  
        if ( (LIMIT_SWITCH_PORT & (1<<LIMIT_SWITCH_PIN)) == 0 ){
        //if ( 0 ) {
            // Limit reached
            StepsCounter = 0;
            InitDone = 1;
            setPhasesPinsIdle();
        }
    }
    
    if ( StepsCounter < StepsTarget ){
        StepsCounter++;  
  
        if ( StepsCounter >= MAX_STEPS_VALUE ){
            // Limit reached
            StepsCounter = MAX_STEPS_VALUE;
            setPhasesPinsIdle();
        }
    }
    
    if ( StepsCounter != StepsTarget ){
        switch( StepsCounter & 0x3 ){
            case 0:
                // 1010
                Phases = (1 << PH1P_PIN) | (0 << PH1M_PIN) | (1 << PH0P_PIN) | (0 << PH0M_PIN);
                break;
            case 1:
                // 0110
                Phases = (0 << PH1P_PIN) | (1 << PH1M_PIN) | (1 << PH0P_PIN) | (0 << PH0M_PIN);
                break;
            case 2:
                //0101
                Phases = (0 << PH1P_PIN) | (1 << PH1M_PIN) | (0 << PH0P_PIN) | (1 << PH0M_PIN);
                break;
            default:
                //1001
                Phases = (1 << PH1P_PIN) | (0 << PH1M_PIN) | (0 << PH0P_PIN) | (1 << PH0M_PIN);
                break;
        }
    }
    if ( StepsCounter == StepsTarget ){
        // Value reached
        setPhasesPinsIdle();
    } 
    setPhasesPins();
}

