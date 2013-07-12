#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "plCommon.h"
#include "plMath.h"
#include "plSeq.h"
#include "plVector3.h"
#include "plRenderable.h"
#include "plEditable.h"
#include "plPickingTexture.h"
#include "plBoneAndCartilage.h"
#include "plMesh.h"
#include "plDraw.h"

class plBoundary : public plRenderable,
                   public plEditable
{
    public:

        plBoundary();       
          
        PLuint size() const;

        const plSeq<plVector3> &points () const { return _points;  } 
        const plSeq<plVector3> &normals() const { return _normals; }
        
        const plVector3        &points ( PLuint index ) const { return _points[index];  }
        const plVector3        &normals( PLuint index ) const { return _normals[index]; }

        const plBoneAndCartilage &model() const { return *_model; }

        plVector3 getAverageNormal() const;

        virtual void   toggleVisibility();        
        virtual void   readFromCSV( const plSeq<plString> &row, const plBoneAndCartilage &model );

        virtual PLuint addPointAndNormal   (const plVector3 &point, const plVector3 &normal);
        virtual void   movePointAndNormal  ( PLuint index, const plVector3 &point, const plVector3 &normal);
        virtual void   removePointAndNormal( PLuint index );
        
        virtual void   draw() const;     
        
        friend std::ostream& operator << ( std::ostream& out, const plBoundary &b );

    protected:
           
        const plBoneAndCartilage *_model;   
           
        plSeq<plVector3> _points;       // always in counterclockwise direction
        plSeq<plVector3> _normals;   
        plMesh           _mesh;    
        PLbool           _showWalls;    
        
        void             _setColour() const;    
        void             _updateMesh(); 
        
};

std::ostream& operator << ( std::ostream& out, const plBoundary &b );

#endif

