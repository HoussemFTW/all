#include <xc.h>

// Configuration bits for PIC18F252
#pragma config OSC = HS         // High-Speed Oscillator mode
#pragma config WDT = OFF        // Watchdog Timer disabled
#pragma config PWRT = ON        // Power-up Timer enabled
#pragma config BOR = ON         // Brown-out Reset enabled
#pragma config LVP = OFF        // Low-Voltage Programming disabled

#define _XTAL_FREQ 20000000    // 20 MHz Crystal Oscillator

// SPI Initialization (Master)
void SPI_Init_Master() {
    TRISCbits.TRISC3 = 0;  // SCK (RC3) as output
    TRISCbits.TRISC4 = 1;  // SDI (RC4) as input
    TRISCbits.TRISC5 = 0;  // SDO (RC5) as output
    SSPSTAT = 0b01000000;  // Set CKE = 1 (SPI Mode 1)
    SSPCON1 = 0b00100000;  // Master mode
}

// SPI Data Transfer Function
unsigned char SPI_Transfer(unsigned char data) {
    SSPBUF = data;         // Write data to buffer
    while (!SSPSTATbits.BF); // Wait for transfer to complete
    return SSPBUF;         // Return received data
}

void main() {
    SPI_Init_Master();     // Initialize SPI

    TRISAbits.TRISA0 = 0;  // Set SS pin as output
    TRISB=0xFF;

    LATAbits.LATA0 = 1;    // SS high (slave not selected)

    unsigned char sendData; // Data to send
    unsigned char receivedData;

    while (1) {
        sendData=PORTB;
        // Select slave (SS low)
        LATAbits.LATA0 = 0;
        __delay_us(10);  // Small delay for stability

        // Send data to slave
        receivedData = SPI_Transfer(sendData);

        // Deselect slave (SS high)
        LATAbits.LATA0 = 1;
        __delay_us(10);  // Small delay

        __delay_ms(500);   // Delay for readability
    }
}
