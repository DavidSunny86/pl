#pragma once

#include "plCommon.h"
#include "plTimer.h"
#include "plPlan.h"
#include "plPlanningBufferData.h"
#include "plPlannerStage0.h"
#include "plPlannerStage1.h"
#include "plPlannerStage2.h"
#include "plPlannerStage3.h"

namespace plAutomaticPlanner
{
    void calculate(plPlan &plan, uint32_t defectSiteIndex);
}
