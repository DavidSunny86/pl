#ifndef PLMESHALGORITHM_H
#define PLMESHALGORITHM_H

#include "plVector3.h"
#include "plPolygon.h"
#include "plTriangle.h"
#include "plMeshConnectivityData.h"

class plMeshAlgorithm
{

  public:

    plMeshAlgorithm();

  protected:
    PLfloat _epsilon;

    // the connectivity data
    plMeshConnectivityData _data;

    // all the rest of these functions return false if an error is detected. At this point the intersector should abort with a message of some kind.
    const plMeshConnectivityDataVert* _findVert( const plVector3& vertex, PLuint verbose, PLuint depth=0 );

    PLbool _splitEdgeOnVect(const plMeshConnectivityDataEdge* edgeAB, const plMeshConnectivityDataVert* vertN, PLuint verbose, PLuint depth=0);
    PLbool _splitFaceOnVect(PLuint faceIndex, const plVector3& vertex, PLuint verbose, PLuint depth=0);

    PLbool _importTriSeq(const plSeq<plTriangle>& tris, PLuint verbose, PLuint depth=0);
    PLbool _exportTriSeq(      plSeq<plTriangle>& tris, PLuint verbose, PLuint depth=0);

    PLbool _checkArraySizes       ( PLuint verbose, PLuint depth=0 );
    PLbool _checkNoDuplicates     ( PLuint verbose, PLuint depth=0 );
    PLbool _checkBidirectional    ( PLuint verbose, PLuint depth=0 );
    PLbool _checkNoSliverTriangles( PLuint verbose, PLuint depth=0 );
    PLbool _checkForAllErrors     ( PLuint verbose, PLuint depth=0 );

    void   _reportSizes( PLuint verbose, PLuint depth=0 );

};

#endif // PLMESHCONNECTIVITY_H
