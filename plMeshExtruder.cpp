#include "plMeshExtruder.h"

namespace plMeshExtruder
{
    // helper to collectOutsideEdges
    std::vector<plEdgePointers> _collectOutsideEdgesUnsorted( const std::vector<plTriangle>& triangles )
    {
        // all edges that are on the border will be unique -
        // that is to say, it will occur exactly once within the set of triangles.
        // any edge that occurs twice must be between two faces, and is therefore not on the border

        std::vector<plEdgePointers> unsortedEdges;  unsortedEdges.reserve( triangles.size() ); // set to max size of triangles, this std::vector is discarded later anyway

        // for each triangle
        for (PLuint i = 0; i < triangles.size(); i++)
        {
            // for each edge
            for (PLuint j = 0; j < 3; j++)
            {
                // next point index on triangle
                PLuint k = (j+1) % 3;

                // try to find the edge in question
                PLuint e;
                for ( e = 0; e < unsortedEdges.size(); e++)
                {
                    // any edge that is being examined will be oriented opposite to what was already found
                    if ( *unsortedEdges[e].p0 == triangles[i][k] && *unsortedEdges[e].p1 == triangles[i][j] )
                    {
                        break;
                    }
                }

                if ( e == unsortedEdges.size() )
                {
                    // if the edge does not already exist, add it to the list
                    unsortedEdges.push_back( plEdgePointers( &triangles[i][j], &triangles[i][k] ) );
                }
                else
                {
                    // if the edge DOES exist, it must be between two faces and therefore can't be on the border... so remove it
                    unsortedEdges.erase( unsortedEdges.begin()+e );
                }
            }
        }
        return unsortedEdges;
    }

    // helper to collectOutsideEdges
    std::vector<plEdgePointers> _collectOutsideEdgesSort( std::vector<plEdgePointers> &unsortedEdges )
    {
        // traverse along the edges and return to the first vertex
        std::vector<plEdgePointers> sortedEdges;    sortedEdges.reserve( unsortedEdges.size() );  // this stores the indices for bookkeeping and future manipulation

        plVector3 finalVertex   = *unsortedEdges[0].p0; // the final vertex should be this, this should be checked
        plVector3 currentVertex = *unsortedEdges[0].p1; // the current vertex, we'll look to 'hop' to another edge with this vertex

        sortedEdges.push_back( unsortedEdges[0] ); // first edge, arbitrarily chosen to be here
        unsortedEdges.erase( unsortedEdges.begin() );             // don't need to search here, so remove it from the list of unsorted edges

        while ( unsortedEdges.size() )
        {
            // search for the current vertex in the list of edges. It will appear as the first vertex in an ordered pair, since all edges are counter-clockwise
            PLuint i;
            for ( i = 0; i < unsortedEdges.size(); i++)
            {
                if ( *unsortedEdges[ i ].p0 == currentVertex )
                {
                    break;
                }
            }

            if ( i == unsortedEdges.size() || (unsortedEdges.size() == 0 && currentVertex != finalVertex)) // should not happen if the edges contain a closed surface
            {
                std::cerr << "Error in plMeshExtruder::collectOutsideEdges: the edges don't contain a closed surface! Aborting." << std::endl;
                return std::vector<plEdgePointers>();
            }
            else
            {
                // update the current vertex
                currentVertex = *unsortedEdges[ i ].p1;

                // move this entry into the sorted list
                sortedEdges.push_back( unsortedEdges[i] );
                unsortedEdges.erase( unsortedEdges.begin()+i );
            }
        }

        return sortedEdges;
    }


    std::vector<plEdgePointers> _collectOutsideEdges( const std::vector<plTriangle>& triangles )
    {
        std::vector<plEdgePointers> edges = _collectOutsideEdgesUnsorted( triangles );
        return _collectOutsideEdgesSort( edges );
    }

    void _prepareTopAndBottomOfExtrusion( const std::vector<plTriangle>& inputTriangles, const plVector3& offset, std::vector<plTriangle>& outputTriangles )
    {
        if (outputTriangles.size() > 0)
            std::cout << "plMeshExtruder::_prepareTopAndBottomOfExtrusion() Warning: outputTriangles had contents before entering function. Clearing." << std::endl;

        outputTriangles.clear();

        outputTriangles = inputTriangles; // add existing triangles

        PLuint indexOfUpperTriangles = outputTriangles.size();
        for (PLuint i = 0; i < indexOfUpperTriangles; i++)
        {
            plVector3 point0( outputTriangles[i].point0() + offset );
            plVector3 point1( outputTriangles[i].point1() + offset );
            plVector3 point2( outputTriangles[i].point2() + offset );
            plVector3 normal( outputTriangles[i].normal() );
            outputTriangles.push_back( plTriangle( normal, point0, point1, point2 ) );
        }

        // reverse normals of bottom side
        for (PLuint i = 0; i < indexOfUpperTriangles; i++)
        {
            outputTriangles[i].swapVertices0And1(); // reverses the normal
        }
    }

    void _prepareSidesOfExtrusionFlat( const std::vector<plEdgePointers>& outsideEdges, const plVector3& offset, std::vector<plTriangle>& outputTrianglesToAddTo )
    {
        // now fill in the sides of the template with triangles
        for (PLuint i = 0; i < outsideEdges.size(); i++)
        {
            const plVector3 &p0 = *outsideEdges[i].p0;
            const plVector3 &p1 = *outsideEdges[i].p1;
            plVector3 p2 = *outsideEdges[i].p1 + offset;
            plVector3 p3 = *outsideEdges[i].p0 + offset;

            outputTrianglesToAddTo.push_back( plTriangle( p0, p1, p2 ) );
            outputTrianglesToAddTo.push_back( plTriangle( p0, p2, p3 ) );
        }
    }

    void _prepareSidesOfExtrusionSmooth( const std::vector<plEdgePointers>& outsideEdges, const plVector3& offset, const PLuint numDivisions, std::vector<plTriangle>& outputTrianglesToAddTo )
    {
        // first determine an evenly-spaced set of floating point numbers, so we can determine a smooth circle shape using cos() and sin() later
        std::vector<PLfloat> divisionParameters;
        divisionParameters.push_back(0.f);
        for (PLuint i = 0; i < numDivisions; i++)
            divisionParameters.push_back( PLfloat(PL_PI) * PLfloat(i+1) / PLfloat(numDivisions+1) );
        divisionParameters.push_back(PL_PI);

        // now fill in the sides of the template with triangles
        for (PLuint i = 0; i < outsideEdges.size(); i++)
        {
            plVector3 relativeX( 0.5f * ( (offset^(*outsideEdges[i].p0 - *outsideEdges[i].p1)).normalize() * offset.length() ) );
            plVector3 relativeY( 0.5f * offset );
            for (PLuint j = 0; j < numDivisions; j++)
            {
                plVector3 p0, p1, p2, p3;
                if (j == 0) // avoid roundoff error for this case
                {
                    p0 = *outsideEdges[i].p0;
                    p1 = *outsideEdges[i].p1;
                }
                else
                {
                    PLfloat paramX(  sin(divisionParameters[j]) );
                    PLfloat paramY( -cos(divisionParameters[j]) );
                    p0 = *outsideEdges[i].p0 + 0.5f * offset + paramX * relativeX + paramY * relativeY;
                    p1 = *outsideEdges[i].p1 + 0.5f * offset + paramX * relativeX + paramY * relativeY;
                }

                if (j == numDivisions-1) // avoid roundoff error for this case
                {
                    p2 = *outsideEdges[i].p1 + offset;
                    p3 = *outsideEdges[i].p0 + offset;
                }
                else
                {
                    PLfloat paramX(  sin(divisionParameters[j+1]) );
                    PLfloat paramY( -cos(divisionParameters[j+1]) );
                    p2 = *outsideEdges[i].p1 + 0.5f * offset + paramX * relativeX + paramY * relativeY;
                    p3 = *outsideEdges[i].p0 + 0.5f * offset + paramX * relativeX + paramY * relativeY;
                }

                outputTrianglesToAddTo.push_back( plTriangle( p0, p1, p2 ) );
                outputTrianglesToAddTo.push_back( plTriangle( p0, p2, p3 ) );
            }
        }
    }


    plMesh extrudeMesh( const plMesh& mesh, const PLfloat magnitude, const plVector3& direction )
    {
        // error checking
        if ( mesh.triangles().empty() )      { std::cerr << "plMeshExtruder::extrudeMesh() error: No triangles in input!\n";     return std::vector<plTriangle>(); }
        if ( magnitude <= 0 )                { std::cerr << "plMeshExtruder::extrudeMesh() error: Magnitude of zero or less!\n"; return std::vector<plTriangle>(); }
        if ( direction.squaredLength() == 0) { std::cerr << "plMeshExtruder::extrudeMesh() error: Direction of zero!\n";         return std::vector<plTriangle>(); }

        std::vector<plEdgePointers> outsideEdges = _collectOutsideEdges( mesh.triangles() );

        if ( outsideEdges.empty() )          { std::cerr << "plMeshExtruder::extrudeMesh() error: No surrounding edges found\n"; return std::vector<plTriangle>(); }

        std::vector<plTriangle> outputTriangles;    outputTriangles.reserve( mesh.triangles().size() * 2 + outsideEdges.size() * 2 ); // allocate enough memory

        // we need to extrude a bunch of these triangles in the direction of the average normal
        plVector3 offset = magnitude * direction.normalize();

        _prepareTopAndBottomOfExtrusion( mesh.triangles(), offset, outputTriangles );
        _prepareSidesOfExtrusionFlat   ( outsideEdges,     offset, outputTriangles );

        return plMesh( outputTriangles );
    }

    /*
    std::vector<plTriangle> extrudeMesh( const std::vector<plTriangle>& inputTriangles,
                                         const PLfloat magnitude,
                                         const PLfloat preTranslation,
                                         const plVector3& direction )
    {
        // error checking
        if ( inputTriangles.size()     == 0) { std::cerr << "plMeshExtruder::extrudeMesh() error: No triangles in input!\n";     return std::vector<plTriangle>(); }
        if ( magnitude                 <= 0) { std::cerr << "plMeshExtruder::extrudeMesh() error: Magnitude of zero or less!\n"; return std::vector<plTriangle>(); }
        if ( direction.squaredLength() == 0) { std::cerr << "plMeshExtruder::extrudeMesh() error: Direction of zero!\n";         return std::vector<plTriangle>(); }

        // apply the translation to the triangles before sending it to be extruded.
        plVector3  preTranslationVector = preTranslation * direction.normalize();
        plMatrix44 preTranslationMatrix;    preTranslationMatrix.setTranslation( preTranslationVector );

        std::vector<plTriangle> preTranslatedTriangles;
        for (PLuint i = 0; i < inputTriangles.size(); i++)
        {
            preTranslatedTriangles.push_back( preTranslationMatrix * inputTriangles[i] );
        }

        // call the normal extrude function now
        return extrudeMesh( preTranslatedTriangles, magnitude, direction );
    }
    */


    /*

    // helper to collectOutsideEdges
    std::vector<plEdgeIndices> _collectOutsideEdgesUnsorted(const std::vector<plTriangle>& inputTriangles)
    {
        // all edges that are on the border will be unique -
        // that is to say, it will occur exactly once within the set of triangles.
        // any edge that occurs twice must be between two faces, and is therefore not on the border

        std::vector<plEdgeIndices> unsortedEdges;  // this stores the indices for bookkeeping and future manipulation
        for (PLuint triangleIndex = 0; triangleIndex < inputTriangles.size(); triangleIndex++)
        {
            // examine each edge
            for (PLuint vertexIndex = 0; vertexIndex < 3; vertexIndex++)
            {
                // vertex pair
                PLuint vertexAIndex = vertexIndex;
                PLuint vertexBIndex = (vertexIndex+1) % 3;

                // try to find the edge in question
                PLuint unsortedEdgeIndex;
                for (unsortedEdgeIndex = 0; unsortedEdgeIndex < unsortedEdges.size(); unsortedEdgeIndex++)
                {
                    // for readability - these are the indices to get to the vertices that we'll actually be looking at
                    PLuint searchTriangleIndex (unsortedEdges[unsortedEdgeIndex].triangleIndex);
                    PLuint searchVertexAIndex  (unsortedEdges[unsortedEdgeIndex].vertexIndexA);
                    PLuint searchVertexBIndex  (unsortedEdges[unsortedEdgeIndex].vertexIndexB);

                    // any edge that is being examined will be oriented opposite to what was already found
                    if (inputTriangles[searchTriangleIndex][searchVertexAIndex] == inputTriangles[triangleIndex][vertexBIndex] &&
                        inputTriangles[searchTriangleIndex][searchVertexBIndex] == inputTriangles[triangleIndex][vertexAIndex])
                    {
                        break;
                    }
                } // end for

                if (unsortedEdgeIndex == unsortedEdges.size()) // if the edge does not already exist, add it to the list
                {
                    unsortedEdges.push_back( plEdgeIndices(triangleIndex, vertexIndex, ((vertexIndex+1)%3) ) );
                }
                else // if the edge DOES exist, it must be between two faces and therefore can't be on the border... so remove it
                {
                    unsortedEdges.remove(unsortedEdgeIndex);
                } // end if
            } // end for
        } // end for
        return unsortedEdges;
    }

    // helper to collectOutsideEdges
    std::vector<plEdgeIndices> _collectOutsideEdgesSort( std::vector<plEdgeIndices> unsortedEdges, const std::vector<plTriangle>& inputTriangles)
    {
        // traverse along the edges and return to the first vertex
        std::vector<plEdgeIndices> sortedEdges;  // this stores the indices for bookkeeping and future manipulation

        plVector3 finalVertex   = inputTriangles[unsortedEdges[0].triangleIndex][unsortedEdges[0].vertexIndexA]; // the final vertex should be this, this should be checked
        plVector3 currentVertex = inputTriangles[unsortedEdges[0].triangleIndex][unsortedEdges[0].vertexIndexB]; // the current vertex, we'll look to 'hop' to another edge with this vertex

        sortedEdges.push_back(unsortedEdges[0]); // first edge, arbitrarily chosen to be here
        unsortedEdges.remove(0);           // don't need to search here, so remove it from the list of unsorted edges

        while ( unsortedEdges.size() )
        {
            // search for the current vertex in the list of edges. It will appear as the first vertex in an ordered pair, since all edges are counter-clockwise
            PLuint unsortedEdgeIndex;
            for (unsortedEdgeIndex = 0; unsortedEdgeIndex < unsortedEdges.size(); unsortedEdgeIndex++)
            {
                PLuint triangleIndex (unsortedEdges[unsortedEdgeIndex].triangleIndex);
                PLuint vertexAIndex  (unsortedEdges[unsortedEdgeIndex].vertexIndexA);
                if (inputTriangles[triangleIndex][vertexAIndex] == currentVertex)
                {
                    break;
                }
            } // end for

            if (unsortedEdgeIndex == unsortedEdges.size() || (unsortedEdges.size() == 0 && currentVertex != finalVertex)) // should not happen if the edges contain a closed surface
            {
                std::cerr << "Error in plIGuide::collectOutsideEdges: either the edges don't contain a closed surface, or there is more than one unclosed mesh in the input! Aborting." << std::endl;
                exit(1);
            }
            else
            {
                // update the current vertex
                PLuint triangleIndex (unsortedEdges[unsortedEdgeIndex].triangleIndex);
                PLuint vertexBIndex  (unsortedEdges[unsortedEdgeIndex].vertexIndexB);
                currentVertex = (inputTriangles[triangleIndex][vertexBIndex]);

                // move this entry into the sorted list
                sortedEdges.push_back(unsortedEdges[unsortedEdgeIndex]);
                unsortedEdges.remove(unsortedEdgeIndex);
            } // end if
        } // end while

        return sortedEdges;
    }


    std::vector<plEdgeIndices> _collectOutsideEdges(const std::vector<plTriangle>& inputTriangles)
    {
        std::vector<plEdgeIndices> edges;
        edges = _collectOutsideEdgesUnsorted( inputTriangles );
        if (edges.size() == 0)
        {
            return edges; // don't proceed, let the calling function catch this.
        }
        edges = _collectOutsideEdgesSort( edges, inputTriangles );
        return edges;
    }


    // return an empty std::vector if this fails.
    std::vector<plTriangle> extrudeMesh(const std::vector<plTriangle>& inputTriangles, const PLfloat magnitude, const plVector3& direction)
    {
        std::vector<plTriangle>    outputTriangles;

        if (inputTriangles.size() == 0)
        {
            std::cerr << "Error in plMeshExtruder::extrudeMesh(): No triangles in input. Aborting." << std::endl;
            return outputTriangles;
        }
        if (magnitude == 0)
        {
            std::cerr << "Error in plMeshExtruder::extrudeMesh(): Magnitude of zero results in no extrusion. Aborting." << std::endl;
            return outputTriangles;
        }
        plVector3 zeroVector(0.f,0.f,0.f);
        if (direction == zeroVector)
        {
            std::cerr << "Error in plMeshExtruder::extrudeMesh(): Direction of zero results in no extrusion. Aborting." << std::endl;
            return outputTriangles;
        }

        std::vector<plEdgeIndices> outsideEdges = _collectOutsideEdges(inputTriangles);

        if (outsideEdges.size() == 0)
        {
            std::cerr << "Error in plMeshExtruder::extrudeMesh(): No surrounding edges found. Is the mesh closed? Aborting." << std::endl;
            return outputTriangles;
        }

        outputTriangles.push_back(inputTriangles); // create a copy of existing triangles, because they will essentially become part of the offset

        // store these for bookkeeping. This is the index where upper (extruded) edges/triangles will start
        PLuint indexOfUpperEdges     = outsideEdges.size();
        PLuint indexOfUpperTriangles = outputTriangles.size();

        // we need to extrude a bunch of these triangles in the direction of the average normal
        plVector3 offset ( magnitude * direction.normalize() );
        for (PLuint i = 0; i < indexOfUpperTriangles; i++)
        {
            plVector3 point0( outputTriangles[i].point0() + offset );
            plVector3 point1( outputTriangles[i].point1() + offset );
            plVector3 point2( outputTriangles[i].point2() + offset );
            plVector3 normal( outputTriangles[i].normal() );
            outputTriangles.push_back( plTriangle(normal,point0,point1,point2) );
        }

        // update edgeIndices
        for (PLuint i = 0; i < indexOfUpperEdges; i++)
        {
            outsideEdges.push_back(outsideEdges[i]);
            outsideEdges[i+indexOfUpperEdges].triangleIndex = outsideEdges[i+indexOfUpperEdges].triangleIndex + indexOfUpperTriangles;
        }

        // we need to reverse the normals of the triangles that lie on top of the knee surface
        for (PLuint i = 0; i < indexOfUpperTriangles; i++)
        {
            outputTriangles[i].swapVertices0And1(); // reverses the normal
        }

        // update edgeIndices
        for (PLuint i = 0; i < indexOfUpperEdges; i++)
        {
            outsideEdges[i].vertexIndexA = (outsideEdges[i].vertexIndexA==2?2:!outsideEdges[i].vertexIndexA); // results in mapping: 2 => 2, 1 => 0, 0 => 1. Since vertices 0 and 1 were flipped.
            outsideEdges[i].vertexIndexB = (outsideEdges[i].vertexIndexB==2?2:!outsideEdges[i].vertexIndexB);
        }

        // now fill in the sides of the template with triangles
        for (PLuint i = 0; i < indexOfUpperEdges; i++)
        {
            PLuint lowerTriangleIndex (outsideEdges[i].triangleIndex);
            PLuint lowerVertexIndexA  (outsideEdges[i].vertexIndexA);
            PLuint lowerVertexIndexB  (outsideEdges[i].vertexIndexB);
            PLuint upperTriangleIndex (outsideEdges[i+indexOfUpperEdges].triangleIndex);
            PLuint upperVertexIndexA  (outsideEdges[i+indexOfUpperEdges].vertexIndexA);
            PLuint upperVertexIndexB  (outsideEdges[i+indexOfUpperEdges].vertexIndexB);
            outputTriangles.push_back(plTriangle(outputTriangles[lowerTriangleIndex][lowerVertexIndexA],
                                           outputTriangles[lowerTriangleIndex][lowerVertexIndexB],
                                           outputTriangles[upperTriangleIndex][upperVertexIndexA] ) );
            outputTriangles.push_back(plTriangle(outputTriangles[lowerTriangleIndex][lowerVertexIndexB],
                                           outputTriangles[upperTriangleIndex][upperVertexIndexB],
                                           outputTriangles[upperTriangleIndex][upperVertexIndexA] ) );
        }

        return outputTriangles;

    }
    */

}
