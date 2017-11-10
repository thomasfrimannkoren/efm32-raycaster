#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"

#include "bsp.h"
#include "bspconfig.h"

#include "frame.h"
#include "display.h"

#include "trig_table.h"
#include "raycast.h"
#include "q24d8.h"
#include "maps.h"

#define MIN(x, y) ( x < y ? x : y )

static const EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_STK_DEFAULT;
static       FRAME_Frame          frame;
static RC_Viewport vp;
static RC_Object player = {
		.position.x = 512,
		.position.y = 1152,
		.direction  = ANGLE_270,
};


static void clockInit( void );


int main( void )
{

	/* Chip errata */
	CHIP_Init();

	clockInit();

	DISP_init();

	FRAME_frameInit( &frame );



	RC_initViewport( &vp, 128, 128, ANGLE_135, 256 );

	while ( 1 ) {
		RC_renderBackground( &player, &vp, &map, &color, &frame );
		DISP_displayFrame( &frame );
		player.direction = angleAdd( player.direction, 2048/512 );
	}



	while ( 1 );

}

static void clockInit( void )
{

	CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;
	CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_STK_DEFAULT;

	lfxoInit.ctune = 0x46;

	EMU_DCDCInit( &dcdcInit );
	CMU_HFXOInit( &hfxoInit );
	CMU_LFXOInit( &lfxoInit );

	/* Switch HFCLK to HFXO and disable HFRCO */
	CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );
	CMU_OscillatorEnable( cmuOsc_HFRCO, false, false );

	/* Switch LFCLK to LFXO and disable HFRCO */
	CMU_ClockSelectSet( cmuClock_LFA, cmuSelect_LFXO );
	CMU_ClockSelectSet( cmuClock_LFB, cmuSelect_LFXO );
	CMU_ClockSelectSet( cmuClock_LFE, cmuSelect_LFXO );
	CMU_OscillatorEnable( cmuOsc_LFRCO, false, false );

	return;

}

static void psychPattern( void )
{
	int k = 0;
	int jiOffset = 16;
	int colorOffset = 0;
	uint8_t color = 0;
	while ( 1 ) {
		jiOffset += 2;
		if ( jiOffset > 16 ) {
			jiOffset = 1;
			colorOffset++;
			if ( colorOffset > 7 ) {
				colorOffset = 0;
			}
		}

		for ( int i = 0; i < 128; ++i ) {
			for ( int j = 0; j < 128; ++j ) {
				if ( k > 7 ) {
					k = 0;
				}

				color = MIN( (j+jiOffset)/16, (i+jiOffset)/16) + colorOffset;
				if ( color > 7 ) {
					color -= 8;
				}
				FRAME_setPixel( &frame, j, i, color );
				k++;
			}
		}

		DISP_displayFrame( &frame );

	}
}
