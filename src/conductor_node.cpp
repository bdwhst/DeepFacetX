#include "bsdfs.h"
#include "microfacet.h"
#include "arnoldUtils.h"
#include "aiAsymBsdfs.h"
#include <ai.h>
AI_SHADER_NODE_EXPORT_METHODS(ConductorNodeMtd);

enum ConductorNodeParams
{
	p_zs = 1,
	p_alpha_x_a, p_alpha_y_a,
    p_alpha_x_b, p_alpha_y_b,
    p_albedoA, p_albedoB,
	p_bdeval
};


node_parameters
{
	AiParameterStr(NodeParamTypeName, ConductorNodeName);
    AiParameterFlt("boundary_depth", -0.5f);
    AiParameterFlt("alpha_x_a", 0.8f);
    AiParameterFlt("alpha_y_a", 0.8f);
    AiParameterFlt("alpha_x_b", 0.1f);
    AiParameterFlt("alpha_y_b", 0.1f);
	AiParameterRGB("albedoA", 0.8f, 0.7f, 0.2f);
	AiParameterRGB("albedoB", 0.8f, 0.7f, 0.2f);
	AiParameterBool("bd_eval", true);
}

node_initialize
{
	auto bsdf = new BSDF;
	*bsdf = AsymConductorBSDF();
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
	AsymConductorBSDF asymBSDF;
	asymBSDF.mat.zs = AiShaderEvalParamFlt(p_zs);
	AtRGB albedo = AiShaderEvalParamRGB(p_albedoA);
    asymBSDF.mat.albedoA = vec3(albedo.r, albedo.g, albedo.b);
	albedo = AiShaderEvalParamRGB(p_albedoB);
	asymBSDF.mat.albedoB = vec3(albedo.r, albedo.g, albedo.b);
    asymBSDF.mat.alphaXA = AiShaderEvalParamFlt(p_alpha_x_a);
    asymBSDF.mat.alphaYA = AiShaderEvalParamFlt(p_alpha_y_a);
	asymBSDF.mat.alphaXB = AiShaderEvalParamFlt(p_alpha_x_b);
    asymBSDF.mat.alphaYB = AiShaderEvalParamFlt(p_alpha_y_b);
	asymBSDF.BDEval = AiShaderEvalParamBool(p_bdeval);

	GetNodeLocalDataRef<BSDF>(node) = asymBSDF;

	if (sg->Rt & AI_RAY_SHADOW)
		return;
	sg->out.CLOSURE() = AiAsymConductorBSDF(sg, { asymBSDF, BSDFState() });
}

