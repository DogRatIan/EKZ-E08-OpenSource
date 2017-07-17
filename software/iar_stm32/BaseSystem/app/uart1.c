//-----------------------------------------------------------------------
// UART1
//-----------------------------------------------------------------------
#include "mcu.h"
#include "define.h"
#include "int.h"

#include "uart1.h"

//-----------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------
// PCLK2 = 72MHz
// Baud = PCLK2 / (16 * USARTDIV)
// VALUE_BRR = int (USARTDIV * 16)
#define VALUE_BRR       625         // 115200: USARTDIV = 39.0625


#define SIZE_RX_BUFFER      64
#define SIZE_TX_BUFFER      64

struct TCircularBuf
{
    unsigned char *Buf;
    int Size;
    int Head;
    int Tail;
};

//-----------------------------------------------------------------------
static unsigned char gRxBuf[SIZE_RX_BUFFER];
static unsigned char gTxBuf[SIZE_TX_BUFFER];

static struct TCircularBuf gRxCBuf;
static struct TCircularBuf gTxCBuf;

static bool gTxing;

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Circular buffer read
//-----------------------------------------------------------------------
static inline int CBufRead (struct TCircularBuf *aCBuf)
{
    unsigned char ch;

    if (aCBuf->Head == aCBuf->Tail)
        return -1;

    ch = aCBuf->Buf[aCBuf->Head];
    aCBuf->Head ++;

    if (aCBuf->Head >= aCBuf->Size)
        aCBuf->Head = 0;

    return ch;
}

//-----------------------------------------------------------------------
// Circular buffer write
//-----------------------------------------------------------------------
static inline int CBufWrite (struct TCircularBuf *aCBuf, unsigned char ch)
{
    int next_tail;

    next_tail = aCBuf->Tail + 1;
    if (next_tail >= aCBuf->Size)
        next_tail = 0;

    if (next_tail == aCBuf->Head)
        return -1;

    aCBuf->Buf[aCBuf->Tail] = ch;
    aCBuf->Tail = next_tail;
  
    return 0;
}

//-----------------------------------------------------------------------
// TX - Interrupt Handler
//-----------------------------------------------------------------------
static void UART0_TX(void)
{
    int i;
    i = CBufRead (&gTxCBuf);
    if (i < 0)
    {
        gTxing = false;
    }
    else
    {
        USART1_DR = (unsigned long)i;
    }
}

//-----------------------------------------------------------------------
// RX - Interrupt Handler
//-----------------------------------------------------------------------
static void UART0_RX(unsigned char rxchar)
{
    CBufWrite (&gRxCBuf, rxchar);
}

//-----------------------------------------------------------------------
//UART0 Interrupt Handler
//-----------------------------------------------------------------------
void USART1_IRQHandler (void)
{
    unsigned long sr;


    sr = USART1_SR;
    
    if (sr & (1 << 6))
    {
        // Tx data reg empty
        UART0_TX();

        //
        USART1_SR &= (~(1 << 6));
    }
    else
    {
        // Read data reg not empty
        UART0_RX(USART1_DR);
    }

    //
    NVIC_ClrPend (NVIC_USART1);
}

//-----------------------------------------------------------------------
// UART Interrupt Setup
//-----------------------------------------------------------------------
void Uart1Init(void)
{
    //Init. TX Buffer
    gTxCBuf.Buf = gTxBuf;
    gTxCBuf.Size = SIZE_TX_BUFFER;
    gTxCBuf.Head = 0;
    gTxCBuf.Tail = 0;
  
    //Init. RX Buffer
    gRxCBuf.Buf = gRxBuf;
    gRxCBuf.Size = SIZE_RX_BUFFER;
    gRxCBuf.Head = 0;
    gRxCBuf.Tail = 0;
  
    gTxing = false;
  
    // No IO Remap
    AFIO_MAPR &= (~(0x01 << 2));

    // Release RESET and enable clock for USART1
    RCC_APB2RSTR &= (~(1 << 14));
    RCC_APB2ENR  |=   (1 << 14);

    // No IO Remap
    AFIO_MAPR &= (~(0x01 << 2));

    // Setup USART
    USART1_BRR  = VALUE_BRR;
    USART1_GTPR = 1;
    USART1_CR1  = (1 << 6) | (1 << 5) | (1 << 3) | (1 << 2);   // 8 bit, no parity
    USART1_CR2  = 0x0000;   // 1 Stop bit
    USART1_CR3  = 0x0000;   // No CTS/RTS

    // Enable USART
    USART1_CR1 |= 0x2000;

    // Enable Interrupt
    NVIC_IntEnable (NVIC_USART1);
}

//-----------------------------------------------------------------------
// Get Rxed data
//-----------------------------------------------------------------------
int Uart1Read (unsigned char *aDest, int aLen)
{
    int i;
    int ch;
    
    i = 0;
    for (; i < aLen; i ++)
    {
        NVIC_IntDisable (NVIC_USART1);
        ch = CBufRead (&gRxCBuf);
        NVIC_IntEnable (NVIC_USART1);
        if (ch < 0)
            break;
        *aDest = (unsigned char)ch;
        aDest ++;
    }

    return i;
}

//-----------------------------------------------------------------------
// Tx data
//-----------------------------------------------------------------------
int Uart1Write (unsigned char *aSrc, int aLen)
{
    int i;
    int ch;

    i = 0;
    for (; i < aLen; i ++)
    {
        NVIC_IntDisable (NVIC_USART1);
        ch = CBufWrite (&gTxCBuf, *aSrc);
        NVIC_IntEnable (NVIC_USART1);
        if (ch < 0)
            break;
        aSrc ++;
    }

    //
    if (gTxing == false)
    {
        gTxing = true;
    }

    // Force data send if nothing is txing
    if (USART1_SR & (1 << 7))
    {
        ch = CBufRead (&gTxCBuf);
        if (ch >= 0)
            USART1_DR  = (unsigned char) ch;
        else
            gTxing = false;
    }
    return i;
}

//-----------------------------------------------------------------------
// Is Txing?
//-----------------------------------------------------------------------
bool Uart1Txing (void)
{
    return gTxing;
}
