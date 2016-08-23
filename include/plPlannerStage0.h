#pragma once

#include "plCommon.h"
#include "plUtility.h"

#include "plVector4.h"
#include "plPlannerStructs.h"
#include "plPlannerShader.h"
#include "plPlanningBufferData.h"

#include "plRenderer.h"

#include "plPlan.h"

#define PL_MAX_GRAFTS_PER_SOLUTION           16
#define PL_MAX_CAP_TRIANGLES                 2048
#define PL_NUM_COMPARISION_DIRECTIONS        32     // number of directions used in surface rms comparison

#define PL_STAGE_0_ITERATIONS                 5     // max local iterations
#define PL_STAGE_0_GROUP_SIZE                 64    // number of local invocations per workgroup
#define PL_STAGE_0_NUM_GROUPS                 16    // number of work groups
#define PL_STAGE_0_INVOCATIONS                PL_STAGE_0_NUM_GROUPS*PL_STAGE_0_GROUP_SIZE

#define PL_STAGE_0_INITIAL_TEMPERATURE        1.0f
#define PL_STAGE_0_STOPPING_TEMPERATURE       0.001f
#define PL_STAGE_0_COOLING_RATE               0.015f //0.00175f

class plAnnealingGroup
{
    public:

        plAnnealingGroup(float32_t initialEnergy);

        void bind();
        void unbind();

        void getSolution(
            std::shared_ptr<plDefectSolution> solution,
            std::shared_ptr<plPlanningBufferData> planningData);
        void getLowestGroupInfo(uint32_t &index, float32_t &energy);

    private:

        plSSBO _invoEnergiesSSBO;
        plSSBO _invoGraftPositionsSSBO;
        plSSBO _invoGraftNormalsSSBO;
        plSSBO _invoGraftRadiiSSBO;
        plSSBO _invoGraftCountsSSBO;

        plSSBO _groupEnergiesSSBO;
        plSSBO _groupGraftPositionsSSBO;
        plSSBO _groupGraftNormalsSSBO;
        plSSBO _groupGraftRadiiSSBO;
        plSSBO _groupGraftCountsSSBO;

};

namespace plPlannerStage0
{

    void run(
        std::shared_ptr<plDefectSolution> state,
        std::shared_ptr<plPlanningBufferData> planningData);

}
