#include "plBoundary.h"

plBoundary::plBoundary()
{
    _showWalls = true;
}


void plBoundary::toggleVisibility()
{
    if (_isVisible && _showWalls)
    {
        _showWalls = false;
    } 
    else if (_isVisible && !_showWalls)
    {
        _isVisible = false;
    }
    else
    {
        _isVisible = true;
        _showWalls = true;
    }
}


plVector3 plBoundary::getAverageNormal() const
{
    plVector3 n(0,0,0);
    for (PLuint i=0; i < _normals.size(); i++)
    {
        n = n + _normals[i];        
    }
    return n.normalize();
}


PLuint plBoundary::size() const
{
    return _points.size();
}


void plBoundary::loadPointAndNormal(const plString &point, const plString &normal)
{
    // load in _points from a plan file, this assumes they are counter-clockwise already
    _points.add(  plVector3( point ) );                  
    _normals.add( plVector3( normal ) );
}

PLuint plBoundary::addPointAndNormal(const plVector3 &point, const plVector3 &normal)
{
    if (_points.size() < 3) 
    {
        // 0 or 1 _points, doesnt matter, just add
        _points.add( point );
        _normals.add( normal );
        return _points.size()-1;
    }
    else if (_points.size() == 3) 
    {
        // 2 _points, ensure third is counter clockwise
        plVector3 n = 0.5f * (_normals[1] + _normals[0]);
        
        if ( ((_points[1] - _points[0]) ^ (point - _points[0])) * n > 0)
        {
            // already counter clockwise
            _points.add( point );
            _normals.add( normal );
            return 2;
        }
        else
        {
            std::cout << "eep\n";
            // clock-wise, add new point between existing two
            _points.shift(1);
            _normals.shift(1);
            _points[1] = point;
            _normals[1] = normal;
            return 1;
        }
    } 
    else 
    {    
        // Find the closest edge to the new point        
        PLfloat minDist = FLT_MAX;
        PLint shift_i = 0;

        const PLfloat EPSILON = 0.000001f;
               
        for (PLuint i = 0; i < _points.size(); i++)
        {
            PLint j = (i+1) % _points.size();   // next point
            // get closest point on current edge
            plVector3 closest = plClosestPointOnLineSegment(point, _points[i], _points[j]);
            // compare distance to current minimum distance
            PLfloat distSquared = (closest - point).squaredLength();
            
            // check if same point as previous edge, occurs when edges are at acute angles, making a pointy "v"
            if ( fabs(distSquared - minDist) < EPSILON )
            {
                // same point as previous edge,                
                int h = (i == 0) ? _points.size()-1 : i-1; // previous index                      
                // get current and previous wall segments        
                plVector3 previous_segment = (_points[i] - _points[h]); 
                plVector3 current_segment  = (_points[j] - _points[i]);     
                // get current wall tangent                        
                plVector3 current_tangent  = current_segment ^ (0.5f * (_normals[j] + _normals[i]));
                 
                // check if current "v" shape is pointing inwards or outwards 
                if ( (current_segment ^ previous_segment) * _normals[i] < 0)
                {
                    // outward pointing "v"
                    if ((point - closest) * current_tangent  > 0 )
                    {
                        // not behind previous edge
                        minDist = distSquared;
                        shift_i = j;
                    } 
                }
                else
                {
                    // inward pointing "v"
                    if ((point - closest) * current_tangent  < 0 )
                    {
                        // in front of current edge
                        minDist = distSquared;
                        shift_i = j;
                    }
                }
            }
            else if (distSquared < minDist)
            {
                // new minimum, store index
                minDist = distSquared;
                shift_i = j;
            }
        }
        _normals.shift(shift_i);
        _normals[shift_i] = normal;

        _points.shift(shift_i);
        _points[shift_i] = point; 
         
        return shift_i;  
    }

}

void plBoundary::movePointAndNormal( PLuint index, const plVector3 &point, const plVector3 &normal)
{
    _points[index] = point;    
    _normals[index] = normal;
}

void plBoundary::removePointAndNormal( PLuint index )
{
    _points.remove(index);
    _normals.remove(index);
}

void _plSetBoundaryColour()
{
    if (PL_BOUNDARY_NONE_SELECTED || PL_BOUNDARY_CURRENT_IS_SELECTED)
    {
        switch (_plPickingState->type)
        {
            case PL_PICKING_TYPE_DEFECT_CORNERS:
                // defect corners 
                glColor3f( PL_BOUNDARY_DEFECT_CORNER_COLOUR ); 
                break;

            case PL_PICKING_TYPE_DEFECT_BOUNDARY:
                // defect boundary
                glColor3f( PL_BOUNDARY_DEFECT_BOUNDARY_COLOUR ); 
                break;
 
            case PL_PICKING_TYPE_DONOR_BOUNDARY:
                // donor boundary
                glColor3f( PL_BOUNDARY_DONOR_COLOUR );
                break;

            case PL_PICKING_TYPE_IGUIDE_BOUNDARY:     
                // iguide boundary
                glColor3f( PL_BOUNDARY_IGUIDE_COLOUR );   
                break;
        }
    }
    else
    {
        switch (_plPickingState->type)
        {
            case PL_PICKING_TYPE_DEFECT_CORNERS:
                // defect corners 
                glColor3f( PL_BOUNDARY_DEFECT_CORNER_COLOUR_DULL ); 
                break;

            case PL_PICKING_TYPE_DEFECT_BOUNDARY:
                // defect boundary
                glColor3f( PL_BOUNDARY_DEFECT_BOUNDARY_COLOUR_DULL ); 
                break;
 
            case PL_PICKING_TYPE_DONOR_BOUNDARY:
                // donor boundary
                glColor3f( PL_BOUNDARY_DONOR_COLOUR_DULL );
                break;

            case PL_PICKING_TYPE_IGUIDE_BOUNDARY:     
                // iguide boundary
                glColor3f( PL_BOUNDARY_IGUIDE_COLOUR_DULL );   
                break;
        }
    }
}

void plBoundary::updateMesh()
{
    plVector3 n = getAverageNormal();

    plSeq<plVector3> interleaved_vertices( _points.size() * 10 );
    plSeq<PLuint>    indices             ( _points.size() * 6 * 4 );

    for (PLuint i = 0; i < _points.size(); i++)
    {        
        int j = (i+1) % _points.size();  // next index
        int k = (i+2) % _points.size();  // next next index
        int l = (i == 0) ? _points.size()-1 : i-1; // previous index

        // tangent vectors of previous, current, and next segments
        plVector3 t1 = ((_points[j] - _points[i]) ^ n).normalize();
        plVector3 t0 = ((_points[i] - _points[l]) ^ n).normalize();        
        plVector3 t2 = ((_points[k] - _points[j]) ^ n).normalize();           

        // average tangent vectors of current segment end _points
        plVector3 x0 = (t0 + t1).normalize();
        plVector3 x1 = (t1 + t2).normalize();

        plVector3 a = _points[i] + PL_BOUNDARY_MESH_WIDTH_HALF * x0;
        plVector3 b = _points[j] + PL_BOUNDARY_MESH_WIDTH_HALF * x1;
        plVector3 c = b + PL_BOUNDARY_MESH_HEIGHT * n;
        plVector3 d = a + PL_BOUNDARY_MESH_HEIGHT * n;

        plVector3 e = _points[i] - PL_BOUNDARY_MESH_WIDTH_HALF * x0;
        plVector3 f = _points[j] - PL_BOUNDARY_MESH_WIDTH_HALF * x1;
        plVector3 g = f + PL_BOUNDARY_MESH_HEIGHT * n;
        plVector3 h = e + PL_BOUNDARY_MESH_HEIGHT * n;

        PLuint base = interleaved_vertices.size()/2;

        // "right" quad
        interleaved_vertices.add(a);    // position
        interleaved_vertices.add(t1);   // normal
        
        interleaved_vertices.add(b);    // position
        interleaved_vertices.add(t1);   // normal
        
        interleaved_vertices.add(c);    // position
        interleaved_vertices.add(t1);   // normal
        
        interleaved_vertices.add(d);    // position
        interleaved_vertices.add(t1);   // normal
        
        // top ridge 
        interleaved_vertices.add(0.5 * (d+h) + PL_BOUNDARY_MESH_CURVE_HEIGHT * n);    // position
        interleaved_vertices.add(n);    // normal
        
        interleaved_vertices.add(0.5 * (c+g) + PL_BOUNDARY_MESH_CURVE_HEIGHT * n);    // position
        interleaved_vertices.add(n);    // normal       
         
        // "left" quad
        interleaved_vertices.add(e);    // position
        interleaved_vertices.add(-t1);  // normal
        
        interleaved_vertices.add(f);    // position
        interleaved_vertices.add(-t1);  // normal
        
        interleaved_vertices.add(g);    // position
        interleaved_vertices.add(-t1);  // normal
        
        interleaved_vertices.add(h);    // position
        interleaved_vertices.add(-t1);  // normal

        // triangle indices
        indices.add(base+0);    indices.add(base+1);    indices.add(base+2);
        indices.add(base+0);    indices.add(base+2);    indices.add(base+3);
        
        indices.add(base+3);    indices.add(base+2);    indices.add(base+5);
        indices.add(base+3);    indices.add(base+5);    indices.add(base+4);
        
        indices.add(base+4);    indices.add(base+5);    indices.add(base+8);
        indices.add(base+4);    indices.add(base+8);    indices.add(base+9);
        
        indices.add(base+7);    indices.add(base+6);    indices.add(base+9);
        indices.add(base+7);    indices.add(base+9);   indices.add(base+8);   
  
    }

    _mesh.destroy();
    _mesh = plMesh(interleaved_vertices, indices);
}

void plBoundary::drawWalls() const
{  
    _mesh.draw();
}

void plBoundary::draw() const
{        
    if (!_isVisible)
        return;

    if (_points.size() > 0) 
    {
        _plSetBoundaryColour();
        
        // draw walls
        if (_showWalls)
        {
            _plPickingState->index = -1;  // draw walls with index of -1
            _plPickingShader->setPickingUniforms(_plPickingState);
            _mesh.draw();
        }
        
        // draw _points
        for (PLuint i=0; i<_points.size(); i++) 
        {
            _plPickingState->index = i; 
            _plPickingShader->setPickingUniforms(_plPickingState);
            
            if (PL_BOUNDARY_POINT_CURRENT_IS_SELECTED)
            {
                plDrawSphere( _points[i], 1.5) ;
            }
            else
            {
                plDrawSphere( _points[i], 1 );
            }
        }
    }
}

//////////////////////////////////////

PLint plBoundaryGetSelectedType()
{
    return _plState->boundarySelectedType;
}


