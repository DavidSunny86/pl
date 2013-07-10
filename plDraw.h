#ifndef __PL_DRAW_H__
#define __PL_DRAW_H__

#include "pl.h"
#include "plProjection.h"
#include "plCamera.h"
#include "plModel.h"
#include "plPlan.h"
#include "plSeq.h"
#include "plMinimalShader.h"
#include "plPhongShader.h"
#include "plPickingShader.h"
#include "plMesh.h"
#include "plGraftEditor.h"
#include "plBoundaryEditor.h"

void _plDrawPicking();
void _plDrawArrowCone(float scale);

void _setOpenGLState();
        
void _beginPicking();
void _endPicking();

void _beginDrawing();
void _endDrawing();

#endif
