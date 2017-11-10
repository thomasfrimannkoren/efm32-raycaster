//
//  q24.8.h
//  3D-testing
//
//  Created by Thomas Koren on 07.11.2017.
//  Copyright © 2017 Thomas Koren. All rights reserved.
//

#ifndef __q24_8_h
#define __q24_8_h

#include <stdint.h>

#define Q24D8_MAX INT32_MAX
#define Q24D8_MIN INT32_MIN

typedef int32_t q24d8;

static inline q24d8 multq( q24d8 a, q24d8 b )
{
    
    uint32_t ret;
    
    ret = a * b;
    ret >>= 8;
    
    return ret;
    
}

static inline q24d8 divq( q24d8 a, q24d8 b )
{
    
    uint32_t ret;
    
    ret   = a / b;
    ret <<= 8;
    
    return ret;
    
}

static inline float qtof( q24d8 a )
{
    
    return ( (float)a )/256.f;
    
}

static inline q24d8 ftoq( float a )
{
    
    return (uint32_t)( a * 256.f );
    
}

static inline int32_t qtoi( q24d8 a )
{
    
    /* To round up to closest integer */
    a  += 128;
    a >>= 8;
    
    return a;
    
}

static inline q24d8 itoq( int32_t a )
{
    
    return a << 8;
    
}

static inline q24d8 floorq( int32_t a )
{

    return a >> 8;

}


/**
* \brief    Fast Square root algorithm
*
* Fractional parts of the answer are discarded. That is:
*      - SquareRoot(3) --> 1
*      - SquareRoot(4) --> 2
*      - SquareRoot(5) --> 2
*      - SquareRoot(8) --> 2
*      - SquareRoot(9) --> 3
*
* \param[in] a - unsigned integer for which to find the square root
*
* \return Integer square root of the input value.
*/
static inline q24d8 sqrtq( q24d8 a )
{
    uint32_t op  = a << 8;
    uint32_t res = 0;
    uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type
    
    
    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }
    
    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

static inline q24d8 highSquare( q24d8 a )
{
    
    a >>= 4;
    a  *= a;
    
    return a;
    
}


#endif /*__q24_8_h*/
