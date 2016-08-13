#pragma once

#include "plCommon.h"
#include "plVector3.h"
#include "plMeshSpecific.h"
#include "plBoundary.h"
#include "plRenderable.h"
#include "plPicking.h"
#include "plMeshCutter.h"
#include "plMeshExtruder.h"


class plIGuideSite : public plRenderable
{

    public:

        plBoundary boundary;

        plIGuideSite();
        plIGuideSite(const plMesh& mesh);
        plIGuideSite(const plBoundary& boundary);

        virtual ~plIGuideSite();

        void extractRenderComponents(plRenderMap& renderMap) const;
        void extractRenderComponents(plRenderMap& renderMap, PLuint technique) const;

        const plMesh& templateBase() const { return _templateBase; }

        PLbool generateTemplateBase();

    private:

        plMesh  _templateBase;

       // std::vector<plTriangle>  _templateBase;    // for use in generating iGuides

};
