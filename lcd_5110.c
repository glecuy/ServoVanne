/***********************************************************************
 * 
 * 
 * http://playground.arduino.cc/Code/PCD8544#.UwjqHpXj5ko
 * http://skyduino.wordpress.com/2012/01/24/tutoriel-arduino-ecran-de-nokia-5110/
 * 
 *********************************************************************/
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "timer.h"

#include "terminal6.h"
#include "terminal12.h"

// PCD8544 interfacing
#define PIN_SCE   (1 << PIND5)
#define PIN_RESET (1 << PIND7)
#define PIN_DC    (1 << PIND4)
#define PIN_SDIN  (1 << PIND3)
#define PIN_SCLK  (1 << PIND2)

#define LCD_COMMAND     0
#define LCD_DATA        1

#define LCD_X     84
#define LCD_Y     48


void LcdWrite(unsigned char dc, unsigned char data)
{
	unsigned char Bit;

	if ( dc == LCD_COMMAND )
		PORTD &= ~PIN_DC;
	else
		PORTD |=  PIN_DC;

	// SCE low
	PORTD &= ~PIN_SCE;
	for ( Bit = 0x80 ;  Bit != 0 ; Bit >>= 1 )
	{
		// SCLK low
		PORTD &= ~PIN_SCLK;		
		if ( data & Bit )
		{
			PORTD |= PIN_SDIN;
		}
		else
		{
			PORTD &= ~PIN_SDIN;
		}
		// SCLK low
		PORTD |= PIN_SCLK;		
	}

	// SCE High
	PORTD |= PIN_SCE;
}


void LcdClear(void)
{
	int index;
	for (index = 0; index < LCD_X * LCD_Y / 8; index++)
	{
		LcdWrite(LCD_DATA, 0x00);
	}
}

void LcdInitialise(void)
{	
	DDRD |= (PIN_SCE|PIN_RESET|PIN_DC|PIN_SDIN|PIN_SCLK);
	//digitalWrite(PIN_RESET, LOW);
	//digitalWrite(PIN_RESET, HIGH);
	PORTD &= ~PIN_RESET;
	//MsSleep(1);
	PORTD |=  PIN_RESET;
	//MsSleep(1);
	// SCE High
	PORTD |= PIN_SCE;
		
	// Initialization Sequence
	LcdWrite(LCD_COMMAND, 0x21); // Extended Commands
	LcdWrite(LCD_COMMAND, 0xBF); // LCD VOP (contrast) - 0xB1 @ 3v3 ou 0xBF @ 5v
	LcdWrite(LCD_COMMAND, 0x04); // Temp coefficent
	LcdWrite(LCD_COMMAND, 0x14); // LCD bias mode = 1:48
	LcdWrite(LCD_COMMAND, 0x20); // Commit change
	LcdWrite(LCD_COMMAND, 0x0C); // Display control = normal mode (0x0D pour mode "négatif")
	
	LcdClear();
}

/* Draw at x(0->83) , y(0->5) */
void Lcd_DrawStringXY(char *characters, unsigned char x, unsigned char y )
{
	int index;
	char character;
	
	while ( (character = *characters) != '\0' )
	{	
		LcdWrite(LCD_COMMAND, 0x80 | (x));  // Colonne
		LcdWrite(LCD_COMMAND, 0x40 | (y));  // Ligne		
		for (index = 0; index < 5; index++)
		{
			LcdWrite(LCD_DATA, pgm_read_byte_near(&Terminal6x8[character - 0x20][index]) );
		}
		LcdWrite(LCD_DATA, 0x00);
		
		characters++;
		x+=6;  // Spacing ... */
	}
}


void Lcd_DrawStringLargeXY(char *characters, unsigned char x, unsigned char y )
{
	int index;
	char character;
	
	while ( (character = *characters) != '\0' )
	{	
		LcdWrite(LCD_COMMAND, 0x80 | (x));  // Colonne
		LcdWrite(LCD_COMMAND, 0x40 | (y));  // Ligne		
		for (index = 0; index < 11; index++)
		{
			LcdWrite(LCD_DATA, pgm_read_byte_near(&Terminal11x16[character - 0x20][2*index]) );
		}
		LcdWrite(LCD_DATA, 0x00);
		LcdWrite(LCD_COMMAND, 0x80 | (x));    // Colonne
		LcdWrite(LCD_COMMAND, 0x40 | (y+1));  // Ligne		
		for (index = 0; index < 11; index++)
		{
			LcdWrite(LCD_DATA, pgm_read_byte_near(&Terminal11x16[character - 0x20][2*index+1]) );
		}
		LcdWrite(LCD_DATA, 0x00);
		
		characters++;
		x+=12;  // Spacing ... */
	}
}
