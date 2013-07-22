#ifndef __PL_AUTOMATIC_PLANNER_H__
#define __PL_AUTOMATIC_PLANNER_H__

#include "plCommon.h"
#include "plSeq.h"
#include "plVector3.h"
#include "plTransform.h"
#include "plPlan.h"

class plSiteGrid
{
    public:
    
        plSeq<plVector3>  points; 
        plSeq<plVector3>  normals;
};


class plAutomaticPlanner
{
    public:

        static void calculate( plPlan &plan );
               
    //private:
    
        plAutomaticPlanner();
                
        static plSeq<plSiteGrid>  _donorSiteGrids;
        static plSeq<plSiteGrid>  _defectSiteGrids;
          
        static void _createGrid ( plSiteGrid &grid, const plSeq<plTriangle> &triangles );  
                        
};

#endif

