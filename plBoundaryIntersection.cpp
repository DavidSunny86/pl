// offset.cpp


#include "plBoundaryIntersection.h"


// Determine the polygons of a triangle mesh that are inside a boundary.
//
// This is a bit brute force in that it compare each triangle edge to
// each wall of the boundary.  That could eventually be improved by
// finding a single intersection point, then walking along the
// boundary across the triangle mesh.


void findInteriorMesh( plSeq<plPolygon> &triangles, plSeq<plWall> &walls, plSeq<plPolygon> &polygons )

{
  for (PLint i=0; i<triangles.size(); i++)
    triangles[i].processed = false;

  polygons.clear();

  plSeq<plVector3> interiorPoints( 3 * triangles.size() );

  // Collect polygons that intersect the boundary

  for (PLint i=0; i<triangles.size(); i++)
    triangleCutsBoundary( triangles[i], walls, polygons, interiorPoints );

  // Collect other polygons that contain an interior point.  This is very slow.

  for (PLint i=0; i<interiorPoints.size(); i++)
    for (PLint j=0; j<triangles.size(); j++)
      if (!triangles[j].processed)
        for (PLint k=0; k<3; k++)
          if (interiorPoints[i] == triangles[j].ps[k]) {
            polygons.add( triangles[j] );
            triangles[j].processed = true;
            for (PLint l=0; l<3; l++) {
              PLint m;
              for (m=0; m<interiorPoints.size(); m++)
                if (triangles[j].ps[l] == interiorPoints[m])
                  break;
                if (m == interiorPoints.size())
                  interiorPoints.add( triangles[j].ps[l] );
            }
            break;
          }
}



void triangleCutsBoundary( plPolygon &tri,  plSeq<plWall> &walls, plSeq<plPolygon> &polys, plSeq<plVector3> &interiorPoints )

{
  plVector3 ps[3] = { tri.ps[0], tri.ps[1], tri.ps[2] };

  plSeq<plCut> edgeCuts;

  for (PLint i=0; i<walls.size(); i++) {

    // For each triangle edge, see if it crosses walls[i]

    plVector3 intPoint;
    PLfloat  edgeParam, wallParam;
    PLint    intDir;

    if (tri.n * walls[i].n0 > 0) // only consider triangles on same side of mesh as walls
      for (PLint j=0; j<3; j++)
        if (edgeCutsWall( ps[j], ps[(j+1)%3], walls[i], intPoint, edgeParam, wallParam, intDir ))
          edgeCuts.add( plCut( intPoint, j, edgeParam, i, wallParam, intDir ) );
  }

  if (edgeCuts.size() == 0)
    return;

  // Build the polygons that remain inside the wall

  // Sort the cuts

  plSeq<plCut> wallCuts = edgeCuts;

  qsort( &edgeCuts[0], edgeCuts.size(), sizeof(plCut), compareEdgeCuts ); // sort by increasing edge index, then by increasing parameter on each edge
  qsort( &wallCuts[0], wallCuts.size(), sizeof(plCut), compareWallCuts ); // sort by increasing wall index, then by increasing parameter on each wall

  // Bookkeeping to know when to stop

  for (PLint i=0; i<edgeCuts.size(); i++)
    edgeCuts[i].processed = false;

  PLint numCutsLeft = edgeCuts.size();

  while (numCutsLeft > 0) {

    // Build one polygon

    plPolygon poly;

    poly.n = tri.n;

    // Find an initial edge cut at which the triangle edge is going
    // outward through wall.

    PLint edgeCutIndex;
    for (edgeCutIndex=0; edgeCutIndex<edgeCuts.size(); edgeCutIndex++)
      if (!edgeCuts[ edgeCutIndex ].processed && edgeCuts[ edgeCutIndex ].dir == +1)
        break;

    if (edgeCutIndex == edgeCuts.size()) {
      std::cerr << "Error: Couldn't find an initial outgoing triangle edge cut." << std::endl;
      exit(1);
    }

    while (true) {

      // Add this int point

      poly.ps.add( edgeCuts[ edgeCutIndex ].p );
      edgeCuts[ edgeCutIndex ].processed = true;
      numCutsLeft--;

      // Find this intersection point in the list of wall cuts.

      PLint wallCutIndex;
      for (wallCutIndex=0; wallCutIndex<wallCuts.size(); wallCutIndex++)
        if (wallCuts[ wallCutIndex ].p == edgeCuts[ edgeCutIndex ].p)
      break;

      if (wallCutIndex == wallCuts.size()) {
        std::cerr << "Error: An intersection point is missing from the wallCuts list." << std::endl;
        exit(1);
      }

      // Move along the walls from this intersection point to the next intersection point.

      PLint thisWallIndex = wallCuts[ wallCutIndex ].wallIndex;
      PLint nextWallIndex = wallCuts[ (wallCutIndex+1)%wallCuts.size() ].wallIndex;

      while (thisWallIndex != nextWallIndex) {

    // We're not yet re-entering on the same wall, so we need to
    // include some of the wall vertices.

        thisWallIndex = (thisWallIndex+1) % walls.size();

        poly.ps.add( walls[ thisWallIndex ].p0 );
      }

      // Advance to the wall cut at which the triangle edge re-enters the wall.

      wallCutIndex = (wallCutIndex+1)%wallCuts.size();

      // Find this intersection point in the list of edge cuts.

      for (edgeCutIndex=0; edgeCutIndex<edgeCuts.size(); edgeCutIndex++)
        if (edgeCuts[ edgeCutIndex ].p == wallCuts[ wallCutIndex ].p)
          break;

      if (edgeCutIndex == edgeCuts.size()) {
        std::cerr << "Error: An intersection point is missing from the edgeCuts list." << std::endl;
        exit(1);
      }

      if (edgeCuts[ edgeCutIndex ].dir != -1) {
        std::cerr << "Error: Expected to have an incoming triangle edge but an outgoing edge was found." << std::endl;
        exit(1);
      }

      // Add this int point

      poly.ps.add( edgeCuts[ edgeCutIndex ].p );
      edgeCuts[ edgeCutIndex ].processed = true;
      numCutsLeft--;

      // Move along the triangle edges from this intersection point to the next.

      PLint thisEdgeIndex = edgeCuts[ edgeCutIndex ].edgeIndex;
      PLint nextEdgeIndex = edgeCuts[ (edgeCutIndex+1)%edgeCuts.size() ].edgeIndex;

      while (thisEdgeIndex != nextEdgeIndex) {

        // We're not intersecting the next wall on the same triangle
        // edge, so walk around the triangle edges.

        thisEdgeIndex = (thisEdgeIndex+1) % 3; // (3 edges per triangle)
        poly.ps.add( ps[ thisEdgeIndex ] );
        interiorPoints.add( ps[ thisEdgeIndex ] );
      }

      // Advance to the edge cut at which the triangle edge exits the wall.

      edgeCutIndex = (edgeCutIndex+1)%edgeCuts.size();

      // Stop if we've reached the starting point.

      if (edgeCuts[ edgeCutIndex ].p == poly.ps[0])
        break;
    }

    polys.add( poly );
  }

  tri.processed = true;
}



PLbool edgeCutsWall( plVector3 &v0, plVector3 &v1, plWall &wall, plVector3 &intPoint, PLfloat &edgeParam, PLfloat &wallParam, PLint &intDir )

{
  // Find the intersection point

  PLfloat dot0 = v0 * wall.n;
  PLfloat dot1 = v1 * wall.n;

  PLfloat denom = dot1 - dot0;

  if (fabs(denom) > 1e-6) {	// edge is not parallel to wall

    PLfloat t = (wall.d - dot0) / denom;

    if (0 <= t && t <= 1) {	// edge endpoints are on opposite sides of wall

      // Project intersection point onto line in wall from wall.p0 to wall.p1

      plVector3 x = v0 + t * (v1-v0);

      PLfloat s = ((x-wall.p0) * (wall.p1-wall.p0)) / ((wall.p1-wall.p0)*(wall.p1-wall.p0));

      if (0 <= s && s <= 1) {

        plVector3 y = wall.p0 + s * (wall.p1-wall.p0);

        if ((x-y).length() < 0.5*(wall.p1-wall.p0).length()) {

          intPoint = x;
          intDir = (denom > 0 ? +1 : -1);
          edgeParam = t;
          wallParam = s;	// We're assuming that the projections are monotonically increasing
                                // as we walk across the mesh from one wall extreme to the other.
          return true;
        }
      }
    }
  }

  return false;
}



PLint compareEdgeCuts( const void* a, const void* b )

{
  plCut &pa = * (plCut*) a;
  plCut &pb = * (plCut*) b;

  if (pa.edgeIndex < pb.edgeIndex)
    return -1;
  else if (pa.edgeIndex > pb.edgeIndex)
    return +1;
  else if (pa.edgeParam < pb.edgeParam)
    return -1;
  else if (pa.edgeParam > pb.edgeParam)
    return +1;
  else
    return 0;
}



PLint compareWallCuts( const void* a, const void* b )

{
  plCut &pa = * (plCut*) a;
  plCut &pb = * (plCut*) b;

  if (pa.wallIndex < pb.wallIndex)
    return -1;
  else if (pa.wallIndex > pb.wallIndex)
    return +1;
  else if (pa.wallParam < pb.wallParam)
    return -1;
  else if (pa.wallParam > pb.wallParam)
    return +1;
  else
    return 0;
}



