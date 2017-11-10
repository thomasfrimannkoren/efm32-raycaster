#include "raycast.h"

#include <stdlib.h>

#include "q24d8.h"
#include "frame.h"
#include "trig_table.h"

#define MIN(X, Y) (((X) <= (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) >= (Y)) ? (X) : (Y))

#define checkIntersection( intersection, map ) \
   ( map[ floorq(intersection.y) ][ floorq(intersection.x) ] )

#define nextIntersection( intersection, delta ) do { \
   intersection.x += delta.x; \
   intersection.x  = MIN( MAX( intersection.x, 0 ), Q24D8_MAX ); \
   intersection.y += delta.y; \
   intersection.y  = MIN( MAX( intersection.y, 0 ), Q24D8_MAX ); \
   } while ( 0 )

#define FISHEYE_CORRECTION  1

uint8_t colHeights[128];
uint8_t colColors[128];
uint8_t roofColor = 0x6;
uint8_t floorColor = 0x3;

void drawColumns( FRAME_Frame *frame, RC_Viewport *vp, uint8_t *colHeight, uint8_t *colColor );
void calculateWallHeight( Angle angle, RC_Object *player, RC_Viewport *vp, RC_Map map, RC_Map colorMap, uint8_t *colHeight, uint8_t *colColor );

void RC_initViewport( RC_Viewport *vp,  int32_t height, int32_t width, Angle fov, uint32_t wallHeight )
{

   Trigval tanVal;
   q24d8   distToVp;

   vp->height = height;
   vp->width  = width;
   vp->fov    = fov;

   tanVal = getTanVal( fov / 2 );
   distToVp = trigDiv( itoq( vp->height )  / 2, tanVal );
   vp->wallXVp = qtoi(multq( itoq( wallHeight ), distToVp ));
   //vp->wallXVp = itoq( 100 );

}

void RC_renderBackground( RC_Object *p, RC_Viewport *vp, RC_Map map, RC_Map colorMap, FRAME_Frame *frame  )
{

   uint32_t col;
   uint32_t angleInc;
   uint32_t angle;

   angleInc  = vp->fov / vp->width;
   angle     = vp->fov / 2;
   angle     = angleAdd( angle, p->direction );

   for ( col = 0; col < vp->width; ++col ) {
      calculateWallHeight( angle, p, vp, map, colorMap, &colHeights[col], &colColors[col] );
      angle = angleSub( angle, angleInc );
   }

   drawColumns( frame, vp, colHeights, colColors );

   return;

}



void drawColumns( FRAME_Frame *frame, RC_Viewport *vp, uint8_t *colHeight, uint8_t *colColor )
{

   uint32_t col;
   uint32_t row;
   uint32_t colRoofLimit;
   uint32_t colFloorLimit;
   uint8_t  color;

   for ( col = 0; col < vp->width; ++col ) {
      colRoofLimit   = vp->height/2;
      colRoofLimit  -= colHeight[col]/2;
      colFloorLimit  = vp->height/2;
      colFloorLimit += colHeight[col]/2;
      color          = colColor[ col ];

      for ( row = 0; row < vp->height; ++row ) {
	 if ( row < colRoofLimit ) {
	    /* Pain with roof color */
	    FRAME_setPixel( frame, col, row, roofColor );
	 }
	 else if ( row > colFloorLimit ) {
	    /* Paint with floor color */
	    FRAME_setPixel( frame, col, row, floorColor );
	 }
	 else {
	    /* Paint with wall color */
	    FRAME_setPixel( frame, col, row, color );
	 }
      }
   }

   return;

}

void calculateWallHeight( Angle angle, RC_Object *p, RC_Viewport *vp, RC_Map map, RC_Map colorMap, uint8_t *colHeight, uint8_t *colColor )
{

   uint8_t       color;
   q24d8         height;
   q24d8         xDistance;
   q24d8         yDistance;
   q24d8         distance;
   q24d8         correctionAngle;
   q24d8         cosCorrection;
   Trigval       tanVal;
   Trigval       cosVal;
   Trigval       sinVal;
   RC_Direction  dir;
   RC_Coordinate yIntersection;
   RC_Coordinate xIntersection;
   RC_Coordinate yDelta;
   RC_Coordinate xDelta;

   /* Get trigonometric values */
   tanVal = getTanVal( angle );
   cosVal = getCosVal( angle );
   sinVal = getSinVal( angle );

   /*
    *  Finding box intersections along x-side of boxes and y-side of boxes
    */

   /* Find intersection and deltas */
   if ( angle < ANGLE_MAX/2 ) {
      /* Looking in negative y-direction ( 0 < angle < 180 ) */
      /* First find the intersection by flooring to grid */
      yIntersection.y = ( p->position.y & 0xFFFFFF00 ) - 1;
      yDelta.y        = itoq( -1 );
   }
   else {
      /* Looking in positive y-direction */
      /* Roofing to grid instead of flooring */
      yIntersection.y = ( p->position.y & 0xFFFFFF00 ) + itoq( 1 );
      yDelta.y        = itoq( 1 );
   }
   /* Find the x-intersection along the y-axis */
   yIntersection.x = p->position.x + trigDiv( ( p->position.y - yIntersection.y ), tanVal );
   yIntersection.x = MIN( MAX( yIntersection.x, 0 ), Q24D8_MAX );
   /* Find movement along x-direction for each y-delta */
   yDelta.x        = trigDiv( itoq( 1 ), tanVal );

   /* Now finding intersection along x-axis boxes */
   if ( ( angle > ANGLE_90 ) && ( angle < ANGLE_270 ) ) {
      /* Looking along negative x-direction ( 90 < angle < 270 ) */
      /* Finding first intersection by flooring to grid */
      xIntersection.x = ( p->position.x & 0xFFFFFF00 ) - 1;
      xDelta.x        = itoq( -1 );
   }
   else {
      /* Looking in positive x */
      /* Roofing to grid instead of flooring */
      xIntersection.x = ( p->position.x & 0xFFFFFF00 ) + itoq( 1 );
      xDelta.x        = itoq( 1 );
   }
   /* Now find y-intersection along x-axis */
   xIntersection.y = p->position.y + trigMult( ( p->position.x - xIntersection.x ), tanVal );
   xIntersection.y = MIN( MAX( xIntersection.y, 0 ), Q24D8_MAX );
   xDelta.y        = trigMult( itoq( 1 ), tanVal );



   /*
    *  Finding map intersections along x-side and y-side 
    */

   /* Finding direction of current angle */
   if ( ( angle < ANGLE_45 ) || ( angle > ANGLE_315 ) ) {
      dir = DIR_RIGHT;
   }
   else if ( angle < ANGLE_135 ) {
      dir = DIR_UP;
   }
   else if ( angle < ANGLE_225 ) {
      dir = DIR_LEFT;
   }
   else {
      dir = DIR_DOWN;
   }

   /* Finding y intersection */
   while ( 1 ) {
      if ( !checkIntersection( yIntersection, map ) ) {
	      nextIntersection( yIntersection, yDelta );
      }
      else {
	 switch ( dir ) {
	    case DIR_UP:
	    case DIR_DOWN:
	       yDistance = abs( p->position.y - yIntersection.y );
	       yDistance = trigDiv( yDistance, sinVal );
	       yDistance = abs( yDistance );
	       break;
	    case DIR_RIGHT:
	    case DIR_LEFT:
	    default:
	       yDistance = abs ( p->position.x - yIntersection.x );
	       yDistance = trigDiv( yDistance, cosVal );
	       yDistance = abs( yDistance );
	       break;
	 }
	 break;
      }
   }

   /* Finding x intersection */
   while ( 1 ) {
      if ( !checkIntersection( xIntersection, map ) ) {
	 nextIntersection( xIntersection, xDelta );
      }
      else {
	 switch ( dir ) {
	    case DIR_UP:
	    case DIR_DOWN:
	       xDistance = abs( p->position.y - xIntersection.y );
	       xDistance = trigDiv( xDistance, sinVal );
	       xDistance = abs( xDistance );
	       break;
	    case DIR_RIGHT:
	    case DIR_LEFT:
	    default:
	       xDistance = abs( p->position.x - xIntersection.x );
	       xDistance = trigDiv( xDistance, cosVal );
	       xDistance = abs( xDistance );
	       break;
	 }
	 break;
      }
   }

   /* Min value is actual distance to wall */
   if ( xDistance < yDistance ) {
      distance = xDistance;
      color    = colorMap[ floorq( xIntersection.y ) ][ floorq( xIntersection.x ) ];
   }
   else {
      distance = yDistance;
      color    = colorMap[ floorq( yIntersection.y ) ][ floorq( yIntersection.x ) ];
   }

#if FISHEYE_CORRECTION
   correctionAngle = angleSub( p->direction, angle );
   cosCorrection   = getCosVal( correctionAngle );
   distance         = trigMult( distance, cosCorrection );
#endif 

   *colColor  = color;
   height = divq( vp->wallXVp, distance );
   height = qtoi( MIN( MAX( height, 0 ), itoq(vp->height) ) );
   *colHeight = (uint8_t) height;


   return;

}
