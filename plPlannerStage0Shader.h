#ifndef __PL_STAGE_0_SHADER_H__
#define __PL_STAGE_0_SHADER_H__

#include "plCommon.h"
#include "plShader.h"
#include "plSiteGrid.h"
#include "plSpline.h"
#include "plMatrix44.h"
#include "plVector4.h"

class plPlannerStage0Shader : public plShader
{

    public:                    
          
        plPlannerStage0Shader ( const char *computeFile );

        void getUniformLocations();
        
        void setSiteUniforms     ( PLuint  meshSize, 
                                   PLfloat meshArea, 
                                   PLuint  gridSize,
                                   PLuint  perimSize,  
                                   const plVector4 &siteNormal ) const;
         
        void setTemperatureUniform( PLfloat temp ) const;        
        void setLocalLoadUniform   ( PLuint load ) const;
               
    private:
    
        PLuint _siteMeshSizeID;                  
        PLuint _siteMeshAreaID;
        PLuint _siteGridSizeID;
        PLuint _sitePerimSizeID;
        PLuint _siteNormalID;
                
        PLuint _temperatureID;    

        PLuint _loadLocalID;

        PLuint _seedID;
                   
};



#endif
