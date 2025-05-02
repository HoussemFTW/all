#include <xc.h>

// Configuration bits for PIC18F252
#pragma config OSC = HS         // High-Speed Oscillator mode
#pragma config WDT = OFF        // Watchdog Timer disabled
#pragma config PWRT = ON        // Power-up Timer enabled
#pragma config BOR = ON         // Brown-out Reset enabled
#pragma config LVP = OFF        // Low-Voltage Programming disabled

#define _XTAL_FREQ 20000000    // 20 MHz Crystal Oscillator

// SPI Initialization (Slave)
void SPI_Init_Slave() {
    TRISCbits.TRISC3 = 1;  // SCK (RC3) as input
    TRISCbits.TRISC4 = 1;  // SDI (RC4) as input
    TRISCbits.TRISC5 = 0;  // SDO (RC5) as output
    SSPSTAT = 0b01000000;  // Set CKE = 1 (SPI Mode 1)
    SSPCON1 = 0b00100100;  // Slave mode

}

unsigned char SPI_Transfer( uint8_t data) {
    SSPBUF = data;         // Write data to buffer
    while (!SSPSTATbits.BF); // Wait for transfer to complete
    return SSPBUF;         // Return received data
}

void main() {
    SPI_Init_Slave();      // Initialize SPI
    TRISB=0x00;
    
     uint8_t receivedData;
     uint8_t sendData;  // Initial response value

    while (1) {
       
            receivedData = SPI_Transfer(sendData);
           
            PORTB=receivedData;
            sendData=receivedData+1;

           
       
    }
}
