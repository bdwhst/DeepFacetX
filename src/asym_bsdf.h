#pragma once
#include "bsdfs.h"
#include "arnoldUtils.h"
#include "randomUtils.h"
#include "mathUtils.h"

AtBSDF* AiAsymBSDF(const AtShaderGlobals* sg, const WithState<AsymMicrofacetBSDF>& asymBSDF);