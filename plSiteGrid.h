#ifndef _PL_SITE_GRID_H_
#define _PL_SITE_GRID_H_

#include "plCommon.h"
#include "plSeq.h"
#include "plVector4.h"
#include "plTriangle.h"
#include "plBoundary.h"
#include "plMeshCutter.h"

class plPointAndNormal
{

    public:
    
        plVector3 point;
        plVector3 normal;
    
        plPointAndNormal( const plVector3 &p, const plVector3 &n)
            : point(p), normal(n)
        {}

        PLbool operator < (const plPointAndNormal &pn) const
        {       
            return point.x < pn.point.x || point.y < pn.point.y || point.z < pn.point.z;
        }
};


class plSiteGrid
{
    public:
    
        plSiteGrid();
        plSiteGrid( const plSeq<plTriangle> &triangles, const plBoundary &boundary );
        
        PLuint     gridSize()   const { return _points.size(); }
        PLuint     meshSize()   const { return _triangles.size(); }
        PLuint     perimSize()  const { return _perimeter.size()/2; }
        PLuint     dataSize()   const { return _points.size()*2 + _triangles.size()*4 + _perimeter.size(); }
        PLfloat    area()       const { return _area; } 
        plVector3  normal()     const { return _normal; };
        
        PLuint getFullSSBO() const;
        PLuint getGridSSBO() const;
        PLuint getMeshSSBO() const;
        
        plIntersection rayIntersect( const plVector3 &rayOrigin, const plVector3 &rayDirection, PLbool ignoreBehindRay = false, PLbool backFaceCull = false ) const;
        
        const plTriangle&  triangles ( PLuint index ) const { return _triangles[index]; }     
        const plVector4&   points    ( PLuint index ) const { return _points[index];    }
        const plVector4&   normals   ( PLuint index ) const { return _normals[index];   }
        const plVector4&   perimeter ( PLuint index ) const { return _perimeter[index]; }
          
    private:
        
        plSeq<plTriangle> _triangles;
        plSeq<plVector4>  _points; 
        plSeq<plVector4>  _normals;
        plSeq<plVector4>  _perimeter;
        PLfloat           _area;
        plVector3         _normal;

        plVector3 _calcSmoothNormal( const plVector3 &centroid, const plVector3 &up, PLfloat radius );
        void _generateGridPoints();  
        void _generatePerimeterPoints( const plBoundary &boundary );
        void _calcArea          ();
        void _calcNormal        ();
        
};

PLuint getGroupFullSSBO( const plSeq<plSiteGrid> &grids );
PLuint getGroupGridSSBO( const plSeq<plSiteGrid> &grids );




#endif
