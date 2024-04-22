#pragma once
#include "bsdfs.h"
#include "arnoldUtils.h"
#include "randomUtils.h"
#include "mathUtils.h"

AtBSDF* AiAsymConductorBSDF(const AtShaderGlobals* sg, const WithState<AsymConductorBSDF>& asymBSDF);
AtBSDF* AiAsymDieletricBSDF(const AtShaderGlobals* sg, const WithState<AsymDielectricBSDF>& asymBSDF);