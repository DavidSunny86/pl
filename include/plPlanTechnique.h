#pragma once

#include "plCommon.h"
#include "plRenderTechnique.h"
#include "plRenderResources.h"


class plPlanTechnique : public plRenderTechnique
{
    public:

        plPlanTechnique();

        void render(const std::set<plRenderComponent>& componentSet) const;

    private:

        void _initState() const;

};
