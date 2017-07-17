//==========================================================================
// SPI
//==========================================================================
//#include "mcu.h"
#include "spi.h"

//==========================================================================
// Defines
//==========================================================================
//#define Cs0Init()       GPIOA_CRL.MODE4 = 1; GPIOA_CRL.CNF4 = 0   //FIO0DIR |= (1 << 6)
#define SetCs0High()    GPIOA->BSRR = GPIO_Pin_4
#define SetCs0Low()     GPIOA->BRR  = GPIO_Pin_4

//==========================================================================
// Send and Receive 1 Byte
//==========================================================================
unsigned char SpiByte (unsigned char data)
{
    //Check Status
    while ( (SPI1->SR & SPI_I2S_FLAG_TXE) == 0x00);
    //Write data to SPO0 register
    SPI1->DR = data;

    //Check Status
    while ( (SPI1->SR & SPI_I2S_FLAG_RXNE) == 0x00);
    //while ( (SPI1->SR & SPI_I2S_FLAG_BSY) == 0x00);
    //
    return SPI1->DR;
}

//==========================================================================
// Set nCS Init
//==========================================================================
void Cs0Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
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

void SPIPinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}

void SpiInit(void)
{
    // Init nCS pin (GPIO)
    Cs0Init();
    SetCs0High();

    // Pin Init 
    SPIPinInit();

    SPI_InitTypeDef  SPI_InitStructure;
    
    // Enable clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // SPI configuration
    SPI_Cmd(SPI1, DISABLE);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    
    // Apply I2C configuration after enabling it
    SPI_Init(SPI1, &SPI_InitStructure);

    // I2C Peripheral Enable
    SPI_Cmd(SPI1, ENABLE);
    
}
