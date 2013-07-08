#ifndef __PL_PLAN_H__
#define __PL_PLAN_H__

#include "pl.h"
#include "plString.h"
#include "plSeq.h"
#include "plVector3.h"
#include "plCSV.h"
#include "plRenderable.h"

#include "plGraft.h"
#include "plModel.h"
#include "plBoundary.h"
#include "plDefectSite.h"
#include "plDonorSite.h"
#include "plIGuide.h"

class plPlan : public plRenderable
{
    public:

        plString date;
        plString directory; 
   
        plPlan( plString filename );        
        plPlan( int argc, char **argv ); 
        
        void importFile( plString filename );        
        void exportFile( plString filename );
               
        void draw( PLbool picking = false );
        
        //void outputForManuela();
        
        friend std::ostream& operator << ( std::ostream& out, const plPlan &p );
        
    //private: eventually make these member variables private
    
        plSeq<plDefectSite>       _defectSites;        
        plSeq<plDonorSite>        _donorSites;
        plSeq<plIGuide>           _iGuides;                
        plSeq<plGraft>            _grafts;        
        plSeq<plBoneAndCartilage> _models;     
};

std::ostream& operator << ( std::ostream& out, const plPlan &p );

#endif

