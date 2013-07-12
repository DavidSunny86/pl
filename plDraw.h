#ifndef __PL_DRAW_H__
#define __PL_DRAW_H__

#include "plCommon.h"
#include "plMesh.h"

class plDraw
{
    public:

        static void arrow    ( const plVector3 &origin, const plVector3 &direction, PLfloat length = PL_HANDLE_LENGTH, PLfloat scale = 1.0f);
        static void arrow    ( const plVector3 &direction, PLfloat length = PL_HANDLE_LENGTH, PLfloat scale = 1.0f);
        static void knob     ( const plVector3 &origin, const plVector3 &direction, PLfloat length = PL_HANDLE_LENGTH, PLfloat scale = 1.0f);
        static void knob     ( const plVector3 &direction, PLfloat length = PL_HANDLE_LENGTH, PLfloat scale = 1.0f);
        static void sphere   ( float radius);
        static void sphere   ( const plVector3 &origin, float radius);
        static void cylinder ( float radius, float height); 
        static void disk     ( const plVector3 &origin, float scale );
        static void disk     ( float scale );

    private:

        plDraw() {};

};

#endif
