#ifndef __FRAME_H_
#define __FRAME_H_

#include <stdint.h>
#if defined ( __ICCARM__ )
   #define PACKED_STRUCT __packed struct
#elif defined ( __clang__ )
   #define PACKED_STRUCT struct __attribute__ ((__packed__))
#elif defined ( __GNUC__ )
   #define PACKED_STRUCT struct __attribute__ ((gcc_struct,__packed__))
#endif

#define FRAME_MODE_UPDATE_DATA       ( 0x01 )
#define FRAME_MODE_MAINTAIN_DATA     ( 0x00 )
#define FRAME_MODE_FRAME_INVERT      ( 0x02 )
#define FRAME_MODE_CLEAR             ( 0x04 )

#define FRAME_COLOR_BLACK            ( 0x00 << 5 )
#define FRAME_COLOR_BLUE             ( 0x01 << 5 )
#define FRAME_COLOR_GREEN            ( 0x02 << 5 )
#define FRAME_COLOR_CYAN             ( 0x03 << 5 )
#define FRAME_COLOR_RED              ( 0x04 << 5 )
#define FRAME_COLOR_MAGENTA          ( 0x05 << 5 )
#define FRAME_COLOR_YELLOW           ( 0x06 << 5 )
#define FRAME_COLOR_WHITE            ( 0x07 << 5 )

typedef PACKED_STRUCT _FRAME_Line {
   uint8_t lineAddr;
   uint8_t data[48];
   uint8_t dummy;    /** Preferably Zero */
} FRAME_Line;

typedef PACKED_STRUCT _FRAME_Frame {
   uint8_t mode;
   FRAME_Line lineData[128];
   uint8_t dummy;   /** Preferably zero */
} FRAME_Frame;

void FRAME_frameInit( FRAME_Frame *frame );

static inline void FRAME_setPixel( FRAME_Frame *frame, uint32_t x, uint32_t y, uint8_t color )
{

   uint32_t xBitIndex;
   uint32_t xByteIdx;
   uint32_t xBitShift;
   uint16_t dualByte;

   /* Calculate Indexes */
   xBitIndex = x * 3;
   xByteIdx  = xBitIndex / 8;
   xBitShift = xBitIndex - ( xByteIdx * 8 );

   /* Extract two bytes. In worst case LSB will be the dummy byte
    * With the current struct this memory access is safe */
   dualByte  = ( (uint16_t)frame->lineData[y].data[xByteIdx] );
   dualByte |= ( (uint16_t)frame->lineData[y].data[xByteIdx + 1] ) << 8;

   /* Clearing and writing */
   dualByte &= ~(0x7 << xBitShift);
   dualByte |= color << xBitShift;

   /* Storing it */
   frame->lineData[y].data[xByteIdx]     = (uint8_t)( dualByte & 0xFF );
   frame->lineData[y].data[xByteIdx + 1] = (uint8_t)( dualByte >> 8 );

   return;

}



#endif /*__FRAME_H_*/
