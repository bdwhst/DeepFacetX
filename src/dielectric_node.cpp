#include "bsdfs.h"
#include "microfacet.h"
#include "arnoldUtils.h"
#include "aiAsymBsdfs.h"
#include <ai.h>
AI_SHADER_NODE_EXPORT_METHODS(DielectricNodeMtd);

enum DielectricNodeParams
{
	p_zs = 1,
	p_ior,
	p_alpha_x_a, p_alpha_y_a,
    p_alpha_x_b, p_alpha_y_b,
    p_albedoA,
	p_bdeval
};


node_parameters
{
	AiParameterStr(NodeParamTypeName, ConductorNodeName);
    AiParameterFlt("boundary_depth", -0.5f);
	AiParameterFlt("ior", 2.2f);
    AiParameterFlt("alpha_x_a", 0.8f);
    AiParameterFlt("alpha_y_a", 0.8f);
    AiParameterFlt("alpha_x_b", 0.1f);
    AiParameterFlt("alpha_y_b", 0.1f);
	AiParameterRGB("albedo", 1.0f, 1.0f, 1.0f);
	AiParameterBool("bd_eval", true);
}

node_initialize
{
	auto bsdf = new BSDF;
	*bsdf = AsymDielectricBSDF();
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
	AsymDielectricBSDF asymBSDF;
	asymBSDF.mat.zs = AiShaderEvalParamFlt(p_zs);
	asymBSDF.mat.ior = AiShaderEvalParamFlt(p_ior);
	AtRGB albedo = AiShaderEvalParamRGB(p_albedoA);
    asymBSDF.mat.albedoA = vec3(albedo.r, albedo.g, albedo.b);
    asymBSDF.mat.alphaXA = AiShaderEvalParamFlt(p_alpha_x_a);
    asymBSDF.mat.alphaYA = AiShaderEvalParamFlt(p_alpha_y_a);
	asymBSDF.mat.alphaXB = AiShaderEvalParamFlt(p_alpha_x_b);
    asymBSDF.mat.alphaYB = AiShaderEvalParamFlt(p_alpha_y_b);
	asymBSDF.BDEval = AiShaderEvalParamBool(p_bdeval);
	
	GetNodeLocalDataRef<BSDF>(node) = asymBSDF;

	if (sg->Rt & AI_RAY_SHADOW)
		return;
	sg->out.CLOSURE() = AiAsymDieletricBSDF(sg, { asymBSDF, BSDFState() });
}

