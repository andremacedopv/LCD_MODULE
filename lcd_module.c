#include <msp430.h>
#include <string.h>
#include "lcd_module.h"

#define PCF_ADR 0x27
#define BIT_BL BIT3
#define BIT_E BIT2
#define BIT_RW BIT1
#define BIT_RS BIT0

uint32_t cursorPosition = 0;

void delay(long limite){
    volatile long cont=0;
    while (cont++ < limite) ;
}

void I2CConfig_UCB1(uint8_t ownAddr, uint8_t baudRate_kHz, uint8_t isMaster){
    // Configurar pinos 5.0 para SDA e 5.1 para SCL do UCB1
    P5SEL0 |= (BIT0|BIT1);
    P5SEL1 &= ~(BIT1|BIT0);
    P5REN |= (BIT0|BIT1);
    P5OUT |= (BIT0|BIT1);

    // Configurar modulo I2C UCB1
    UCB1CTLW0 = UCSWRST;
    UCB1CTLW0 |= UCMODE_3 | UCSYNC | UCSSEL__SMCLK;
    if(isMaster){
        UCB1CTLW0 |= UCMST;
    }
    UCB1BRW = 1000/baudRate_kHz;
    UCB1I2COA0 = ownAddr;
    UCB1CTLW0 &= ~UCSWRST;
}

// Gerar START e STOP para colocar PCF em estado conhecido
void PCF_STT_STP(void){
    int x=0;
    UCB1I2CSA = PCF_ADR;        //Endereço Escravo

    while (x<5){
        UCB1CTLW0 |= UCTR    |   //Mestre TX
                    UCTXSTT;    //Gerar START
        while ( ((UCB1IFG & UCTXIFG0) == 0) && ((UCB1IFG & UCNACKIFG) == 0));  //Esperar TXIFG=1
        UCB1CTLW0 |= UCTXSTP;                //Gerar STOP
        delay(200);
        if ( (UCB1CTLW0 & UCTXSTP) == 0)   break;   //Esperar STOP
        x++;
    }
    while ( (UCB1CTLW0 & UCTXSTP) == UCTXSTP);   //I2C Travado (Desligar / Ligar)
}

// Auxiliar inicialização do LCD (RS=RW=0)
// *** Só serve para a inicialização ***
void LCD_aux(char dado){
    while ( (UCB1IFG & UCTXIFG0) == 0);              //Esperar TXIFG=1
    UCB1TXBUF = ((dado<<4)&0XF0) | BIT_BL;          //PCF7:4 = dado;
    delay(50);
    while ( (UCB1IFG & UCTXIFG0) == 0);              //Esperar TXIFG=1
    UCB1TXBUF = ((dado<<4)&0XF0) | BIT_BL | BIT_E;  //E=1
    delay(50);
    while ( (UCB1IFG & UCTXIFG0) == 0);              //Esperar TXIFG=1
    UCB1TXBUF = ((dado<<4)&0XF0) | BIT_BL;          //E=0;
}

// Incializar LCD modo 4 bits
void LCD_inic(void){

    PCF_STT_STP();      //Colocar PCF em estado conhecido

    // Preparar I2C para operar
    UCB1I2CSA = PCF_ADR;    //Endereço Escravo
    UCB1CTL1 |= UCTR    |   //Mestre TX
                UCTXSTT;    //Gerar START
    while ( (UCB1IFG & UCTXIFG0) == 0);          //Esperar TXIFG=1
    UCB1TXBUF = 0;                              //Saída PCF = 0;
    while ( (UCB1CTL1 & UCTXSTT) == UCTXSTT);   //Esperar STT=0
    if ( (UCB1IFG & UCTXIFG0) == UCNACKIFG)    //NACK?
                while(1);

    // Começar inicialização
    LCD_aux(0);     //RS=RW=0, BL=1
    delay(20000);
    LCD_aux(3);     //3
    delay(10000);
    LCD_aux(3);     //3
    delay(10000);
    LCD_aux(3);     //3
    delay(10000);
    LCD_aux(2);     //2

    // Entrou em modo 4 bits
    LCD_aux(2);     LCD_aux(8);     //0x28
    LCD_aux(0);     LCD_aux(8);     //0x08
    LCD_aux(0);     LCD_aux(1);     //0x01
    LCD_aux(0);     LCD_aux(6);     //0x06
    LCD_aux(0);     LCD_aux(0xF);   //0x0F

    while ( (UCB1IFG & UCTXIFG0) == 0)   ;          //Esperar TXIFG=1
    UCB1CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB1CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

// Escrever dado na porta
void I2C_write_UCB1(char dado){
    UCB1I2CSA = PCF_ADR;        //Endereço Escravo
    UCB1CTLW0 |= UCTR    |       //Mestre TX
                UCTXSTT;        //Gerar START

    while ((UCB1IFG & UCTXIFG0) == 0);   //Esperar TXIF0G=1
    UCB1TXBUF = dado;                    //Escrever dado no buffer

    while (((UCB1IFG & UCTXIFG0) == 0) && ((UCB1IFG & UCNACKIFG) == 0));   //Esperar STT=0
    if ((UCB1IFG & UCNACKIFG) == UCNACKIFG)       //NACK?
        while(1);                          //Escravo gerou NACK

    while((UCB1IFG & UCTXIFG0) == 0);
    UCB1IFG &= ~UCTXIFG0;

    UCB1CTLW0 |= UCTXSTP;                        //Gerar STOP
    while ((UCB1CTLW0 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

uint8_t I2C_read_UCB1(){
    uint8_t dado;
    UCB1I2CSA = PCF_ADR;        //Endereço Escravo
    UCB1CTLW0 &= ~UCTR;
    UCB1CTLW0 |= UCTXSTT;        //Gerar START

    while (((UCB1IFG & UCTXIFG0) == 0) && ((UCB1IFG & UCNACKIFG) == 0));   //Esperar STT=0
    UCB1CTLW0 |= UCTXSTP;
    while ((UCB1CTLW0 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    while((UCB1IFG & UCRXIFG0) == 0);
    dado = UCB1RXBUF;
    UCB1IFG &= ~UCRXIFG0;
    delay(50);
    return dado;
}

void lcdBacklightON(void){
    I2C_write_UCB1(BIT3);
    delay(50);
}

void lcdBacklightOFF(void){
    I2C_write_UCB1(0);
    delay(50);
}

void WriteASCII_B1(uint8_t ascii){
    // Escrever primeiro nibble do codigo ascii
    I2C_write_UCB1((ascii & 0xF0) | 0x09);
    I2C_write_UCB1((ascii & 0xF0) | 0x0D);
    I2C_write_UCB1((ascii & 0xF0) | 0x09);
    // Escrever segundo nibble do codigo ascii
    I2C_write_UCB1(((ascii & 0x0F) << 4) | 0x09);
    I2C_write_UCB1(((ascii & 0x0F) << 4) | 0x0D);
    I2C_write_UCB1(((ascii & 0x0F) << 4) | 0x09);
}

void WriteString_B1(char *string){
    uint8_t size = strlen(string);
    uint8_t i;

    for(i = 0; i < size && i < 16; i++){
        WriteASCII_B1(string[i]);
    }
}
void LCD_clear(){

    I2C_write_UCB1(0x08);
    I2C_write_UCB1(0x0C);
    I2C_write_UCB1(0x08);

    I2C_write_UCB1(0x18);
    I2C_write_UCB1(0x1C);
    I2C_write_UCB1(0x18);
}

void LCD_changeCursorPosition(uint8_t row, uint8_t col){
    if(row < 0 || row > 1 || col < 0 || col > 15){
        return;
    }

    uint8_t position = row*16 + col;
    if(row)
        position += 48;

    I2C_write_UCB1((position & 0xF0) | 0x88);
    I2C_write_UCB1((position & 0xF0) | 0x8C);
    I2C_write_UCB1((position & 0xF0) | 0x88);

    I2C_write_UCB1(((position & 0x0F) << 4) | 0x08);
    I2C_write_UCB1(((position & 0x0F) << 4) | 0x0C);
    I2C_write_UCB1(((position & 0x0F) << 4) | 0x08);
}
