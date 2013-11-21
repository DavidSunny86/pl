#include "plDonorSite.h" 

plDonorSite::plDonorSite() 
{
}


plDonorSite::plDonorSite( PLuint modelID, const plBoneAndCartilage& model )
    : plModelSpecific( modelID, model )
{
}


plDonorSite::plDonorSite( PLuint modelID, const plBoneAndCartilage& model, const plBoundary &b  )
    : plModelSpecific( modelID, model ), boundary( b )
{
}


void plDonorSite::extractRenderComponents( plRenderMap& renderMap ) const
{
    if ( !_isVisible )
        return;
      
    // draw spline boundary 
    plPickingStack::loadRed( PL_PICKING_TYPE_DONOR_BOUNDARY );
    boundary.extractRenderComponents( renderMap ); 
}


/*
void plDonorSite::draw() const
{      
    if ( !_isVisible )
        return;
      
    // draw spline boundary 
    plPickingStack::loadRed( PL_PICKING_TYPE_DONOR_BOUNDARY );
    boundary.draw();   
   
}
*/










