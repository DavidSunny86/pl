#ifndef __PL_VECTOR4_H__
#define __PL_VECTOR4_H__

#include <iostream>
#include <cstdlib>
#include <cfloat>
#include <cstdio>
#include <cmath>

#include "pl.h"

class plVector4 
{
	public:

		PLfloat x, y, z, w;

		plVector4();
		plVector4( PLfloat xx, PLfloat yy, PLfloat zz, PLfloat ww );
		plVector4( PLchar *string );  
		plVector4( plString str );

		PLbool 	  operator == (const plVector4 &p) const;
		PLbool 	  operator != (const plVector4 &p) const;
		plVector4 operator +  (const plVector4 &p) const;
		plVector4 operator -  (const plVector4 &p) const;
		PLfloat   operator *  (const plVector4 &p) const;
        
		plVector4 operator-() const;
		
		plVector4 normalize() const;
		PLfloat length() const;
		PLfloat squaredLength() const;
  
};


// Scalar/plVector4 multiplication

plVector4 operator * ( PLfloat k, const plVector4 &p );

// I/O operators

std::ostream& operator << ( std::ostream& stream, const plVector4 &p );
std::istream& operator >> ( std::istream& stream, plVector4 &p );


#endif
