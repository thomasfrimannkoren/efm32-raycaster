#include "display.h"

#include <stdbool.h>

#include "em_device.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "em_rtcc.h"
#include "em_prs.h"

//#include "display_config.h"

#define DISP_LDMA_CH               ( 0 )

#define DISP_ENABLE_PIN            ( 9 )
#define DISP_ENABLE_PORT           ( gpioPortA )
#define DISP_DATA_PIN              ( 14 )
#define DISP_DATA_PORT             ( gpioPortA )
#define DISP_CLK_PIN               ( 15 )
#define DISP_CLK_PORT              ( gpioPortC )
#define DISP_CS_PIN                ( 14 )
#define DISP_CS_PORT               ( gpioPortC )
#define DISP_POL_PIN               ( 11 )
#define DISP_POL_PORT              ( gpioPortA )


static const LDMA_TransferCfg_t usart1TxTransfer = LDMA_TRANSFER_CFG_PERIPHERAL( ldmaPeripheralSignal_USART1_TXEMPTY );
static LDMA_Descriptor_t txDescriptors[4] = {
   LDMA_DESCRIPTOR_LINKREL_M2P_BYTE( 0, &(USART1->TXDATA), 0, 1 ),
   LDMA_DESCRIPTOR_LINKREL_M2P_BYTE( 0, &(USART1->TXDATA), 0, 1 ),
   LDMA_DESCRIPTOR_LINKREL_M2P_BYTE( 0, &(USART1->TXDATA), 0, 1 ),
   LDMA_DESCRIPTOR_SINGLE_M2P_BYTE( 0, &(USART1->TXDATA), 0 ),
};
static volatile uint8_t  ldmaBusy     = false;
static int               ldmaErrors   = 0;
 
/******************************************************************************/
/*                                                                            */ 
/* Local function declaration                                                 */
/*                                                                            */
/******************************************************************************/
static void rtccInit ( void );
static void spiInit  ( void );
static void dmaInit  ( void );

/******************************************************************************/
/*                                                                            */ 
/* Interrupt function implementation                                          */
/*                                                                            */
/******************************************************************************/
void LDMA_IRQHandler( void )
{

   uint32_t flags;

   flags = LDMA->IF;
   LDMA->IFC = flags;

   if ( flags & LDMA_IF_ERROR ) {
      ldmaErrors++;
   }

   if ( flags & ( 0x1 << DISP_LDMA_CH ) ) {
      ldmaBusy--;
      if ( ldmaBusy == 0 ) {
    	  USART_Enable( USART1, usartDisable );
      }
   }

   return;

}

void RTCC_IRQHandler( void )
{

   uint32_t flags;

   flags = RTCC->IF;
   RTCC->IFC = flags;

   if ( flags & RTCC_IF_CNTTICK ) {
      GPIO_PinOutToggle( gpioPortH, 10 );
   }

}

/******************************************************************************/
/*                                                                            */ 
/* Global function implementation                                             */
/*                                                                            */
/******************************************************************************/

/* Initialize display and shit */
void DISP_init( void )
{

   CMU_ClockEnable( cmuClock_GPIO, true );

   GPIO_PinModeSet( DISP_ENABLE_PORT, DISP_ENABLE_PIN, gpioModePushPull, 0 );
   GPIO_PinModeSet( DISP_DATA_PORT, DISP_DATA_PIN, gpioModePushPull, 0 );
   GPIO_PinModeSet( DISP_CLK_PORT, DISP_CLK_PIN, gpioModePushPull, 0 );
   GPIO_PinModeSet( DISP_CS_PORT, DISP_CS_PIN, gpioModePushPull, 0 );
   GPIO_PinModeSet( DISP_POL_PORT, DISP_POL_PIN, gpioModePushPull, 0 );
   GPIO_PinModeSet( gpioPortH, 10, gpioModePushPull, 0 );
   GPIO_PinModeSet( gpioPortH, 13, gpioModePushPull, 0 );

   rtccInit();

   spiInit();
   
   dmaInit();

   GPIO_PinOutSet( DISP_ENABLE_PORT, DISP_ENABLE_PIN );

   return;

}

/* Push a frame out to the display */
void DISP_displayFrame( FRAME_Frame *frame )
{

   uint32_t frameSize;
   uint8_t *frameData;

   GPIO_PinOutToggle( gpioPortH, 13 );

   if ( ldmaBusy ) {
      while ( ldmaBusy );
   }

   frameSize = sizeof( FRAME_Frame );
   frameData = (uint8_t *) frame;
   txDescriptors[0].xfer.xferCnt = 2047;
   txDescriptors[0].xfer.srcAddr = &(frameData[0]);
   txDescriptors[1].xfer.xferCnt = 2047;
   txDescriptors[1].xfer.srcAddr = &(frameData[2048*1]);
   txDescriptors[2].xfer.xferCnt = 2047;
   txDescriptors[2].xfer.srcAddr = &(frameData[2048*2]);
   txDescriptors[3].xfer.xferCnt = frameSize - 2048*3 - 1;
   txDescriptors[3].xfer.srcAddr = &(frameData[2048*3]);

   ldmaBusy = 4;
   LDMA_StartTransfer( DISP_LDMA_CH, &usart1TxTransfer, txDescriptors );

   USART_Enable( USART1, usartEnableTx );

   return;

}


/******************************************************************************/
/*                                                                            */ 
/* Local function implementation                                              */
/*                                                                            */
/******************************************************************************/

static void rtccInit( void )
{

   RTCC_Init_TypeDef rtccInit = RTCC_INIT_DEFAULT;
   RTCC_CCChConf_TypeDef ccConf = RTCC_CH_INIT_COMPARE_DEFAULT;

   CMU_ClockEnable( cmuClock_RTCC, true );
   CMU_ClockEnable( cmuClock_HFLE, true );

   rtccInit.precntWrapOnCCV0 = true;
   rtccInit.prescMode        = rtccCntTickCCV0Match;
   //rtccInit.enable           = false;

   ccConf.compBase           = rtccCompBasePreCnt;
   ccConf.compMatchOutAction = rtccCompMatchOutActionToggle;

   RTCC_Init( &rtccInit );
   RTCC_ChannelInit( 0, &ccConf );
   RTCC_ChannelCCVSet( 0, 4096 );
   NVIC_EnableIRQ( RTCC_IRQn );
   RTCC_IntEnable( RTCC_IEN_CNTTICK );
   RTCC_Enable( true );

   while ( RTCC->SYNCBUSY );

   /* PRS Setup */
   CMU_ClockEnable( cmuClock_PRS, true );

   PRS_SourceAsyncSignalSet( 11, PRS_CH_CTRL_SOURCESEL_RTCC, PRS_CH_CTRL_SIGSEL_RTCCCCV0 );

   PRS->ROUTELOC2 = ( PRS->ROUTELOC2 & ~_PRS_ROUTELOC2_CH11LOC_MASK ) | PRS_ROUTELOC2_CH11LOC_LOC0;
   PRS->ROUTEPEN  = PRS_ROUTEPEN_CH11PEN;

}

static void spiInit( void )
{

   USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

   CMU_ClockEnable( cmuClock_USART1, true );

   usartInit.autoCsEnable = true;
   usartInit.baudrate     = 2500000;
   usartInit.msbf         = false;
   usartInit.enable       = usartDisable;
   usartInit.autoCsSetup  = 3;
   usartInit.autoCsHold   = 7;


   USART_InitSync( USART1, &usartInit );

   USART1->CTRL |= USART_CTRL_CSINV;

   USART1->ROUTELOC0 = USART_ROUTELOC0_CSLOC_LOC3
      | USART_ROUTELOC0_TXLOC_LOC6
      | USART_ROUTELOC0_CLKLOC_LOC3;

   USART1->ROUTEPEN = USART_ROUTEPEN_TXPEN
      | USART_ROUTEPEN_CLKPEN
      | USART_ROUTEPEN_CSPEN;

	return;

}

static void dmaInit( void )
{

   LDMA_Init_t ldmaInit = LDMA_INIT_DEFAULT;

   CMU_ClockEnable( cmuClock_LDMA, true );

   LDMA_Init( &ldmaInit );

   NVIC_EnableIRQ( LDMA_IRQn );
   LDMA_IntEnable( LDMA_IF_ERROR | ( 0x1 << DISP_LDMA_CH ) ); 

}


