#include "plModel.h"

////////////////////////////////////////////////////////
// plModel
////////////////////////////////////////////////////////

plModel::plModel( std::string filename )
{

    if (filename.compare(filename.length()-4, 4, ".stl") != 0)
    {
        std::cerr << "plModel error: Unrecognized suffix on filename '" << filename 
                  << "'. plModel filenames should have suffix .stl" << std::endl;  		
        return;
    }

    plReadSTLFile( _triangles, filename );

    _mesh = plMesh(_triangles);
    
    _isTransparent = false;
    _isVisible = true;
}


void plModel::getMinMax(plVector3 &min, plVector3 &max) const
{
    min = plVector3(FLT_MAX, FLT_MAX, FLT_MAX);
    max = -1 * min;

    for ( PLint i = 0; i < _triangles.size(); i++)
    {  
        const plVector3 &v = _triangles[i].centroid;

        if (v.x < min.x) min.x = v.x;
        if (v.y < min.y) min.y = v.y;
        if (v.z < min.z) min.z = v.z;

        if (v.x > max.x) max.x = v.x;
        if (v.y > max.y) max.y = v.y;
        if (v.z > max.z) max.z = v.z;
    }
}


plVector3 plModel::getAverageNormal( PLfloat radius, const plVector3 &origin, const plVector3 &up )
{
    plVector3 normal(0,0,0);
    PLint count = 0;
    float radiusSquared = radius * radius;  // increasing by factor of two to fruther smooth normal
    
    // Find polygons on top of graft
    for (PLint i=0; i<_triangles.size(); i++) 
    {
        if (_triangles[i].normal * up > 0.001)
        {        
            PLfloat dist1 = (_triangles[i].point1 - origin).squaredLength();
            PLfloat dist2 = (_triangles[i].point2 - origin).squaredLength();
            PLfloat dist3 = (_triangles[i].point3 - origin).squaredLength();
           
            // if any point of triangle is withing radial sphere, accept
            float minDist = PL_MIN_OF_3( dist1, dist2, dist3 );

            if (minDist <= radiusSquared)
            {        
                normal = normal + _triangles[i].normal;
                count++;
            }
        }
    } 

    if (count == 0)
    {
        // no triangles in radial sphere, just assume previous normal, (this can be bad.....)
        std::cout << "No normal found\n";
        return up;
    }    

    return (1.0f/(PLfloat)(count) * normal).normalize();
}


PLbool compareOrderPairs( const plOrderPair &a, const plOrderPair &b )
{
    return (a.distance > b.distance);
}


void plModel::draw() const
{
    if (!_isVisible)
        return;

    if (!_isTransparent) 
    {
        _mesh.draw();
    }
    else
    {
        // Sort by distance
        plVector3 viewDir = plCameraGetDirection();
        
        std::vector<plOrderPair> order;
        order.reserve(_triangles.size());              
        for (unsigned int i=0; i<_triangles.size(); i++) 
        {
            order.push_back( plOrderPair(i, _triangles[i].centroid * viewDir) );
        }
        std::sort(order.begin(), order.end(), compareOrderPairs);
        
        _mesh.draw(order);
    }
  
}


void plModel::toggleVisibility()
{
    if (_isTransparent) 
    {
        _isVisible = false;
        _isTransparent = false;
    } 
    else if (!_isVisible) 
    {
        _isVisible = true;
        _isTransparent = false;
    } 
    else 
    {
        _isVisible = true;
        _isTransparent = true;
    }
}


PLbool plModel::rayIntersect( plVector3 &intPoint, plVector3 &intNorm, const plVector3 &start, const plVector3 &dir, PLbool ignoreBehindRay, PLbool backFaceCull ) const
        
{
    PLfloat min = FLT_MAX;

    plVector3 p, n;
    PLfloat t;

    for ( PLint i = 0; i < _triangles.size(); i++)
    {  
        if (_triangles[i].rayIntersect( p, n, t, start, dir, ignoreBehindRay, backFaceCull))
        {
            if (fabs(t) < min) 
            {
                min = fabs(t);
                intPoint = p;
                intNorm = n;
            }
        }

    }
    return (min < FLT_MAX);
}

////////////////////////////////////////////////////////
// plBoneAndCartilage
////////////////////////////////////////////////////////

plBoneAndCartilage::plBoneAndCartilage( plString bone_file, plString cartilage_file)
    :   _bone(bone_file), _cartilage(cartilage_file)
{
}


void plBoneAndCartilage::toggleBoneVisibility()
{
    _bone.toggleVisibility();
}


void plBoneAndCartilage::toggleCartilageVisibility()
{
    _cartilage.toggleVisibility();
}


void plBoneAndCartilage::draw() const
{
    // DRAW BONE
    if (_bone.isTransparent())
    {
        glEnable( GL_STENCIL_TEST );                // if transparent, prevent overwriting plan pixels during picking
        glColor4f(PL_MODEL_BONE_COLOUR, 0.2);
    }
    else
    {
        glDisable( GL_STENCIL_TEST );
        glColor4f(PL_MODEL_BONE_COLOUR, 1.0);    
    }    
    _plPickingState->type = PL_PICKING_TYPE_BONE;
    _plPickingShader->setPickingUniforms(_plPickingState->type,_plPickingState->id, 0);    
    _bone.draw();
    
    // DRAW CARTILAGE
    if (_cartilage.isTransparent())
    {
        glEnable( GL_STENCIL_TEST );                // if transparent, prevent overwriting plan pixels during picking
        glColor4f(PL_MODEL_CARTILAGE_COLOUR, 0.2);
    }    
    else
    {
        glDisable( GL_STENCIL_TEST );
        glColor4f(PL_MODEL_CARTILAGE_COLOUR, 1.0);  
    }    
    _plPickingState->type = PL_PICKING_TYPE_CARTILAGE;
    _plPickingShader->setPickingUniforms(_plPickingState->type,_plPickingState->id, 0);    
    _cartilage.draw();
}


void plBoneAndCartilage::getMinMax(plVector3 &min, plVector3 &max) const
{
    plVector3 bmin, bmax, cmin, cmax;
    _bone.getMinMax(bmin, bmax);
    _cartilage.getMinMax(cmin, cmax);
    
    min.x = (bmin.x < cmin.x) ? bmin.x : cmin.x;
    min.y = (bmin.y < cmin.y) ? bmin.y : cmin.y;
    min.z = (bmin.z < cmin.z) ? bmin.z : cmin.z;
    
    max.x = (bmax.x > cmax.x) ? bmax.x : cmax.x;
    max.y = (bmax.y > cmax.y) ? bmax.y : cmax.y;
    max.z = (bmax.z > cmax.z) ? bmax.z : cmax.z;
}     
   
   
PLbool plBoneAndCartilage::rayIntersectBone( plVector3 &intPoint, plVector3 &intNorm, const plVector3 &start, const plVector3 &dir, PLbool ignoreBehindRay, PLbool backFaceCull ) const
{
    return _bone.rayIntersect(intPoint, intNorm, start, dir, ignoreBehindRay, backFaceCull);   
}


PLbool plBoneAndCartilage::rayIntersectCartilage( plVector3 &intPoint, plVector3 &intNorm, const plVector3 &start, const plVector3 &dir, PLbool ignoreBehindRay, PLbool backFaceCull ) const
{
    return _cartilage.rayIntersect(intPoint, intNorm, start, dir, ignoreBehindRay, backFaceCull);     
}


plVector3 plBoneAndCartilage::getBoneAverageNormal( PLfloat radius, const plVector3 &origin, const plVector3 &up )

{
    return _bone.getAverageNormal(radius, origin, up);
}


plVector3 plBoneAndCartilage::getCartilageAverageNormal( PLfloat radius, const plVector3 &origin, const plVector3 &up )
{
    return _cartilage.getAverageNormal(radius, origin, up);
}

///////////////////////////////

void plModelAdd( plString bone_file, plString cartilage_file )
{
	_plBoneAndCartilageModels.add( new plBoneAndCartilage(bone_file, cartilage_file) );
}


void plModelRemove( PLuint model_id )
{
    if (plErrorCheckModelBounds(model_id, "plModelRemove"))
        return;

	delete _plBoneAndCartilageModels[model_id]; // deallocate memory
	_plBoneAndCartilageModels.remove(model_id);
}


PLuint plModelGetCount()
{
	return _plBoneAndCartilageModels.size();
}


void plModelSelect(PLuint model_id)
{
    if (plErrorCheckModelBounds(model_id, "plModelRemove"))
        return;

	_plState->selectModel(model_id); 
}


void plModelToggleBone( PLuint model_id )
{
	if (plErrorCheckModelBounds(model_id, "plModelToggleBone"))
        return;

	_plBoneAndCartilageModels[model_id]->toggleBoneVisibility();
}


void plModelToggleCartilage( PLuint model_id )
{
    if (plErrorCheckModelBounds(model_id, "plModelToggleCartilage"))
        return;

	_plBoneAndCartilageModels[model_id]->toggleCartilageVisibility();
}


void plModelDraw( PLuint model_id )
{
    if (plErrorCheckModelBounds(model_id, "plModelDraw"))
        return;
        
	_plBoneAndCartilageModels[model_id]->draw();
}


void plModelDrawAll()
{
	for (PLint i = 0; i < plModelGetCount(); i++)
	{
		plModelDraw(i);
	}	
}


PLbool plModelBoneIntersect(plVector3 &point, plVector3 &normal, PLuint model_id, const plVector3 &ray_origin, const plVector3 &direction, PLbool ignore_behind_ray, PLbool backface_cull )
{
    if (plErrorCheckModelBounds(model_id, "plModelBoneIntersect"))
        return false;
        
    return _plBoneAndCartilageModels[model_id]->rayIntersectBone(point, normal, ray_origin, direction, ignore_behind_ray, backface_cull);
}


PLbool plModelCartilageIntersect(plVector3 &point, plVector3 &normal, PLuint model_id, const plVector3 &ray_origin, const plVector3 &direction, PLbool ignore_behind_ray, PLbool backface_cull )
{
    if (plErrorCheckModelBounds(model_id, "plModelCartilageIntersect"))
        return false;
        
    return _plBoneAndCartilageModels[model_id]->rayIntersectCartilage(point, normal, ray_origin, direction, ignore_behind_ray, backface_cull);
}
	
	
plVector3 plModelBoneGetAvgNormal( PLuint model_id, PLfloat radius, const plVector3 &origin, const plVector3 &up )
{
    if (plErrorCheckModelBounds(model_id, "plModelBoneGetAvgNormal"))
        return up;
           
    return _plBoneAndCartilageModels[model_id]->getBoneAverageNormal( radius, origin, up);           
}


plVector3 plModelCartilageGetAvgNormal( PLuint model_id, PLfloat radius, const plVector3 &origin, const plVector3 &up )
{
    if (plErrorCheckModelBounds(model_id, "plModelCartilageGetAvgNormal"))
        return up;
        
    return _plBoneAndCartilageModels[model_id]->getCartilageAverageNormal( radius, origin, up);
}


