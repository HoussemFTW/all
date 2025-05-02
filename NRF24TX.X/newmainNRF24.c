/*
 * File:   newmainNRF24.c
 * Author: Derouiche
 *
 * Created on 21 mars 2025, 19:45
 */


#include <xc.h>
#include "nrf24.h"

// Configuration bits for PIC18F252
#pragma config OSC = HS         // High-Speed Oscillator mode
#pragma config WDT = OFF        // Watchdog Timer disabled
#pragma config PWRT = ON        // Power-up Timer enabled
#pragma config BOR = ON         // Brown-out Reset enabled
#pragma config LVP = OFF        // Low-Voltage Programming disabled


#define _XTAL_FREQ 20000000    // 20 MHz Crystal Oscillator



void SPI_Init_Master() {
    TRISCbits.TRISC3 = 0;  // SCK (RC3) as output
    TRISCbits.TRISC4 = 1;  // SDI (RC4) as input
    TRISCbits.TRISC5 = 0;  // SDO (RC5) as output
    TRISCbits.TRISC1 = 0;  // CE as output
    TRISCbits.TRISC2 = 0;  // CSN as output
    SSPSTAT = 0b01000000;  // Set CKE = 1 (SPI Mode 1)
    SSPCON1 = 0b00100000;  // Master mode
}

unsigned char SPI_Transfer(unsigned char data) {
    SSPBUF = data;         // Write data to buffer
    while (!SSPSTATbits.BF); // Wait for transfer to complete
    return SSPBUF;         // Return received data
}

void nrfWrite(unsigned char reg,unsigned char val){
    CSN=0;
    SPI_Transfer(W_REGISTER | reg);
    SPI_Transfer(val);
    CSN=1;
}


unsigned char nrfRead(unsigned char reg){
    unsigned char val;
    CSN=0;
    SPI_Transfer(R_REGISTER | reg);
    val=SPI_Transfer(0xFF);
    CSN=1;
    return val;
}


void sendPayload(unsigned char *val){
    CSN=0;
    SPI_Transfer(W_TX_PAYLOAD);
    for (int i = 0; i < 5; i++) {
        SPI_Transfer(val[i]);
    }
    CSN = 1;
    
    // Pulse CE to send
    CE = 1;
    __delay_us(20);  // Pulse for at least 10µs
    CE = 0;
    
}





void NRF24_Init() {
    unsigned char tx_address[5] = {0x01, 0x01, 0x01, 0x01, 0x01}; 
    CE = 0; // Disable CE during setup
    CSN = 1; // Deselect NRF24

    __delay_ms(100); // Allow NRF24 to power up
    

    nrfWrite(CONFIG, 0x00);  // Power off
    nrfWrite(SETUP_AW, 0x03); // Set 5-byte address width (recommended)
    
    CSN=0;
    SPI_Transfer(W_REGISTER | TX_ADDR); // Write to TX_ADDR registerCSN=0;
    for (int i = 0; i < 5; i++) {
        SPI_Transfer(tx_address[i]);} // Send each byte
    CSN=1;
    
    CSN=0;
    SPI_Transfer(W_REGISTER | RX_ADDR_P0); // Write to TX_ADDR registerCSN=0;
    for (int i = 0; i < 5; i++) {
        SPI_Transfer(tx_address[i]);} // Send each byte
    CSN=1;
    
    nrfWrite(EN_AA, 0x01);    // Enable Auto-ACK on all pipes
    nrfWrite(SETUP_RETR, 0x2A); // 500µs delay, 10 retries
    nrfWrite(EN_RXADDR, 0x01);  // Enable RX Pipe 0 and Pipe 1
    nrfWrite(RF_CH, 40);  // Set RF channel to 76 (2.476 GHz)
    nrfWrite(RF_SETUP,0b00000110); //250kbps, 0dBm
    nrfWrite(RX_PW_P0, 1);  // Expect 1-byte payload on Pipe 0
    nrfWrite(CONFIG, 0x0A);  // Power up, TX mode, CRC enabled

    CE = 1; // Enable CE
}

//unsigned char onn[5]={1,1,1,1,1};
//unsigned char offf[5]={0,0,0,0,0};


void main(void) {
    
    TRISB=0xFF;
    
    SPI_Init_Master();
    NRF24_Init();
    
    
    __delay_ms(250);
    
    
    
    while(1){
    // Ensure the NRF24L01+ is ready for transmission
    CE = 0;
    __delay_us(130);  // TX Mode Settling Time

    // Check if TX FIFO is full before sending new data
    if (nrfRead(STATUS) & 0x10) {  // TX_FULL (bit 4) is set
        CSN = 0;
        SPI_Transfer(FLUSH_TX);  // Flush TX FIFO to prevent issues
        CSN = 1;
    }

    // Send first payload (0x01)
    CSN = 0;
    SPI_Transfer(W_TX_PAYLOAD);  
    SPI_Transfer(PORTB);          
    CSN = 1;

    CE = 1;  // Start transmission
    __delay_us(20);  // Pulse CE for at least 10µs
    CE = 0;

    // Wait for TX to complete or fail
    while (!(nrfRead(STATUS) & (0x20 | 0x10)));  // Wait for TX_DS or MAX_RT

    unsigned char status = nrfRead(STATUS);  // Read STATUS register

    if (status & 0x20) {  // TX_DS (Transmission successful)
        nrfWrite(STATUS, 0x20);  // Clear TX_DS flag
    } 
    else if (status & 0x10) {  // MAX_RT (Maximum retries reached)
        CSN = 0;
        SPI_Transfer(FLUSH_TX);  // Flush TX FIFO
        CSN = 1;
        nrfWrite(STATUS, 0x10);  // Clear MAX_RT flag
    }

    __delay_ms(500);  // Delay between transmissions

    
}


}