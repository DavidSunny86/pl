#include "plIGuide.h"

plIGuide::plIGuide() 
    : site( NULL ) 
{
    toolDepth = 43.5;
}


plIGuide::plIGuide( plIGuideSite *s, PLuint sid, const std::vector<plPlugInfo> &p, const std::vector<plKWire*> &k, const std::vector<PLuint> &kids, const std::vector<const plSpline*> splns, std::vector<PLuint> &dids )
    : site( s ), siteID( sid ), plugs( p ), kWires( k ), kWireIDs( kids ), splines(splns), defectIDs(dids)
{
    toolDepth = 43.5;
}


PLbool plIGuide::generateIGuideModels()
{
    const PLuint PL_OCTREE_DEPTH_IGUIDE_MODELS = 1;

    iGuideModelsToAdd.clear();
    iGuideModelsToSubtract.clear();

    if ( !site )
    {
        std::cerr << "Error: No site chosen for iGuide. Aborting." << std::endl;
        return false;
    }
    
    if ( site->boundary.size() <= 2 )
    {
        std::cerr << "Error: Boundary does not have enough points. Aborting." << std::endl;
        return false;
    }

    std::string anatomyFilename = _prepareFilenameWithVariables( false, 'M', 0, "bone" );
    iGuideModelsToSubtract.push_back( new plModel( site->boundary.mesh().triangles(), anatomyFilename, PL_OCTREE_DEPTH_IGUIDE_MODELS ) );

    // template base TODO: recreate the template base shape ONLY if it needs updating

    // generate template base
    if ( site->generateTemplateBase() )
    {
        // if successful
        std::string templateBaseFilename = _prepareFilenameWithVariables( true , 'M', 0, "templateBase" );
        iGuideModelsToAdd.push_back( new plModel( site->templateBase().triangles(), templateBaseFilename, PL_OCTREE_DEPTH_IGUIDE_MODELS) );
    }


    // generate base over the defect site
    PLfloat baseTranslationDistance     = 1.01f;
    PLfloat baseThickness               = 5.00f;
    PLfloat interiorTranslationDistance = -10.0f;
    PLfloat interiorThickness           = 11.0f;
    
    std::cout << "000" << std::endl;
    
    for ( PLuint i = 0; i < splines.size(); i++ )
    {
        std::cout << "001" << std::endl;
        // create the base shape over the defect site
        std::string             baseOverDefectFilename  = _prepareFilenameWithVariables(true ,'S',i,"templateBase"); 
        
        std::cout << "0011" << std::endl;   
         
        plMatrix44 baseTranslation;  
        baseTranslation.setTranslation( baseTranslationDistance * splines[i]->getAverageNormal() );


        plMesh baseOverDefect = plMeshExtruder::extrudeMesh( baseTranslation * splines[i]->mesh(),
                                                                                       baseThickness, 
                                                                                       splines[i]->getAverageNormal() );
          std::cout << "002" << std::endl;      
        iGuideModelsToAdd.push_back( new plModel( baseOverDefect.triangles(), 
                                                  baseOverDefectFilename, 
                                                  PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
        
        // create the defect site volume
        std::string             interiorDefectFilename  = _prepareFilenameWithVariables(false ,'M',i,"defectInterior");
        
        plMatrix44 interiorTranslation;  
        interiorTranslation.setTranslation( interiorTranslationDistance * splines[i]->getAverageNormal() );

        
        plMesh interiorDefect = plMeshExtruder::extrudeMesh( interiorTranslation * splines[i]->mesh(),
                                                                                       interiorThickness,  
                                                                                       splines[i]->getAverageNormal() );
                                                               
        iGuideModelsToSubtract.push_back( new plModel( interiorDefect.triangles(), 
                                                       interiorDefectFilename, 
                                                       PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
                                                       
        std::cout << "00" << std::endl;
    }

    std::cout << "0" << std::endl;

    // plug pieces
    std::vector<plTriangle> roundCylinder;
    plSTL::importFile( roundCylinder, "./iGuideElements/Generator_Cylinder_Round.stl" );

    std::vector<plTriangle> sharpCylinder;
    plSTL::importFile( sharpCylinder, "./iGuideElements/Generator_Cylinder_Sharp.stl" );

    std::vector<plTriangle> correctCylinder; // correction cylinder
    plSTL::importFile( correctCylinder, "./iGuideElements/Generator_Cylinder_Correction.stl" );

    std::vector<plTriangle> keyCube;
    plSTL::importFile( keyCube, "./iGuideElements/Generator_Key_Cube.stl" );

    std::cout << "1" << std::endl;

    for ( PLuint i = 0; i < plugs.size(); i++ )
    {   
        plMatrix44 cylinderToPlugTransform = plugs[i].transform().matrix();
        
        //plVector4  plugToCartilageSurfaceTranslation = plVector4( 0.0f, 0.0f, 0.0f, 0.0f );
        //plMatrix44 plugToCartilageSurfaceTransform( plugToCartilageSurfaceTranslation.x, plugToCartilageSurfaceTranslation.y, plugToCartilageSurfaceTranslation.z );
        //plMatrix44 cylinderToCartilageSurfaceTransform = plugToCartilageSurfaceTransform * cylinderToPlugTransform;
        
        PLfloat   keyTranslation = 6.f;
        PLfloat   keyRotation    = 0.f;
        
        //if ( plugs[i].type == PL_PICKING_INDEX_GRAFT_DONOR )

        PLfloat holeDiameter    = plugs[i].radius();
        PLfloat sleeveDiameter  = holeDiameter   + 2.f ;
        PLfloat baseDiameter    = sleeveDiameter + 1.f ;
        PLfloat holderDiameter  = baseDiameter   + 4.f ;
        PLfloat correctDiameter = holderDiameter + 0.1f;

        PLfloat sleeveHeight = toolDepth - plugs[i].length(); // - plugs[i].cartilageThickness();
        
        plVector3 holeScale   ( holeDiameter,   36.f, holeDiameter   );
        plVector3 sleeveScale ( sleeveDiameter, sleeveHeight, sleeveDiameter );
        plVector3 baseScale   ( baseDiameter,   20.f, baseDiameter   );
        plVector3 holderScale ( holderDiameter, 10.f, holderDiameter );
        plVector3 keyScale    ( 3.f, 29.9f, 4.f );
        plVector3 correctScale( correctDiameter, baseThickness - 1.f, correctDiameter   );

        //if ( plugs[i].type == PL_PICKING_INDEX_GRAFT_DONOR )
            //plVector3 supportScale( 18.f,  4.f, 18.f  );
        std::cout << "2" << std::endl;
        std::vector<plTriangle> holeTriangles    = _createTemplatePieceTransformed( sharpCylinder,   cylinderToPlugTransform, 0.0f, holeScale,    0.0f,           0.0f );
        std::vector<plTriangle> sleeveTriangles  = _createTemplatePieceTransformed( roundCylinder,   cylinderToPlugTransform, 0.0f, sleeveScale,  0.0f,           0.0f );
        std::vector<plTriangle> baseTriangles    = _createTemplatePieceTransformed( sharpCylinder,   cylinderToPlugTransform, 0.0f, baseScale,    0.0f,           0.0f );
        std::vector<plTriangle> holderTriangles  = _createTemplatePieceTransformed( roundCylinder,   cylinderToPlugTransform, 0.0f, holderScale,  0.0f,           0.0f );
        std::vector<plTriangle> keyTriangles     = _createTemplatePieceTransformed( keyCube,         cylinderToPlugTransform, 0.0f, keyScale,     keyTranslation, 0.0f );
        std::vector<plTriangle> correctTriangles = _createTemplatePieceTransformed( correctCylinder, cylinderToPlugTransform, 0.0f, correctScale, 0.0f,           0.0f );

        //if (plugs[i].type == PL_PICKING_INDEX_GRAFT_DONOR)
            //std::vector<plTriangle> supportTriangles( createTemplatePieceTransformed(sharpCylinder, recipientTransform, cylinderRecipientSupportOffset, supportScale, 0.0f, 0.0f) );

        PLchar typeLetter = ( plugs[i].type() == PL_PICKING_INDEX_GRAFT_DONOR ) ? 'H' : 'R';
            
        plString holeFilename    = _prepareFilenameWithVariables( PL_IGUIDE_BOOLEAN_MESH_DIFFERENCE   , typeLetter, plugs[i].graftID(), "hole"      );
        plString sleeveFilename  = _prepareFilenameWithVariables( PL_IGUIDE_BOOLEAN_MESH_UNION        , typeLetter, plugs[i].graftID(), "sleeve"     );
        plString baseFilename    = _prepareFilenameWithVariables( true , typeLetter, plugs[i].graftID(), "base"      );
        plString holderFilename  = _prepareFilenameWithVariables( PL_IGUIDE_BOOLEAN_MESH_UNION        , typeLetter, plugs[i].graftID(), "holder"     );
        plString keyFilename     = _prepareFilenameWithVariables( PL_IGUIDE_BOOLEAN_MESH_UNION        , typeLetter, plugs[i].graftID(), "key"        );
        plString correctFilename = _prepareFilenameWithVariables( PL_IGUIDE_BOOLEAN_MESH_INTERSECTION , typeLetter, plugs[i].graftID(), "correction" );

        iGuideModelsToSubtract.push_back( new plModel( holeTriangles   , holeFilename   , PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
        iGuideModelsToAdd.push_back     ( new plModel( sleeveTriangles , sleeveFilename , PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
        iGuideModelsToAdd.push_back     ( new plModel( baseTriangles   , baseFilename   , PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
        iGuideModelsToAdd.push_back     ( new plModel( holderTriangles , holderFilename , PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
        iGuideModelsToAdd.push_back     ( new plModel( keyTriangles    , keyFilename    , PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
        //iGuideModelsToSubtract.push_back( new plModel( correctTriangles, correctFilename, PL_OCTREE_DEPTH_IGUIDE_MODELS ) );
        //if (plugs[i].type() == PL_PICKING_INDEX_GRAFT_DONOR)
        std::cout << "3" << std::endl;
    }
    
    return true;
}


std::vector<plTriangle> plIGuide::_translateTriangles( const std::vector< plTriangle >& triangles, const plVector3& translation )
{
    std::vector<plTriangle> translatedTriangles;
    
    for ( const plTriangle& triangle : triangles )
    {   
        translatedTriangles.push_back( plTriangle( triangle.normal(), 
                                                   triangle.point0() + translation, 
                                                   triangle.point1() + translation, 
                                                   triangle.point2() + translation ) );
    }
    
    return translatedTriangles;
}


PLbool plIGuide::exportIGuideModels( const std::string &directory ) 
{
    if ( iGuideModelsToAdd.empty() && iGuideModelsToSubtract.empty() )
    {
        std::cerr << "Error: No IGuide pieces to export. Did you forget to generate them? Aborting." << std::endl;
        return false;
    }
    // TODO: Compare the number of meshes to what we expect based on the sizes of the various arrays, output a WARNING if it isn't what we expect.

    for (PLuint i = 0; i < iGuideModelsToAdd.size(); i++)
    {
        plSTL::exportFileBinary( iGuideModelsToAdd[i]->mesh().triangles(), ( directory + iGuideModelsToAdd[i]->filename ) );
    }
    for (PLuint i = 0; i < iGuideModelsToSubtract.size(); i++)
    {
        plSTL::exportFileBinary( iGuideModelsToSubtract[i]->mesh().triangles(), ( directory + iGuideModelsToSubtract[i]->filename ) );
    }
    return true;
}


std::string plIGuide::_prepareFilenameWithVariables( PLint operation, PLchar type, PLint graftIndex, const std::string &pieceName )
{
    std::stringstream strstream;
    strstream << type << graftIndex;
    switch (operation) // TODO: a DEFINE or enum or something
    {
        case PL_IGUIDE_BOOLEAN_MESH_DIFFERENCE:
            strstream << "Difference";
            break;
        case PL_IGUIDE_BOOLEAN_MESH_UNION:
            strstream << "Union";
            break;
        case PL_IGUIDE_BOOLEAN_MESH_INTERSECTION:
            strstream << "Intersect";
            break;
    }
    strstream << pieceName << ".stl";
    return strstream.str();
}


std::vector<plTriangle> plIGuide::_createTemplatePieceTransformed ( const std::vector<plTriangle>& baseTriObject,
                                                              const plMatrix44& plugTransform,
                                                              const PLfloat&    zOffset,
                                                              const plVector3&  scale,
                                                              const PLfloat&    keyTranslationXAxis,
                                                              const PLfloat&    keyRotationZAxis )
{
    plMatrix44 zTranslationMatrix;    zTranslationMatrix.setTranslation(0.0f, zOffset, 0.0f);
    plMatrix44 scaleMatrix;           scaleMatrix.setScale(scale);
    plMatrix44 keyTranslationMatrix;  keyTranslationMatrix.setTranslation(keyTranslationXAxis, 0.0f, 0.0f);
    plMatrix44 keyRotationMatrix;     keyRotationMatrix.setRotationD(keyRotationZAxis, plVector3(0,1,0));
    plMatrix44 transformationMatrix = plugTransform * zTranslationMatrix * keyRotationMatrix * keyTranslationMatrix * scaleMatrix;

    std::vector<plTriangle> outputTriObject;
    for (PLuint i=0; i<baseTriObject.size(); i++)
    {
        outputTriObject.push_back( transformationMatrix * baseTriObject[i] );
    }
    return outputTriObject;
}


void plIGuide::clearIGuideModels ()
{
    for (PLuint i = 0; i < iGuideModelsToAdd.size(); i++)
    {
        delete iGuideModelsToAdd[i];
    }
    iGuideModelsToAdd.clear();
    for (PLuint i = 0; i < iGuideModelsToSubtract.size(); i++)
    {
        delete iGuideModelsToSubtract[i];
    }
    iGuideModelsToSubtract.clear();
}

/*
void plIGuide::draw() 
{
    for (PLuint i = 0; i < iGuideModelsToAdd.size(); i++)
    {
        iGuideModelsToAdd[i]->draw( plVector3( 0.75f,0.75f,1.0f ) );
    }
    for (PLuint i = 0; i < iGuideModelsToSubtract.size(); i++)
    {
        iGuideModelsToSubtract[i]->draw( plVector3( 1.0f,0.75f,0.75f ) );
    }
    //plPicking::value.type = PL_PICKING_TYPE_IGUIDE_BOUNDARY;
    //boundary.draw();
}
*/

void plIGuide::toggleVisibility()
{
    for (PLuint i = 0; i < iGuideModelsToAdd.size(); i++)
    {
        iGuideModelsToAdd[i]->toggleVisibility();
    }
    for (PLuint i = 0; i < iGuideModelsToSubtract.size(); i++)
    {
        iGuideModelsToSubtract[i]->toggleVisibility();
    }
}
