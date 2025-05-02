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
    nrfWrite(CONFIG, 0x0B);  // Power up, TX mode, CRC enabled

    CE = 1; // Enable CE
}
unsigned char payload;
void main(void){
    TRISB=0x00;
    SPI_Init_Master();
    NRF24_Init();
    
    __delay_ms(250);
    
    
    while(1){
    unsigned char status = nrfRead(STATUS);  // Read STATUS register
    
    if (status & 0x40) {  // Check if RX_DR (bit 6) is set (Data Ready)
        do {
            CSN = 0;
            SPI_Transfer(R_RX_PAYLOAD);  // Issue command to read payload
            payload = SPI_Transfer(0xFF);  // Read single byte payload
            CSN = 1;

            LATB=payload;  // Turn LED on/off based on received value
            
            status = nrfRead(STATUS);  // Read STATUS again to check if more packets exist

        } while (!(nrfRead(FIFO_STATUS) & 0x01));  // Repeat if RX FIFO is NOT empty

        nrfWrite(STATUS, 0x40);  // Clear RX_DR flag to allow new data reception
    }
}


    
}