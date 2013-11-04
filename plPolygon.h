#ifndef __PL_POLYGON_H__
#define __PL_POLYGON_H__

#include "plCommon.h"

#include "plVector3.h"


class plEdge
{

  public:

    plVector3 pt1;
    plVector3 pt2;

    plEdge ();
    plEdge (const plVector3 &v1, const plVector3 &v2);

    PLbool    contains  (const plVector3& point, const PLfloat &epsilon = PL_EPSILON) const;
    plVector3 otherPt   (const plVector3 &point, const PLfloat &epsilon = PL_EPSILON) const;
    PLbool    equals    (const plEdge& e2,       const PLfloat &epsilon = PL_EPSILON) const; // flexible epsilon
    PLbool    operator==(const plEdge& e2) const; // epsilon is absolute zero
    plEdge&   operator= (const plEdge& other);

};


class plPolygon 
{

    public:
    
        std::vector<plVector3> points;
        plVector3        normal;

};

std::ostream& operator << ( std::ostream &stream, const plEdge &e );
std::ostream& operator << ( std::ostream &stream, const plPolygon &p );

#endif
