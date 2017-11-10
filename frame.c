#include "frame.h"

#include "em_device.h"


void FRAME_frameInit( FRAME_Frame *frame )
{

	uint32_t addr;

	frame->mode = FRAME_MODE_UPDATE_DATA;

	for ( addr = 0; addr < 128; ++addr ) {
		frame->lineData[addr].lineAddr = addr+1;
	}

	return;

}
