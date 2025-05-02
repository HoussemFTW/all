/*
 * File:   newmainTX2.c
 * Author: Derouiche
 *
 * Created on 1 mai 2025, 22:30
 */


#include <xc.h>


#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


#define _XTAL_FREQ 30000000


void SPI_Init_Slave() {
    TRISCbits.TRISC3 = 1;  // SCK (RC3) as input
    TRISCbits.TRISC4 = 1;  // SDI (RC4) as input
    TRISCbits.TRISC5 = 0;  // SDO (RC5) as output
    SSPSTAT = 0b01000000;  // Set CKE = 1 (SPI Mode 1)
    SSPCON = 0b00100100;  // Slave mode

}


unsigned char SPI_Transfer(unsigned char data) {
    SSPBUF = data;         // Write data to buffer
    while (!SSPSTATbits.BF); // Wait for transfer to complete
    return SSPBUF;         // Return received data
}



void main(void) {
    SPI_Init_Slave(); 
    TRISB=0x00;
     uint8_t receivedData;
     uint8_t sendData;
     
     while (1) {
       
            receivedData = SPI_Transfer(sendData);
           
            PORTB=receivedData;
            sendData=receivedData+1;
            
     }
    
    
    
}
