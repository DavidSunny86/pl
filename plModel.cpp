#include "plModel.h"

////////////////////////////////////////////////////////
// plModel
////////////////////////////////////////////////////////

plModel::plModel( std::string filename )
{
    if (filename.compare(filename.length()-4, 4, ".stl") != 0)
    {
        std::cout << "plModel error: Unrecognized suffix on filename '"
                  << "'. plModel filenames should have suffix .stl" << std::endl;  		
        return;
    }
   
    plSTLImportFile( _triangles, filename );
    _mesh = plMesh(_triangles);
    _filename = filename;
    _isTransparent = false;
    isVisible = true;
}


void plModel::getMinMax(plVector3 &min, plVector3 &max) const
{
    min = plVector3(FLT_MAX, FLT_MAX, FLT_MAX);
    max = -1 * min;

    for ( PLuint i = 0; i < _triangles.size(); i++)
    {  
        const plVector3 &v = _triangles[i].centroid();

        if (v.x < min.x) min.x = v.x;
        if (v.y < min.y) min.y = v.y;
        if (v.z < min.z) min.z = v.z;

        if (v.x > max.x) max.x = v.x;
        if (v.y > max.y) max.y = v.y;
        if (v.z > max.z) max.z = v.z;
    }
}


plVector3 plModel::getAverageNormal( PLfloat radius, const plVector3 &origin, const plVector3 &up ) const
{
    plVector3 normal(0,0,0);
    PLint count = 0;
    float radiusSquared = radius * radius;  // increasing by factor of two to fruther smooth normal
    
    // Find polygons on top of graft
    for (PLuint i=0; i<_triangles.size(); i++) 
    {
        if (_triangles[i].normal() * up > 0.001)
        {        
            PLfloat dist1 = (_triangles[i].point0() - origin).squaredLength();
            PLfloat dist2 = (_triangles[i].point1() - origin).squaredLength();
            PLfloat dist3 = (_triangles[i].point2() - origin).squaredLength();
           
            // if any point of triangle is withing radial sphere, accept
            float minDist = PL_MIN_OF_3( dist1, dist2, dist3 );

            if (minDist <= radiusSquared)
            {        
                normal = normal + _triangles[i].normal();
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


PLbool _compareOrderPairs( const plOrderPair &a, const plOrderPair &b )
{
    return (a.distance > b.distance);
}


void plModel::draw( const plVector3 &colour ) const
{
    if (!isVisible)
        return;

    if (!_isTransparent) 
    {
        glDisable( GL_STENCIL_TEST );            // if opaque, allow overwriting pixels during picking
        glColor4f( colour.x, colour.y, colour.z, 1.0); 
        _mesh.draw();
    }
    else
    {
        glEnable( GL_STENCIL_TEST );             // if transparent, prevent overwriting pixels during picking
        glColor4f( colour.x, colour.y, colour.z, 0.2);

        // Sort by distance
        plVector3 viewDir = _plCamera->direction();
        
        std::vector<plOrderPair> order;
        order.reserve(_triangles.size());              
        for (PLuint i=0; i<_triangles.size(); i++) 
        {
            order.push_back( plOrderPair(i, _triangles[i].centroid() * viewDir) );
        }
        std::sort(order.begin(), order.end(), _compareOrderPairs);
        
        _mesh.draw(order);
    } 
}


void plModel::toggleVisibility()
{
    if (_isTransparent) 
    {
        isVisible = false;
        _isTransparent = false;
    } 
    else if (!isVisible) 
    {
        isVisible = true;
        _isTransparent = false;
    } 
    else 
    {
        isVisible = true;
        _isTransparent = true;
    }
}


plIntersection plModel::rayIntersect( const plVector3 &start, const plVector3 &dir, PLbool ignoreBehindRay, PLbool backFaceCull ) const        
{
    PLfloat min = FLT_MAX;

    plIntersection closest_intersection(false);

    for ( PLuint i = 0; i < _triangles.size(); i++)
    {  
        plIntersection intersection = _triangles[i].rayIntersect( start, dir, ignoreBehindRay, backFaceCull);
        
        if (intersection.exists)
        {
            if ( fabs(intersection.t) < min) 
            {
                min = intersection.t;
                closest_intersection = intersection;
            }
        }

    }
    return closest_intersection; //(min < FLT_MAX);
}


plString plModel::getFilenameWithoutPath()
{
    plString filenameOnly( _filename );
    plStringStripPreceedingFilepath( filenameOnly );
    return filenameOnly;
}


std::ostream& operator << ( std::ostream& out, const plModel &m )
{
    out << m._filename;
    return out;
}


////////////////////////////////////////////////////////
// plBoneAndCartilage
////////////////////////////////////////////////////////

plBoneAndCartilage::plBoneAndCartilage()
{           
}


plBoneAndCartilage::plBoneAndCartilage( plString bone_file, plString cartilage_file)
    :   bone(bone_file), cartilage(cartilage_file)
{       
}

/*
void plBoneAndCartilage::toggleBoneVisibility()
{
    bone.toggleVisibility();
}


void plBoneAndCartilage::toggleCartilageVisibility()
{
    cartilage.toggleVisibility();
}
*/

void plBoneAndCartilage::draw() const
{
    // DRAW BONE
    /*
    if (bone.isTransparent())
    {
        glEnable( GL_STENCIL_TEST );                // if transparent, prevent overwriting plan pixels during picking
        glColor4f(PL_MODEL_BONE_COLOUR, 0.2);
    }
    else
    {
        glDisable( GL_STENCIL_TEST );
        glColor4f(PL_MODEL_BONE_COLOUR, 1.0);    
    }    
    */
    _plPickingState->type = PL_PICKING_TYPE_BONE;
    _plPickingShader->setPickingUniforms(_plPickingState->type,_plPickingState->id, 0);    
    bone.draw( plVector3(PL_MODEL_BONE_COLOUR) );

    // DRAW CARTILAGE
    /*
    if (cartilage.isTransparent())
    {
        glEnable( GL_STENCIL_TEST );                // if transparent, prevent overwriting plan pixels during picking
        glColor4f(PL_MODEL_CARTILAGE_COLOUR, 0.2);
    }    
    else
    {
        glDisable( GL_STENCIL_TEST );
        glColor4f(PL_MODEL_CARTILAGE_COLOUR, 1.0);  
    }    
    */
    _plPickingState->type = PL_PICKING_TYPE_CARTILAGE;
    _plPickingShader->setPickingUniforms(_plPickingState->type,_plPickingState->id, 0);    
    cartilage.draw( plVector3(PL_MODEL_CARTILAGE_COLOUR) );
}


void plBoneAndCartilage::getMinMax(plVector3 &min, plVector3 &max) const
{
    plVector3 bmin, bmax, cmin, cmax;
    bone.getMinMax(bmin, bmax);
    cartilage.getMinMax(cmin, cmax);
    
    min.x = (bmin.x < cmin.x) ? bmin.x : cmin.x;
    min.y = (bmin.y < cmin.y) ? bmin.y : cmin.y;
    min.z = (bmin.z < cmin.z) ? bmin.z : cmin.z;
    
    max.x = (bmax.x > cmax.x) ? bmax.x : cmax.x;
    max.y = (bmax.y > cmax.y) ? bmax.y : cmax.y;
    max.z = (bmax.z > cmax.z) ? bmax.z : cmax.z;
}     
 
void plBoneAndCartilage::readFromCSV( const plSeq<plString> &row, const plString &directory )
{
    // fill in the field            
    plString subfield = row[2];

    if (plStringCompareCaseInsensitive(subfield, "bone file") )
    {
        bone = plModel( plStringConcat(directory, row[3])  );       // combine directory and filename
    }                        
    else if (plStringCompareCaseInsensitive(subfield, "cartilage file") )
    {
        cartilage = plModel( plStringConcat(directory, row[3])  );  // combine directory and filename
    }        
    else
    {
        std::cerr << "Error importing plan, 'model': Unrecognized word '" << subfield << "' in third column." << std::endl;    
    }    
}

/*
plIntersection plBoneAndCartilage::rayIntersectBone( const plVector3 &start, const plVector3 &dir, PLbool ignoreBehindRay, PLbool backFaceCull) const
{
    return bone.rayIntersect(start, dir, ignoreBehindRay, backFaceCull);   
}


plIntersection plBoneAndCartilage::rayIntersectCartilage( const plVector3 &start, const plVector3 &dir, PLbool ignoreBehindRay, PLbool backFaceCull ) const
{
    return cartilage.rayIntersect(start, dir, ignoreBehindRay, backFaceCull);     
}


plVector3 plBoneAndCartilage::getBoneAverageNormal( PLfloat radius, const plVector3 &origin, const plVector3 &up ) const

{
    return bone.getAverageNormal(radius, origin, up);
}


plVector3 plBoneAndCartilage::getCartilageAverageNormal( PLfloat radius, const plVector3 &origin, const plVector3 &up ) const
{
    return cartilage.getAverageNormal(radius, origin, up);
}
*/

///////////////////////////////////////////
/*
void plModelAdd( plString bone_file, plString cartilage_file )
{
	_plBoneAndCartilageModels.add( new plBoneAndCartilage(bone_file, cartilage_file) );
}


plBoneAndCartilage& plModelGet( PLuint model_id )
{
    if (plErrorCheckModelBounds(model_id, "plModelGet"))
        return *_plBoneAndCartilageModels[0];   
        
    return *_plBoneAndCartilageModels[model_id]; 
}


void plModelRemove( PLuint model_id )
{
    if (plErrorCheckModelBounds(model_id, "plModelRemove"))
        return;

	delete _plBoneAndCartilageModels[model_id]; // deallocate memory
	_plBoneAndCartilageModels.remove(model_id);
}


PLuint plModelCount()
{
	return _plBoneAndCartilageModels.size();
}


void plModelSelect(PLuint model_id)
{
    if (plErrorCheckModelBounds(model_id, "plModelRemove"))
        return;

	_plState->selectModel(model_id); 
}


void plModelBoneToggleVisibility( PLuint model_id )
{
	if (plErrorCheckModelBounds(model_id, "plModelToggleBone"))
        return;

	_plBoneAndCartilageModels[model_id]->toggleBoneVisibility();
}


void plModelCartilageToggleVisibility( PLuint model_id )
{
    if (plErrorCheckModelBounds(model_id, "plModelToggleCartilage"))
        return;

	_plBoneAndCartilageModels[model_id]->toggleCartilageVisibility();
}


plIntersection plModelBoneIntersect(PLuint model_id, const plVector3 &ray_origin, const plVector3 &direction, PLbool ignore_behind_ray, PLbool backface_cull )
{
    if (plErrorCheckModelBounds(model_id, "plModelBoneIntersect"))
        return false;
        
    return _plBoneAndCartilageModels[model_id]->rayIntersectBone(ray_origin, direction, ignore_behind_ray, backface_cull);
}


plIntersection plModelCartilageIntersect(PLuint model_id, const plVector3 &ray_origin, const plVector3 &direction, PLbool ignore_behind_ray, PLbool backface_cull )
{
    if (plErrorCheckModelBounds(model_id, "plModelCartilageIntersect"))
        return false;
        
    return _plBoneAndCartilageModels[model_id]->rayIntersectCartilage(ray_origin, direction, ignore_behind_ray, backface_cull);
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
*/

