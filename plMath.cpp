#include "plMath.h"

plVector3 plMath::projectVectorOnPlane( const plVector3 &vector, const plVector3 &plane_normal)
{
    PLfloat dist = vector * plane_normal;
    
    return vector - (dist*plane_normal);
}

plVector3 plMath::closestPointOnSegment(const plVector3 &point, const plVector3 &a, const plVector3 &b)
{
    plVector3 ab = b - a;
    // Project c onto ab, computing parameterized position d(t) = a + t*(b  a)
    PLfloat t = ((point - a) * ab) / (ab * ab);
    // If outside segment, clamp t (and therefore d) to the closest endpoint
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    // Compute projected position from the clamped t
    return a + t * ab;
}

PLfloat plMath::clamp( PLfloat val, PLfloat min, PLfloat max)
{
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

void plMath::swap( PLfloat &a, PLfloat &b )
{
    PLfloat temp = a;
    a = b; 
    b = temp;
}

/*
int plPlaneIntersection( const plvector3 &p1_normal, const plvector3 &p2_normal, Plane p1, Plane p2, Point &p, Vector &d)
{
    const PLfloat EPSILON = 0.000001f;
    // Compute direction of intersection line
    plVector3 d = p1_normal ^  p2_normal;
    // If d is zero, the planes are parallel (and separated)
    // or coincident, so they’re not considered intersecting
    if (d * d < EPSILON) 
        return 0;
        
    PLfloat d11 = p1_normal * p1_normal;
    PLfloat d12 = p1_normal * p2_normal;
    PLfloat d22 = p2_normal * p2_normal;
    PLfloat denom = d11*d22 - d12*d12;
    PLfloat k1 = (p1.d*d22 - p2.d*d12) / denom;
    PLfloat k2 = (p2.d*d11 - p1.d*d12) / denom;
    p = k1*p1.n + k2*p2.n;
    return 1;
}

plVector3 plClosestPointOnPlane(const plVector3 &q, const plVector3 &p, const plVector3 &n)
{
    float t = 1/(n * n) * (n*(q - p));
    return q - t * n;
}
*/


void plMath::convexPolysToTris(plSeq<plPolygon> &polys, plSeq<plTriangle> &tris)
{
    tris.clear();
    for (int i = 0; i < polys.size(); i++)
    {
        if (polys[i].points.size() > 2) // general case
        {
            plVector3 normal(polys[i].normal);
            plVector3 point0(polys[i].points[0]);
            // create a fan of triangles from this point
            for (int currentVertex=1;currentVertex<polys[i].points.size()-1;currentVertex++)
            {
                plVector3 point1(polys[i].points[currentVertex]);
                plVector3 point2(polys[i].points[currentVertex+1]);
                plTriangle tri(normal,point0,point1,point2);
                tris.add(tri);
            }
        }
        else // must be an invalid polygon
        {
            std::cerr << "Error: polygon with less than three vertices detected: " << polys[i].points.size() << ". Aborting." << std::endl;
            exit(1);
        }
    }
}


void plMath::concavePolysToTris(plSeq<plPolygon> &polys, plSeq<plTriangle> &tris)
{
    tris.clear();
    for (PLuint i = 0; i < polys.size(); i++)
    {
        plSeq<plTriangle> polyConverted;
        PLfloat area = FLT_MAX; //(std::numeric_limits<PLfloat>::max());
        plMath::concavePolysToTrisHelper(polys[i],polyConverted,area);
        for (PLuint j = 0; j < polyConverted.size(); j++)
        {
            tris.add(polyConverted[j]);
        }
    }
}

// take as input a polygon
// iterate through each possible triangle involving the vertices at indices 0 and 1 (so 0-1-2, 0-1-3, and so on, basically we have 0-1-X)
// there will be two polygons remaining:
// the one from index 1 to X
// the one from index X to 0
// recursively call this function for each of these smaller polygons
// store only the sequence of triangles that results in the smallest surface area
// also store the surface area of the triangulation
// INITIAL VALUE FOR OUTPUTTRIANGLES SHOULD BE AN EMPTY PLSEQ!
// INITIAL VALUE FOR OUTPUTMINSURFACEAREA SHOULD BE THE MAXIMUM VALUE FOR A FLOATING POINT
void plMath::concavePolysToTrisHelper(plPolygon &inputPolygon, plSeq<plTriangle> &outputTriangles, PLfloat &outputMinSurfaceArea)
{
    if (inputPolygon.points.size() <= 2) { // base case
        outputMinSurfaceArea = 0.f;
        return;
    }

    // try all permutations of triangles involving the edge between points[0] and points[1]
    for (PLuint i = 2; i < inputPolygon.points.size(); i++)
    {
        plTriangle bisectingTriangle( inputPolygon.points[0], inputPolygon.points[1], inputPolygon.points[i] );
        PLfloat    bisectingTriangleArea ( ((inputPolygon.points[0]-inputPolygon.points[i])^(inputPolygon.points[1]-inputPolygon.points[i])).length() / 2.f );

        // first polygon
        plPolygon firstPolygonInput;
        for (PLuint j = 1; j <= i; j++)
        {
            firstPolygonInput.points.add(inputPolygon.points[j]);
        }
        plSeq<plTriangle> firstPolygonOutputTris;
        PLfloat           firstPolygonOutputArea = FLT_MAX; //(std::numeric_limits<PLfloat>::max());
        plMath::concavePolysToTrisHelper(firstPolygonInput,firstPolygonOutputTris,firstPolygonOutputArea);

        // second polygon
        plPolygon secondPolygonInput;
        secondPolygonInput.points.add(inputPolygon.points[0]);
        for (PLuint j = i; j < inputPolygon.points.size(); j++)
        {
            secondPolygonInput.points.add(inputPolygon.points[j]);
        }
        plSeq<plTriangle> secondPolygonOutputTris;
        PLfloat           secondPolygonOutputArea = FLT_MAX; //(std::numeric_limits<PLfloat>::max());
        plMath::concavePolysToTrisHelper(secondPolygonInput,secondPolygonOutputTris,secondPolygonOutputArea);

        // calculate current surface
        PLfloat currentSurfaceArea( bisectingTriangleArea + firstPolygonOutputArea + secondPolygonOutputArea );

        // update the output if necessary
        if (currentSurfaceArea < outputMinSurfaceArea)
        {
            outputTriangles.clear();
            outputTriangles.add(bisectingTriangle);
            for (int j = 0; j < firstPolygonOutputTris.size(); j++)
            {
                outputTriangles.add(firstPolygonOutputTris[j]);
            }
            for (int j = 0; j < secondPolygonOutputTris.size(); j++)
            {
                outputTriangles.add(secondPolygonOutputTris[j]);
            }
            outputMinSurfaceArea = currentSurfaceArea;
        }
    } // end for

} // end void function()

