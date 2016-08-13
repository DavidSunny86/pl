#ifndef PL_SCAN_POINT_READER_H
#define PL_SCAN_POINT_READER_H

#include "plVector3.h"

#include "plString.h"
#include <fstream>
//#include "plRenderable.h"

class plScanPointData //: public plRenderable
{
public:
  // attributes
    plVector3 point;
    plVector3 normal;
    PLfloat   radius;

  // members
    plScanPointData();
    plScanPointData(const plVector3&, const plVector3&, PLfloat);

    PLbool draw() const;
};

std::ostream& operator << ( std::ostream &stream, const plScanPointData &p );

namespace plScanPointReader
{
    std::vector<plScanPointData> readPoints(const plString&);
}

#endif // PLSCANPOINTREADER_H
