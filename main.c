#include <msp430.h> 
#include "lcd_module.h"

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;
	
	I2CConfig_UCB1(0x34,50,1);
	LCD_inic();
	lcdBacklightON();

	char *string = "Andre";
	WriteString_B1(string);

	while(1);

	return 0;
}
