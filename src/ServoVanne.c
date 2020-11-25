/*****************************************************************************
//  File Name    : ServoVanne.c
//
*****************************************************************************/
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "uart_printf.h"
#include "timer1.h"
#include "timer0_stepper.h"
#include "lcd_5110.h"
#include "temperature.h"


#define VERSION "Pomp v 2.03"

/*************
 * History:
 * 256 table values (8bit)
 * Each table entry is the mean value: Sigma / N
 * Index loop for ever modulo 256
 * N_max depends on the rate of values (Typ 1 per Sec)
 *******************************/
#define TEMP_8_UNDEF -128
#pragma pack(1)
struct
{
    signed char Values[256];
    int N_max;
    int N;
    unsigned char Index;
}TempHistory;
#pragma pack()

#pragma pack(1)
struct
{
    // Count Hundredth of hour of active state
    unsigned char Values[24];
    int N;
    unsigned char Index;
}CycleHistory;
#pragma pack()

/* Global variables */
static char TempString[16];  // Dangerous !
static unsigned short TargetVanne;
static unsigned short StepperRampUp;
// Current Heating Cycle time
static unsigned short OnStateTime;

int OutdoorTemp;

/* Pomp_cmd connected to Pin C2 (25) */
#define PompInit() (DDRC |= (1 << PINC2)) // Pomp_cmd LED as output
#define PompOn()   (PORTC |= (1 << PINC2))
#define PompOff()  (PORTC &= ~(1 << PINC2))


#define MAX_OPENING_TIME (30*60)

/* Return a value in mm
 * corresponding to the desired opening ratio for the valve
 * There's no heating before 50 % !
 */
#define MAX_OPENING 58
#define MIN_OPENING 5
unsigned short TempToValve( int temperature )
{
    if      ( temperature > +250 )  /* 25° */
        return MIN_OPENING;
    else if ( temperature > +200 )
        return 25;
    else if ( temperature > +160 )
        return 28;
    else if ( temperature > +120 )
        return 30;
    else if ( temperature > +80 )
        return 34;
    else if ( temperature > +60 )
        return 38;
    else if ( temperature > +10 )
        return 41;
    else if ( temperature > -30 )
        return 44;
    else if ( temperature > -60 )
        return 48;
    else if ( temperature > -100 )
        return 52;
    else
        return MAX_OPENING;  // 58 mm    // 100 %

}



unsigned char Thermostat( void )
{
    if ( PINB & (1<<PINB0) )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void HistoryInit( long int Depth )
{
    int i;

    for ( i=0 ; i< 256 ; i++ )
    {
        TempHistory.Values[i] = TEMP_8_UNDEF;
    }
    TempHistory.Index = 0;
    TempHistory.N_max = (Depth+128L)/256;
    TempHistory.N = TempHistory.N_max-1;
}

/* Add value to history
 * Params T Temp in 10th degrees
 *
 **********************************************/
void HistoryAddValue( int T )
{
    long int Value;

    if ( TempHistory.Values[TempHistory.Index] == TEMP_8_UNDEF )
        TempHistory.Values[TempHistory.Index] = (signed char)(T/4);
    /* Get current value, if N==0, Value is reset */
    Value  = 4 * (long int)TempHistory.Values[TempHistory.Index];
    Value *= (long int)TempHistory.N;
    Value += (long int)T;
    TempHistory.N++;
    /* Calculate the mean value */
    TempHistory.Values[TempHistory.Index] = (signed char)((Value/(long int)TempHistory.N)/4);
    /* Test for next table entry */
    if ( TempHistory.N >= TempHistory.N_max )
    {
        TempHistory.Index++;
        TempHistory.N = 0;
        TempHistory.Values[TempHistory.Index] = (signed char)(T/4);
        //printf("TempHistory.Index=%d\n", TempHistory.Index );
    }
}


int HistoryGetMax( void )
{
    int i;
    int max = -128;

    //printf( "N= %d Index = %d\n", TempHistory.N, TempHistory.Index );

    for ( i=0 ; i< 256 ; i++ )
    {
        if ( (TempHistory.Values[i] != TEMP_8_UNDEF) && (TempHistory.Values[i] > max) )
        {
            max = TempHistory.Values[i];
        }
    }

    return ((int)max * 4);
}

int HistoryGetMin( void )
{
    int i;
    int min = +127;

    for ( i=0 ; i< 256 ; i++ )
    {
        if ( (TempHistory.Values[i] != TEMP_8_UNDEF) && (TempHistory.Values[i] < min) )
        {
            min = TempHistory.Values[i];
        }
    }

    return ((int)min * 4);
}

/*
 * Add a point in cycle (On/off)
 * Supposed to be called every Second
 *
 ********************************************/
void CycleHistoryAdd( unsigned char State )
{
    CycleHistory.N++;
    if ( (CycleHistory.N % 60) != 0 )
    {
        return;
    }
    if ( State != 0 )
    {
        CycleHistory.Values[CycleHistory.Index]++;
    }
    if ( CycleHistory.N >= 3600 )
    {
        CycleHistory.N = 0;
        CycleHistory.Index++;
        if ( CycleHistory.Index >= 24 )
        {
            CycleHistory.Index = 0;
        }
        CycleHistory.Values[CycleHistory.Index]=0;
    }
}

/*
 * Return the number of minutes at active State
 *
 ********************************************/
unsigned int CycleHistoryRead( void )
{
    int i;
    unsigned int Total;

    Total = 0;
    for ( i=0; i< 24 ; i++ )
    {
        Total += (unsigned int)CycleHistory.Values[i];
    }

    return Total;
}

void FormatCycleHistoryToString( void )
{
    int i;
    unsigned int Total;
    unsigned int H, M;

    Total = 0;
    for ( i=0; i< 24 ; i++ )
    {
        Total += (unsigned int)CycleHistory.Values[i];
    }

    H = Total / 60;
    M = Total % 60;
    sprintf_P(TempString, PSTR("Cycle: %2uh%02um "), H, M);
}



int DbgTemperatureRead(void)
{
    static int temp;
    static int dt;

    if ( dt == 0 )
        dt = 5;
    temp+=dt;
    if ( (temp > 220) || (temp < -150) )
        dt = -dt;

    return -12;
}

void TemperatureFormatString( int temp )
{
    int tenth;

    tenth = temp%10;
    if ( tenth < 0 )
        tenth = -tenth;
    sprintf_P(TempString, PSTR("%+01d.%d` "),  temp/10, tenth );
}


int main(void)
{
    unsigned long Tick=0;
    unsigned short RampUp;
    int temp;

    OnStateTime=0;
    TargetVanne=0;
    StepperRampUp=0;

    timer1_init();
    timerO_Stepper_Init(0);
    TemperatureInit();

    //  Enable global interrupts
    sei();

    // Initialize UART print
    uart_printf_init();
    // Init LCD device
    LcdInitialise();

    PompInit();

    // 24 H history => about 6 Min for each interval
    HistoryInit( 3600L*24 );

    //printf("ServoVanneInit\n\n");

    DDRB |= (1 << PINB2) | (1 << PINB1); // Set LEDs as output

    Lcd_DrawStringXY( VERSION,  1, 2 );
    Lcd_DrawStringXY( __DATE__, 1, 4 );
    MsSleep(2000);
    LcdClear();

//  while ( 1 )
//  {
//      sprintf_P(TempString, PSTR("%02X %02X %02X "), (int)PORTB, (int)PORTC, (int)PORTD );
//      Lcd_DrawStringXY( TempString, 20, 0 );
//  }

    while ( 1 )
    {
        /* Test One second elapse */
        if ( timer1_GetTicks() >= (Tick + 10) )
        {
            Tick = timer1_GetTicks();

            //Lcd_DrawStringXY( "ABCDEFGHIJKLM", 0, 1 );

            //OutdoorTemp = 210;
            OutdoorTemp = TemperatureRead();
            // Min Max management
            HistoryAddValue( OutdoorTemp );

            //Display Current Min Max;
            TemperatureFormatString( OutdoorTemp );
            Lcd_DrawStringLargeXY( TempString, 12, 3 );
            //printf_P( PSTR("Temp= %s \n"), TempString );

            temp = HistoryGetMin();
            TemperatureFormatString( temp );
            Lcd_DrawStringXY( TempString, 0, 5 );

            temp = HistoryGetMax();
            TemperatureFormatString( temp );
            Lcd_DrawStringXY( TempString, 50, 5 );

            // Convert outdoor temp to opening (%)
            TargetVanne = TempToValve( OutdoorTemp );
            // Add few mm if state is on for a long time
            if ( (OnStateTime > 3600 ) && (TargetVanne < MAX_OPENING) )
            {
                OnStateTime=0;
                TargetVanne+=1;
            }

            sprintf_P(TempString, PSTR("Vanne: "));
            Lcd_DrawStringXY( TempString, 3, 0 );

            //sprintf_P(TempString, PSTR("Cycle: %u mn   "), CycleHistoryRead());
            FormatCycleHistoryToString();
            Lcd_DrawStringXY( TempString, 3, 1 );
            if ( Thermostat() != 0  )
            {
                PompOn();
                RampUp = (StepperRampUp+128)/256;
                //sprintf_P(TempString, PSTR("%2u->%2u%%  "), StepperRampUp/((MAX_OPENING*256)/100), (TargetVanne*100)/MAX_OPENING );
                sprintf_P(TempString, PSTR("%2u/%2u  "), RampUp, TargetVanne );
                PORTB |= (1 << PINB2);  // Red Led ON
                if ( RampUp < TargetVanne )
                    StepperRampUp += ((MAX_OPENING*256)/MAX_OPENING_TIME);
                CycleHistoryAdd(1);
                OnStateTime++;
            }
            else
            {
                PompOff();
                StepperRampUp = (MIN_OPENING*256);
                sprintf_P(TempString, PSTR("Arret "));
                PORTB &= ~(1 << PINB2);  // Red Led OFF
                CycleHistoryAdd(0);
                OnStateTime=0;
            }
            Lcd_DrawStringXY( TempString, 45, 0 );
            //printf_P( PSTR("RampUp=%u/TargetVanne=%u mm \n"), StepperRampUp, TargetVanne );
            //printf_P( PSTR("StepperRampUp=%u\n"), StepperRampUp );

            /* Set Stepper value (Range 0-64 mm) */
            if ( StepperRampUp > (255*MAX_OPENING) )
                timerO_Stepper_SetValue( MAX_OPENING );
            else
                timerO_Stepper_SetValue( (StepperRampUp+128) / 256 );
        }

    }

  return 0;
}

/* EOF  */



