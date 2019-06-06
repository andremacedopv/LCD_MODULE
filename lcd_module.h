/*
 * lcd_module.h
 *
 *  Created on: 5 de jun de 2019
 *      Author: andre
 */
#include <stdint.h>

#ifndef LCD_MODULE_H_
#define LCD_MODULE_H_

void delay(long limite);

void I2CConfig_UCB1(uint8_t ownAddr, uint8_t baudRate_kHz, uint8_t isMaster);

void PCF_STT_STP(void);

void LCD_aux(char dado);

void LCD_inic(void);

void I2C_write_UCB1(char dado);

uint8_t I2C_read_UCB1();

void lcdBacklightON(void);

void lcdBacklightOFF(void);

void WriteASCII_B1(uint8_t ascii);

void WriteString_B1(char *string);

void LCD_clear();

void LCD_changeCursorPosition(uint8_t row, uint8_t col);

#endif /* LCD_MODULE_H_ */
