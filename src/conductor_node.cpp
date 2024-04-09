#include "bsdfs.h"
#include "microfacet.h"
#include "arnoldUtils.h"
#include "asym_bsdf.h"
#include <ai.h>
AI_SHADER_NODE_EXPORT_METHODS(ConductorNodeMtd);

enum ConductorNodeParams
{
	p_zs = 1,
	p_alpha_x_a, p_alpha_y_a,
    p_alpha_x_b, p_alpha_y_b,
    p_albedo
};


node_parameters
{
	AiParameterStr(NodeParamTypeName, ConductorNodeName);
    AiParameterFlt("boundary_depth", -0.5f);
    AiParameterFlt("alpha_x_a", 0.8f);
    AiParameterFlt("alpha_y_a", 0.8f);
    AiParameterFlt("alpha_x_b", 0.1f);
    AiParameterFlt("alpha_y_b", 0.1f);
	AiParameterRGB("albedo", 0.8f, 0.7f, 0.2f);
}

node_initialize
{
	auto bsdf = new BSDF;
	*bsdf = AsymMicrofacetBSDF();
	AiNodeSetLocalData(node, bsdf);
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
	AsymMicrofacetBSDF asymBSDF;
	asymBSDF.mat.zs = AiShaderEvalParamFlt(p_zs);
	AtRGB albedo = AiShaderEvalParamRGB(p_albedo);
    asymBSDF.mat.albedo = vec3(albedo.r, albedo.g, albedo.b);
    asymBSDF.mat.alphaXA = AiShaderEvalParamFlt(p_alpha_x_a);
    asymBSDF.mat.alphaYA = AiShaderEvalParamFlt(p_alpha_y_a);
	asymBSDF.mat.alphaXB = AiShaderEvalParamFlt(p_alpha_x_b);
    asymBSDF.mat.alphaYB = AiShaderEvalParamFlt(p_alpha_y_b);
	
    asymBSDF.mat.fEval = util_conductor_evalPhaseFunction;
    asymBSDF.mat.fSample = util_conductor_samplePhaseFunction;

	GetNodeLocalDataRef<BSDF>(node) = asymBSDF;

	if (sg->Rt & AI_RAY_SHADOW)
		return;
	sg->out.CLOSURE() = AiAsymBSDF(sg, { asymBSDF, BSDFState() });
}

