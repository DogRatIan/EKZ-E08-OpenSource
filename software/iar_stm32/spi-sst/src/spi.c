//==========================================================================
// SPI
//==========================================================================
#include "mcu.h"
#include "spi.h"

//==========================================================================
// Defines
//==========================================================================
#define SetCs0High()    GPIOA_BSRR = (1 << 4)
#define SetCs0Low()     GPIOA_BRR  = (1 << 4)

//==========================================================================
// Send and Receive 1 Byte
//==========================================================================
int SpiByte (unsigned char data)
{
    unsigned short l;

    // Check Status - TXE
    l = 10000;
    while ((SPI1_SR & (0x01 << 1)) == 0x00)
    {
        l --;
        if (l == 0)
            return -1;
    }

    //Write data to SPO register
    SPI1_DR = data;

    // Check Status - RXNE
    l = 10000;
    while ((SPI1_SR & (0x01 << 0)) == 0x00)
    {
        l --;
        if (l == 0)
            return -1;
    }

    //while ( (SPI1->SR & SPI_I2S_FLAG_BSY) == 0x00);

    return SPI1_DR;
}

//==========================================================================
// Set nCS
//==========================================================================
void SpiSetCs (int aState)
{
    if (aState)
        SetCs0High ();
    else
        SetCs0Low ();
}

//==========================================================================
// Init SPI0
//==========================================================================

void SpiInit(void)
{
    // Init nCS pin (GPIO)
    SetCs0High();

    // Release SPI reset
    RCC_APB2RSTR &= (~(1 << 12));

    //Enable SPI clock
    RCC_APB2ENR  |=   (1 << 12);

    // SPI configuration
    SPI1_CR1 = 0;               // Disable SPI

    SPI1_CR1 = (  (0x01 << 0)   // CPHA
                | (0x01 << 1)   // CPOL
                | (0x01 << 2)   // Master Mode
                | (0x01 << 3)   // Baud rate= PCLK/4
                | (0x00 << 7)   // MSB first
                | (0x03 << 8)   // Software SSI control
                | (0x00 << 10)  // Full duplex
                | (0x00 << 11)  // 8-bit data
                | (0x00 << 12)  // CRC at data phase
                | (0x00 << 13)  // Disable CRC
                | (0x00 << 14)  // Unidirectional data mode (using MOSI amd MISO)
                );
 
    SPI1_CR2 = 0x00;
    SPI1_SR = 0x00;

    // SPI Peripheral Enable
    SPI1_CR1 |= (0x01 << 6);     // Enable SPI
}
