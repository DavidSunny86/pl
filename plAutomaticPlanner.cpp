#include "plAutomaticPlanner.h"

plSeq<plSiteGrid> plAutomaticPlanner::_donorSiteGrids;
plSeq<plSiteGrid> plAutomaticPlanner::_defectSiteGrids;

PLuint            plAutomaticPlanner::_siteDataTextureID;

PLuint            plAutomaticPlanner::_overlappingTriangleAreasTextureID; 
PLuint            plAutomaticPlanner::_stateEnergiesTextureID;    
PLuint            plAutomaticPlanner::_stateGraftPositionsTextureID;
PLuint            plAutomaticPlanner::_stateGraftNormalsTextureID;
PLuint            plAutomaticPlanner::_stateGraftRadiiTextureID;
PLuint            plAutomaticPlanner::_stateGraftCountsTextureID;

// DEBUG
plSeq<plVector3>  plAutomaticPlanner::DEBUG_GRAFT_LOCATIONS;

void plAutomaticPlanner::calculate( plPlan &plan )
{
    // generate site grids
    _generateSiteGrids( plan );

    _bufferTextures();
    _dispatch();

} 

void plAutomaticPlanner::_generateSiteGrids( plPlan &plan )
{
    _donorSiteGrids.clear();
    _defectSiteGrids.clear();

    std::cout << "Calculating Plan ... \n";
    
    for ( PLuint i = 0; i < plan.defectSites().size(); i++)
    {   
        std::cout << "\tCalculating defect site grid " << i << " \n";

        plSiteGrid grid( plan.defectSites(i).spline.triangles(), plan.defectSites(i).boundary );
        _defectSiteGrids.add( grid ); 
        
        std::cout << "\t\t" <<  grid.meshSize() << " triangles calculated \n";
        std::cout << "\t\t" <<  grid.size()     << " grid points calculated \n";
    }
    

    for ( PLuint i = 0; i < plan.donorSites().size(); i++)
    {      
        std::cout << "\tCalculating donor site grid " << i << " \n";

        plSiteGrid grid( plan.donorSites(i).model()->cartilage.triangles(), plan.donorSites(i).boundary);
        _donorSiteGrids.add( grid );
        
        std::cout << "\t\t" <<  grid.meshSize() << " triangles calculated \n";
        std::cout << "\t\t" <<  grid.size()     << " grid points calculated \n";
    }    
}


void plAutomaticPlanner::_dispatch()
{    
    PLtime t0 = plTimer::now();

    _dispatchStage0();
    
    PLtime t1 = plTimer::now();
    std::cout << "\nAutomatic planner stage 0 complete:\n";     
    std::cout << "\tCompute shader execution time: " << t1 - t0 << " milliseconds \n";
}


void reportError( const plString &str  ) 
{
    GLuint errnum;
    const char *errstr;
    while (errnum = glGetError()) 
    {
        errstr = reinterpret_cast<const char*> (gluErrorString(errnum));
        std::cout << str << " " << errstr << "\n";
    }
}


void plAutomaticPlanner::_bufferTextures()
{
    plSiteGrid &grid = _defectSiteGrids[0];

    PLuint meshSize = _defectSiteGrids[0].meshSize();     
    PLuint gridSize = _defectSiteGrids[0].size();
      
    plSeq<plVector4> data( (gridSize*2)+(meshSize*4) );    
    for( PLuint i=0; i < gridSize; i++ ) { data.add( _defectSiteGrids[0].points(i)  ); }    
    for( PLuint i=0; i < gridSize; i++ ) { data.add( _defectSiteGrids[0].normals(i) ); }    
    for( PLuint i=0; i < meshSize; i++ ) { data.add( plVector4( _defectSiteGrids[0].triangles(i).point0(), 1.0 ) ); 
                                           data.add( plVector4( _defectSiteGrids[0].triangles(i).point1(), 1.0 ) );
                                           data.add( plVector4( _defectSiteGrids[0].triangles(i).point2(), 1.0 ) );
                                           data.add( plVector4( _defectSiteGrids[0].triangles(i).normal(), 1.0 ) ); }
    
    // buffer site data
    glGenTextures(1, &_siteDataTextureID);
    glBindTexture(GL_TEXTURE_1D, _siteDataTextureID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, (gridSize*2)+(meshSize*4), 0, GL_RGBA, GL_FLOAT, &data[0]);
    reportError("site data");
    
    // triangle overlap texture 
    plSeq<PLfloat> overlappedTriangles(0, meshSize*gridSize);  // fill with 0's

    glGenTextures(1, &_overlappingTriangleAreasTextureID);                              
    glBindTexture(GL_TEXTURE_2D, _overlappingTriangleAreasTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, meshSize, gridSize, 0, GL_RED, GL_FLOAT, &overlappedTriangles[0]);
    reportError("overlappedTriangles");
    
    // state energy output    
    plSeq<PLfloat> stateEnergies(-1, PL_ANNEALING_THREADS);     // fill with -1's
  
    glGenTextures(1, &_stateEnergiesTextureID);                              
    glBindTexture(GL_TEXTURE_1D, _stateEnergiesTextureID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, PL_ANNEALING_THREADS, 0, GL_RED, GL_FLOAT, &stateEnergies[0]);
    reportError("stateEnergies");

    // state graft data
    plSeq<PLfloat> statePoints(-1, PL_MAX_GRAFTS_PER_SOLUTION*PL_ANNEALING_THREADS*4); // fill with -1's    
    plSeq<PLfloat> stateRadii(-1, PL_MAX_GRAFTS_PER_SOLUTION*PL_ANNEALING_THREADS);   // fill with -1's    

    glGenTextures(1, &_stateGraftPositionsTextureID);                              
    glBindTexture(GL_TEXTURE_2D, _stateGraftPositionsTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, PL_MAX_GRAFTS_PER_SOLUTION, PL_ANNEALING_THREADS, 0, GL_RGBA, GL_FLOAT, &statePoints[0]);
    reportError("statePositions");
    
    glGenTextures(1, &_stateGraftNormalsTextureID);                              
    glBindTexture(GL_TEXTURE_2D, _stateGraftNormalsTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, PL_MAX_GRAFTS_PER_SOLUTION, PL_ANNEALING_THREADS, 0, GL_RGBA, GL_FLOAT, &statePoints[0]);
    reportError("stateNormals");
    
    glGenTextures(1, &_stateGraftRadiiTextureID);                              
    glBindTexture(GL_TEXTURE_2D, _stateGraftRadiiTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, PL_MAX_GRAFTS_PER_SOLUTION, PL_ANNEALING_THREADS, 0, GL_RED, GL_FLOAT, &stateRadii[0]);
    reportError("stateRadii");
    
    // fill graft counts
    plSeq<PLuint> stateGraftCounts(-1, PL_ANNEALING_THREADS); // fill with -1's

    glGenTextures(1, &_stateGraftCountsTextureID);                              
    glBindTexture(GL_TEXTURE_1D, _stateGraftCountsTextureID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, PL_ANNEALING_THREADS, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &stateGraftCounts[0]);
    reportError("stateGraftCounts");

    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


float acceptanceProbability( PLfloat energy, PLfloat newEnergy, PLfloat temperature ) 
{
    // If the new solution is better, accept it
    if (newEnergy < energy) 
    {
        return 1.0f;
    }
    // If the new solution is worse, calculate an acceptance probability
    return exp( -(newEnergy - energy) / temperature );
}

void shuffle( plSeq<PLuint> &array )
{   
    PLuint size = array.size();
    for (PLuint i = 0; i < size-1; i++) 
    {
      PLuint j = i + rand() / (RAND_MAX / (size - i) + 1);
      PLuint t = array[j];
      array[j] = array[i];
      array[i] = t;
    }    
}


void plAutomaticPlanner::_dispatchStage0()
{
    PLuint gridSize   = _defectSiteGrids[0].size();
    PLuint meshSize   = _defectSiteGrids[0].meshSize();    
    PLuint workgroups = 1; //ceil( gridSize / (PLfloat) 1024); // ensure enough workgroups are used
    
    // compile / link stage 0 shader
    plPlannerStage0Shader stage0("./shaders/plannerStage0.comp");

    // bind shader
    stage0.bind(); 
    reportError("stage0 bind");

    // bind input/output buffers            
    glBindImageTexture(0, _siteDataTextureID,            0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(1, _overlappingTriangleAreasTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);       
    glBindImageTexture(2, _stateEnergiesTextureID,       0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F    );    
    glBindImageTexture(3, _stateGraftPositionsTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );
    glBindImageTexture(4, _stateGraftNormalsTextureID,   0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );
    glBindImageTexture(5, _stateGraftRadiiTextureID,     0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F    );   
    glBindImageTexture(6, _stateGraftCountsTextureID,    0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI   );       
    reportError("stage0 image tex");    
               
    PLfloat           stateTemperature   (PL_ANNEALING_INITIAL_TEMPERATURE);    
    PLfloat           stateEnergy        (_defectSiteGrids[0].area());       
    PLuint            stateGraftCount    (0);
    plSeq<plVector4>  stateGraftPositions( plVector4(-1,-1,-1,-1), PL_MAX_GRAFTS_PER_SOLUTION );
    plSeq<plVector4>  stateGraftNormals  ( plVector4(-1,-1,-1,-1), PL_MAX_GRAFTS_PER_SOLUTION );
    plSeq<PLfloat>    stateGraftRadii    (-1, PL_MAX_GRAFTS_PER_SOLUTION);

    plSeq<PLuint> indexOrder(PL_ANNEALING_THREADS); for (PLuint i=0; i< PL_ANNEALING_THREADS; i++) indexOrder.add(i);

    plSeq<PLfloat>    energies      ( -1.0, PL_ANNEALING_THREADS );
    plSeq<PLuint>     graftCounts   (-1, PL_ANNEALING_THREADS);             
    plSeq<plVector4>  graftPositions( plVector4(-1,-1,-1,-1), PL_MAX_GRAFTS_PER_SOLUTION*PL_ANNEALING_THREADS);
    plSeq<plVector4>  graftNormals  ( plVector4(-1,-1,-1,-1), PL_MAX_GRAFTS_PER_SOLUTION*PL_ANNEALING_THREADS);
    plSeq<PLfloat>    graftRadii    (-1.0, PL_MAX_GRAFTS_PER_SOLUTION*PL_ANNEALING_THREADS);
              
    while (stateTemperature > 0.01f)
    {
        // set uniforms
        stage0.setAnnealingUniforms( _defectSiteGrids[0].meshSize(), 
                                     _defectSiteGrids[0].area(), 
                                     _defectSiteGrids[0].size(),
                                     stateTemperature,
                                     stateEnergy,
                                     stateGraftCount,
                                     stateGraftPositions,
                                     stateGraftNormals,
                                     stateGraftRadii );                                     
  
        reportError("stage0 uniforms");

        // call compute shader with 1D workgrouping
        glDispatchCompute( workgroups, 1, 1 );
        
        // memory barrier
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);       

        reportError("stage0 mem barr");  
                       
        // read energies into array            
        glBindTexture(GL_TEXTURE_1D, _stateEnergiesTextureID);    
        glGetTexImage(GL_TEXTURE_1D, 0, GL_RED, GL_FLOAT, &energies[0]);            
        reportError("stage0 get energies");   
                
        //std::cout << "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\nAnnealing temperature: " << stateTemperature << "\n";             
        
        // iterate through all state energies and find best one
        shuffle( indexOrder );
        PLint bestIndex = -1;        
        for (PLuint i=0; i < PL_ANNEALING_THREADS; i++ )
        {    
            PLuint j = indexOrder[i];          
            PLfloat r = ((float) rand() / (RAND_MAX));              
            if ( acceptanceProbability( stateEnergy, energies[j], stateTemperature ) > r )
            {    
                stateEnergy = energies[j];
                bestIndex = j;
            }
        }
        
        if ( bestIndex != -1 )
        {
            // state positions
            glBindTexture(GL_TEXTURE_2D, _stateGraftPositionsTextureID);    
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &graftPositions[0]); 
            reportError("stage0 get positions");
            
            // state normals
            glBindTexture(GL_TEXTURE_2D, _stateGraftNormalsTextureID);    
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &graftNormals[0]); 
            reportError("stage0 get normals");
            
            // state radii
            glBindTexture(GL_TEXTURE_2D, _stateGraftRadiiTextureID);    
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &graftRadii[0]); 
            reportError("stage0 get radii");     

            // state count
            glBindTexture(GL_TEXTURE_1D, _stateGraftCountsTextureID);    
            glGetTexImage(GL_TEXTURE_1D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &graftCounts[0]); 
            reportError("stage0 get count");
                           
            PLuint offset = bestIndex*PL_MAX_GRAFTS_PER_SOLUTION;
            stateGraftCount = graftCounts[bestIndex];

            for (PLuint i=0; i < stateGraftCount; i++)
            {
                stateGraftPositions[i] = graftPositions[offset+i];
                stateGraftNormals[i]   = graftNormals  [offset+i];
                stateGraftRadii[i]     = graftRadii    [offset+i];                
            }
              
            std::cout << "New Best energy: " << stateEnergy << ", " << stateGraftCount << " grafts, Annealing temperature: " << stateTemperature << "\n";" \n";
            
        }

        // reset covered areas back to 0
        
        plSeq<PLfloat> overlappingTriangles(0, meshSize*PL_ANNEALING_THREADS);  
		glBindTexture(GL_TEXTURE_2D, _overlappingTriangleAreasTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, meshSize, PL_ANNEALING_THREADS, 0, GL_RED, GL_FLOAT, &overlappingTriangles[0]);
		
        stateTemperature *= 1.0f-PL_ANNEALING_COOLING_RATE;
    }

    for (PLuint i=0; i < stateGraftCount; i++)
    {
        plVector4 p = stateGraftPositions[i];
        plVector4 n = stateGraftNormals[i];
        
        DEBUG_GRAFT_LOCATIONS.add( plVector3( p.x, p.y, p.z ) );                                              
        DEBUG_GRAFT_LOCATIONS.add( plVector3( n.x, n.y, n.z ) );
    }

}

