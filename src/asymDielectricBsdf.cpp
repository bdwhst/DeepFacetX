#include "aiAsymBsdfs.h"


static RandomDevice device;

AI_BSDF_EXPORT_METHODS(AsymDielectricBSDFMtd);

bsdf_init
{
	auto fs = GetAtBSDFCustomDataPtr<WithState<AsymDielectricBSDF>>(bsdf);
	fs->state.SetDirectionsAndRng(sg, true);

	static const AtBSDFLobeInfo lobe_info[] = {
		{ AI_RAY_SPECULAR_REFLECT, 0, AtString() },
		{ AI_RAY_DIFFUSE_REFLECT, 0 , AtString() },
		{ AI_RAY_SPECULAR_TRANSMIT, 0, AtString() },
		{ AI_RAY_DIFFUSE_TRANSMIT, 0 , AtString() },
	};

	AiBSDFInitLobes(bsdf, lobe_info, 4);
	AiBSDFInitNormal(bsdf, fs->state.nf, false);
}

bsdf_sample
{
	auto fs = GetAtBSDFCustomDataPtr<WithState<AsymDielectricBSDF>>(bsdf);
	auto& state = fs->state;
    RandomEngine rng(state.seed ^ floatBitsToInt(rnd.x));
	BSDFSample sample = fs->bsdf.Sample(state.wo, rng);
    
	if (sample.IsInvalid())
		return AI_BSDF_LOBE_MASK_NONE;
	int isTransmission = sameHemisphere(state.wo, sample.wi) ? 0 : 2;
    float pdf = fs->bsdf.PDF(state.wo, sample.wi);
	out_wi = AtVectorDv(toWorld(state.nf, sample.wi));
	out_lobe_index = (IsDeltaRay(sample.type) ? 0 : 1) | (isTransmission ? 2 : 0);
	//out_lobe_index = (0) | (isTransmission ? 2 : 0);
	out_lobes[out_lobe_index] = AtBSDFLobeSample(sample.f, 1.0f, pdf);

	return lobe_mask & LobeMask(out_lobe_index);
}

bsdf_eval
{
	auto fs = GetAtBSDFCustomDataPtr<WithState<AsymDielectricBSDF>>(bsdf);
	auto& state = fs->state;
	RandomEngine rng(state.seed ^ floatBitsToInt(wi.x));
	vec3 wiLocal = toLocal(state.nf, wi);

	AtRGB f = fs->bsdf.F(state.wo, wiLocal, rng);
	float pdf = fs->bsdf.PDF(state.wo, wiLocal);

	if (pdf < 1e-6f || isnan(pdf) || IsInvalid(f) || Luminance(f) > 1e8f)
		return AI_BSDF_LOBE_MASK_NONE;
	int isTransmission = sameHemisphere(state.wo, wiLocal) ? 0 : 2;
	int lobe = (fs->bsdf.IsDelta() ? 0 : 1) | (isTransmission ? 2 : 0);
	//int lobe = (0) | (isTransmission ? 2 : 0);
	out_lobes[lobe] = AtBSDFLobeSample(f, pdf, pdf);
	return lobe_mask & LobeMask(lobe);
}

AtBSDF* AiAsymDieletricBSDF(const AtShaderGlobals* sg, const WithState<AsymDielectricBSDF>& asymBSDF)
{
	AtBSDF* bsdf = AiBSDF(sg, AI_RGB_WHITE, AsymDielectricBSDFMtd, sizeof(WithState<AsymDielectricBSDF>));
	GetAtBSDFCustomDataRef<WithState<AsymDielectricBSDF>>(bsdf) = asymBSDF;
	return bsdf;
}