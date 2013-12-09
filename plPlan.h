#ifndef PL_PLAN_H
#define PL_PLAN_H

#include "plCommon.h"
#include "plString.h"

#include "plVector3.h"
#include "plCSV.h"
#include "plRenderable.h"
#include "plGraft.h"
#include "plModel.h"
#include "plDefectSite.h"
#include "plDonorSite.h"
#include "plIGuideSite.h"
#include "plIGuide.h"

class plPlan : public plRenderable,
               public plArthroViewable
{
    public:

        plPlan( plString filename );        
        plPlan( int argc, char **argv ); 
        
        ~plPlan();

        void extractRenderComponents( plRenderMap& renderMap ) const;
        void extractRenderComponents( plRenderMap& renderMap, PLuint technique ) const;

        void importFile( const plString &filename );        
        void exportFile( const plString &filename );

        // non-const reference getters          
        plDefectSite& defectSites( PLuint index ) { return *_defectSites[index]; }
        plDonorSite&  donorSites ( PLuint index ) { return *_donorSites[index];  }
        plIGuideSite& iGuideSites( PLuint index ) { return *_iGuideSites[index]; }
        plIGuide&     iGuides    ( PLuint index ) { return *_iGuides[index];     }
        plGraft&      grafts     ( PLuint index ) { return *_grafts[index];      }   
        plModel&      models     ( PLuint index ) { return *_models[index];      }
           
        // const reference getters      
        const plDefectSite& defectSites( PLuint index ) const { return *_defectSites[index]; }
        const plDonorSite&  donorSites ( PLuint index ) const { return *_donorSites[index];  }
        const plIGuideSite& iGuideSites( PLuint index ) const { return *_iGuideSites[index]; }
        const plIGuide&     iGuides    ( PLuint index ) const { return *_iGuides[index];     }
        const plGraft&      grafts     ( PLuint index ) const { return *_grafts[index];      }     
        const plModel&      models     ( PLuint index ) const { return *_models[index];      }
        
        // const array getters (gives access to array without allowing control to add, remove, etc)
        const std::vector<plDefectSite*>& defectSites() const { return _defectSites; }
        const std::vector<plDonorSite*>&  donorSites()  const { return _donorSites;  }
        const std::vector<plIGuideSite*>& iGuideSites() const { return _iGuideSites; }
        const std::vector<plIGuide*>&     iGuides()     const { return _iGuides;     }
        const std::vector<plGraft*>&      grafts()      const { return _grafts;      }
        const std::vector<plModel*>&      models()      const { return _models;      }
        
        void addDefectSite( PLuint modelIndex = 0 );
        void addDonorSite ( PLuint modelIndex = 0 );
        void addIGuideSite( PLuint modelIndex = 0 );
        void addGraft     ( const plPlug& harvest, const plPlug& recipient, PLfloat radius, PLfloat length = 15.0f );

        void removeDefectSite( PLuint index );
        void removeDonorSite ( PLuint index );
        void removeIGuideSite( PLuint index );
        void removeGraft     ( PLuint index );

        void toggleArthroView();
            
        void updateGraftMarkerPositions();    
                
        void clear();

    private: 

        std::vector<plDefectSite*> _defectSites;
        std::vector<plDonorSite*>  _donorSites;
        std::vector<plIGuideSite*> _iGuideSites;
        std::vector<plIGuide*>     _iGuides;
        std::vector<plKWire*>      _kWires;
        std::vector<plGraft*>      _grafts;
        std::vector<plModel*>      _models;
        
        PLint _getModelIndex     ( const plMeshSpecific& mesh ) const;
        PLint _getDefectSiteIndex( const plMeshSpecific& mesh ) const;
        
};

#endif

