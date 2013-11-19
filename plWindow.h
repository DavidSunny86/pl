#ifndef PL_WINDOW_H
#define PL_WINDOW_H

#include "plCommon.h"
#include "plMatrix44.h"
#include "plRenderingPipeline.h"
#include "plRenderer.h"

namespace plWindow
{
    PLuint    width            ();
    PLuint    height           ();    
    PLuint    viewportWidth    ();
    PLuint    viewportHeight   ();
    PLint     windowToViewportX( PLint x );
    PLint     windowToViewportY( PLint y );
    
    void      reshape          ( PLuint width, PLuint height );
    plVector3 mouseToWorld     ( PLint x, PLint y, PLint z );
    plVector3 worldToScreen    ( PLfloat x, PLfloat y, PLfloat z );
    void      cameraToMouseRay ( plVector3 &rayOrigin, plVector3 &rayDirection, PLint x, PLint y ); 				
}

#endif
