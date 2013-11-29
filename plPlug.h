#ifndef PL_PLUG_H
#define PL_PLUG_H

#include "plCommon.h"
#include "plMeshSpecific.h"
#include "plVector3.h"
#include "plBoneAndCartilage.h"
#include "plTransform.h"

// TODO: FIXED DRAGGING TO INCLUDE A ROTATION OFFSET, ROTATION OFFSET CAPS AT n DEGREES AND IS MAINTAINED

class plPlug : public plMeshSpecific
{
    public:

        plPlug( PLuint type );
        plPlug( const plMesh& mesh, PLuint type, const plTransform& surfaceTransform, const plTransform& rotationalOffset );

        const plTransform& transform() const; 
        const plTransform& rotation() const;        
        plMatrix44         matrix() const;
        plTransform        finalTransform() const; 

        void move( const plVector3& origin, const plVector3& y );

    private:
        
        PLuint      _type;        
        plTransform _surfaceTransform;
        plTransform _rotationalOffset;       

};


#endif
