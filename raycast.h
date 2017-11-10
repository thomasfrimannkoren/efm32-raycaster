#ifndef __RAYCAST_H_
#define __RAYCAST_H_

#include "q24d8.h"
#include "trig_table.h"
#include "frame.h"


typedef struct _RC_Viewport {
	int32_t  height;
	int32_t  width;
	q24d8    wallXVp;
	Angle    fov;
} RC_Viewport;

typedef struct _RC_Coordinate {
	q24d8 x;
	q24d8 y;
} RC_Coordinate;

typedef struct _RC_Object {
	RC_Coordinate position;
	Angle direction;
} RC_Object;

typedef uint8_t RC_Map[32][32];

typedef enum{DIR_RIGHT, DIR_UP, DIR_LEFT, DIR_DOWN} RC_Direction;

void RC_initViewport( RC_Viewport *vp,  int32_t height, int32_t width, Angle fov, uint32_t wallHeight );
void RC_renderBackground( RC_Object *p, RC_Viewport *vp, RC_Map map, RC_Map colorMap, FRAME_Frame *frame  );


#endif /*__RAYCAST_H_*/
