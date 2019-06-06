#include <msp430.h> 
#include "lcd_module.h"

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;
//	P1DIR |= BIT0;
//	P1OUT &= ~BIT0;

	I2CConfig_UCB1(0x34,50,1);
	LCD_inic();
    lcdBacklightON();

    char *string1 = "Andre";
    char *string2 = "Danilo";


	while(1){
	    WriteString_B1(string1);
	    delay(100000);
	    LCD_changeCursorPosition(1, 0);
	    WriteString_B1(string2);
	    delay(100000);
	    LCD_clear();
	    delay(100000);
	}

	return 0;
}
