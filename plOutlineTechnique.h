#ifndef PL_OUTLINE_TECHNIQUE_H
#define PL_OUTLINE_TECHNIQUE_H

#include "plCommon.h"
#include "plRenderTechnique.h"
#include "plRenderResources.h"


class plOutlineTechnique : public plRenderTechnique
{

    public:

        plOutlineTechnique();       

        void render( const std::set< plRenderComponent >& componentSet ) const;

    private:

        void _initState() const;
        plVAO _generateQuad() const;
        plRenderComponent _generateComponent() const;
};


#endif 
