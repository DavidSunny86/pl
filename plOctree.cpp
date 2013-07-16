#include "plOctree.h"
 
plOctree::plOctree( const plModel &model, PLuint depth, PLbool exclusive )
    : _exclusive(exclusive)
{
    if (depth == 0)
    {
        std::cerr << "plOctree plOctree() error: depth must be greater than 0\n";
        exit(1);
    }
    
    // get min and max extents of model
    plVector3 min, max;
    model.getMinMax(min,max);    
    
    // centre point of octree
    plVector3 centre = 0.5f * (min+max);
    
    // find largest distance component, becomes half width
    plVector3 minDiff = min - centre;
    plVector3 maxDiff = max - centre;
    PLfloat minMax = PL_MAX_OF_3( fabs(minDiff.x), fabs(minDiff.y), fabs(minDiff.z) );
    PLfloat maxMax = PL_MAX_OF_3( fabs(maxDiff.x), fabs(maxDiff.y), fabs(maxDiff.z) );       
    PLfloat halfWidth = PL_MAX_OF_2( minMax, maxMax );
    
    // Construct and fill in root 
    root = new plOctreeNode( centre, halfWidth );
 
    // traverse and fill sub cubes with triangles until desired depth is reached  
    _fill( model.triangles(), depth, exclusive );
}


void plOctree::draw() const
{
    root->draw();
}


plIntersection plOctree::rayIntersect( const plVector3 &rayOrigin, const plVector3 &rayDirection, PLbool ignoreBehindRay, PLbool backFaceCull) const
{
    plIntersection closestIntersection(false);

    plSeq<const plTriangle*> triangles;

    if (root->rayIntersect( triangles, rayOrigin, rayDirection, ignoreBehindRay ))
    {
        PLfloat min = FLT_MAX;

        for ( PLuint i = 0; i < triangles.size(); i++)
        {  
            plIntersection intersection = triangles[i]->rayIntersect( rayOrigin, rayDirection, ignoreBehindRay, backFaceCull );
            
            if (intersection.exists)
            {
                if ( fabs(intersection.t) < min) 
                {
                    min = fabs(intersection.t);
                    closestIntersection = intersection;
                }
            }

        }
    }
    
    std::cout << "triangle checks:" << triangles.size() << "\n";
    
    return closestIntersection; 
    
}


void plOctree::_fill(const plSeq<plTriangle> &triangles, PLuint depth, PLbool exclusive)
{
    for (PLuint i=0; i< triangles.size(); i++)
    {
        root->insert( triangles[i], depth-1, exclusive );
    }
}


plOctreeNode::plOctreeNode( const plVector3 &c, PLfloat hw)
    : children(NULL, 8), centre(c), halfWidth(hw)
{
    // create mesh
    _updateMesh();
}


void plOctreeNode::draw() const
{
    PLint count = 0;

    for (PLuint i=0; i < 8; i++)
    {
        if (children[i] != NULL)
        {
            children[i]->draw();
            count++;
        }
    }

    if (contained.size() > 0 || count > 0)    // only draw if contains objects, or has children that contain
        _mesh.draw();
    
}


void plOctreeNode::insert( const plTriangle &tri, PLuint depth, PLbool exclusive)
{
    // assumes the given triangle is already bound by the octree, so there 
    // will enver be the case where it does not intersect the current node
    
    // add to current node
    contained.add( &tri );

    // contains the 0-7 index of the child, determined using bit wise addition
    PLint child = 0;

    // distance from each axis
    PLfloat dx = tri.centroid().x - centre.x;
    PLfloat dy = tri.centroid().y - centre.y;
    PLfloat dz = tri.centroid().z - centre.z;

    // if distance is less than radius, then the triangle straddles a boundary
    if ( fabs(dx) < tri.radius() || fabs(dy) < tri.radius() || fabs(dz) < tri.radius() )
    {
        // straddles a boundary
        //contained.add( &tri );
        
        if (!exclusive)
        {
            // non exclusive, add to intersected children as well
            for (PLuint i=0; i<8; i++)
            {
                if ( sphereCheck( tri.centroid(), tri.radius(), i) )
                    // part of bounding sphere intersects child, insert
                    _insertChild( i, tri, depth, exclusive ); 
            }    
       }     
            
    }    
    else
    {    
        // fully contained in child, find child         
        if ( dx > 0 ) child += 1;
        if ( dy > 0 ) child += 2;
        if ( dz > 0 ) child += 4;
         
        _insertChild( child, tri, depth, exclusive ); 
    } 
}

void plOctreeNode::_insertChild( PLuint index, const plTriangle &tri, PLuint depth, PLbool exclusive )
{
    if (children[index] != NULL)    
    {     
        // child already exists, recursively insemaxAABB  
        children[index]->insert( tri, depth-1, exclusive );
    }    
    else   
    { 
        // child does not exist, if terminal depth has not been reached, create child node
        if (depth > 0)
        {
        
            plVector3 offset;
            PLfloat step = halfWidth * 0.5f;
            offset.x = ((index & 1) ? step : -step);
            offset.y = ((index & 2) ? step : -step);
            offset.z = ((index & 4) ? step : -step);
            children[index] = new plOctreeNode( centre + offset, step);
            children[index]->insert( tri, depth - 1, exclusive );
        }
        /*
        else
        {
            // terminal depth reached, add to this node
            contained.add( &tri );
        }
        */
    }
}

PLfloat plOctreeNode::squaredDistanceFromPoint( const plVector3 &point, PLint child ) const
{
    plVector3 minAABB, maxAABB;    
    if (child != -1)
    {
        // check for child      
        plVector3 offsetCentre = centre; 
        PLfloat step = 0.5f * halfWidth;
        offsetCentre.x += ((child & 1) ? step : -step);
        offsetCentre.y += ((child & 2) ? step : -step);
        offsetCentre.z += ((child & 4) ? step : -step);
        minAABB = plVector3( offsetCentre.x - step, offsetCentre.y - step, offsetCentre.z - step );
        maxAABB = plVector3( offsetCentre.x + step, offsetCentre.y + step, offsetCentre.z + step );
    }
    else
    {
        // check for this node
        minAABB = plVector3( centre.x - halfWidth, centre.y - halfWidth, centre.z - halfWidth );
        maxAABB = plVector3( centre.x + halfWidth, centre.y + halfWidth, centre.z + halfWidth );   
    }

    PLfloat sqDist = 0.0f;
    // For each axis count any excess distance outside box extents
    // x
    if (point.x < minAABB.x) sqDist += (minAABB.x - point.x) * (minAABB.x - point.x);
    if (point.x > maxAABB.x) sqDist += (point.x - maxAABB.x) * (point.x - maxAABB.x);
    // y
    if (point.y < minAABB.y) sqDist += (minAABB.y - point.y) * (minAABB.y - point.y);
    if (point.y > maxAABB.y) sqDist += (point.y - maxAABB.y) * (point.y - maxAABB.y);
    // z
    if (point.z < minAABB.z) sqDist += (minAABB.z - point.z) * (minAABB.z - point.z);
    if (point.z > maxAABB.z) sqDist += (point.z - maxAABB.z) * (point.z - maxAABB.z);
        
    return sqDist;
}

PLbool plOctreeNode::sphereCheck( const plVector3 &centre, PLfloat radius, PLint child ) const
{
    // compute squared distance between sphere center and AABB
    PLfloat sqDist = squaredDistanceFromPoint(centre, child);
    // sphere and AABB intersect if the distance is less than the sphere radius
    return sqDist <= radius * radius;
}


PLbool plOctreeNode::rayIntersect( plSeq<const plTriangle*> &triangles, const plVector3 &rayOrigin, const plVector3 &rayDirection, PLbool ignoreBehindRay ) const
{

    plVector3 minAABB( centre.x - halfWidth, centre.y - halfWidth, centre.z - halfWidth );
    plVector3 maxAABB( centre.x + halfWidth, centre.y + halfWidth, centre.z + halfWidth );
    
    // r.dir is unit direction vector of ray
    plVector3 directionDiv( 1.0f / rayDirection.x,
                            1.0f / rayDirection.y,
                            1.0f / rayDirection.z );
                            
    // minAABB is the corner of AABB with minimal coordinates - left bottom, maxAABB is maximal corner
    // rayOrigin is origin of ray
    float t1 = (minAABB.x - rayOrigin.x)*directionDiv.x;
    float t2 = (maxAABB.x - rayOrigin.x)*directionDiv.x;
    float t3 = (minAABB.y - rayOrigin.y)*directionDiv.y;
    float t4 = (maxAABB.y - rayOrigin.y)*directionDiv.y;
    float t5 = (minAABB.z - rayOrigin.z)*directionDiv.z;
    float t6 = (maxAABB.z - rayOrigin.z)*directionDiv.z;

    float tmin = PL_MAX_OF_3( PL_MIN_OF_2(t1, t2), 
                              PL_MIN_OF_2(t3, t4), 
                              PL_MIN_OF_2(t5, t6) );
      
    float tmax = PL_MIN_OF_3( PL_MAX_OF_2(t1, t2), 
                              PL_MAX_OF_2(t3, t4), 
                              PL_MAX_OF_2(t5, t6) );

    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us    
    if (ignoreBehindRay && tmax < 0)
        return false;       // box is behind ray, 
    
    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax)
        return false;

    PLuint childCount = 0;
    // intersection exists, recurse further
    for (PLuint i=0; i<children.size(); i++)
    {       
        if (children[i] != NULL)
        {
            // not a leaf, recurse
            children[i]->rayIntersect( triangles, rayOrigin, rayDirection ); 
            childCount++;            
        }           
    }

    if (childCount == 0)
    {
        // leaf, add triangles
        for (PLuint j=0; j<contained.size(); j++)
            triangles.add( contained[j] );
    }

    return true;
}



void plOctreeNode::_updateMesh()
{
    // DEBUG MESH
    plSeq<plVector3> vertices( 8  );
    plSeq<PLuint>    indices ( 8*3 ); //12 );
    
    // front face
    vertices.add( plVector3( centre.x - halfWidth, centre.y - halfWidth, centre.z + halfWidth) );
    vertices.add( plVector3( centre.x + halfWidth, centre.y - halfWidth, centre.z + halfWidth) );
    vertices.add( plVector3( centre.x + halfWidth, centre.y + halfWidth, centre.z + halfWidth) );
    vertices.add( plVector3( centre.x - halfWidth, centre.y + halfWidth, centre.z + halfWidth) );
    
    // back face
    vertices.add( plVector3( centre.x - halfWidth, centre.y - halfWidth, centre.z - halfWidth) );
    vertices.add( plVector3( centre.x + halfWidth, centre.y - halfWidth, centre.z - halfWidth) );
    vertices.add( plVector3( centre.x + halfWidth, centre.y + halfWidth, centre.z - halfWidth) );
    vertices.add( plVector3( centre.x - halfWidth, centre.y + halfWidth, centre.z - halfWidth) );
    
    // front
    indices.add( 0 );   indices.add( 1 );   
    indices.add( 1 );   indices.add( 2 );   
    indices.add( 2 );   indices.add( 3 );   
    indices.add( 3 );   indices.add( 0 );   

    // sides 
    indices.add( 0 );   indices.add( 4 );   
    indices.add( 1 );   indices.add( 5 );   
    indices.add( 2 );   indices.add( 6 );   
    indices.add( 3 );   indices.add( 7 ); 

    // back
    indices.add( 4 );   indices.add( 5 );   
    indices.add( 5 );   indices.add( 6 );   
    indices.add( 6 );   indices.add( 7 );   
    indices.add( 7 );   indices.add( 4 );

    _mesh.setBuffers( vertices, indices );
}

