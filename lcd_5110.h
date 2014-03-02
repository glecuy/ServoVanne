/***********************************************************************
 * 
 * lcd_5110.h
 * 
 *********************************************************************/
#ifndef _LCD_PCD8544_H_
#define _LCD_PCD8544_H_


void LcdClear(void);
void LcdInitialise(void);
void Lcd_DrawString(char *characters);

void Lcd_DrawStringXY(char *characters, unsigned char x, unsigned char y );
void Lcd_DrawStringLargeXY(char *characters, unsigned char x, unsigned char y );



#endif // _LCD_PCD8544_H_
