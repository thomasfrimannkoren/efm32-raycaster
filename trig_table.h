#ifndef __TRIG_TABLE_H_
#define __TRIG_TABLE_H_

#include <stdint.h>
#include "q24d8.h"

#define __CLZ __builtin_clz

#define TRIG_TABLE_SIZE       ( 2048 )

#define ANGLE_MAX             ( 2048 )
#define ANGLE_45              ( 256 )
#define ANGLE_90              ( 512 )
#define ANGLE_135             ( 768 )
#define ANGLE_180             ( 1024 )
#define ANGLE_225             ( 1280 )
#define ANGLE_270             ( 1536 )
#define ANGLE_315             ( 1792 )

typedef uint32_t Angle;
typedef int32_t  Trigval;

extern const int32_t cosTable[TRIG_TABLE_SIZE];
extern const int32_t tanTable[TRIG_TABLE_SIZE];

/* Angles are defined with 0 as 0 and 2048 as 360 */

static inline Angle angleInc( Angle a ) 
{

   a++;
   a &= 0x7FF;

   return a;

}

static inline Angle angleDec ( Angle a ) 
{

   a--;
   a &= 0x7FF;

   return a;

}

static inline Angle angleAdd ( Angle a, Angle b )
{

   a = a + b;
   a &= 0x7FF;;

   return a;

}

static inline Angle angleSub ( Angle a, Angle b )
{

   a = a - b;
   a &= 0x7FF;;

   return a;

}


#define getCosVal( a ) cosTable[a]
#define getTanVal( a ) tanTable[a]
#define getSinVal( a ) cosTable[ ( a - ANGLE_90 ) & 0x7FF ]

static inline q24d8 trigMult( q24d8 q, Trigval a )
{

   if ( q > 0xFFFFFF ) {
      a >>= 8;
      q *= a;
      q >>= 8;
   }
   else {
      q *= a;
      q >>= 16;
   }

   return q;

}


static inline q24d8 trigDiv( q24d8 q, Trigval a )
{

   uint32_t preshift;
   uint32_t postshift;
      
   preshift = __CLZ( q );
   if ( preshift > 16 ) {
      preshift = 16;
   }

   postshift = 16 - preshift;

   q <<= preshift;
   q  /= a;
   q <<= postshift;

   return q;

}




#endif /*__TRIG_TABLE_H_*/
