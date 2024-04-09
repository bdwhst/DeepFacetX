#include "asym_bsdf.h"


static RandomDevice device;

AI_BSDF_EXPORT_METHODS(AsymBSDFMtd);

bsdf_init
{
	auto fs = GetAtBSDFCustomDataPtr<WithState<AsymMicrofacetBSDF>>(bsdf);
	fs->state.SetDirectionsAndRng(sg, false);

	static const AtBSDFLobeInfo lobe_info[] = {
		{ AI_RAY_SPECULAR_REFLECT, 0, AtString() },
		{ AI_RAY_DIFFUSE_REFLECT, 0, AtString() },
	};

	AiBSDFInitLobes(bsdf, lobe_info, 2);
	AiBSDFInitNormal(bsdf, fs->state.nf, true);
}

bsdf_sample
{
	auto fs = GetAtBSDFCustomDataPtr<WithState<AsymMicrofacetBSDF>>(bsdf);
	auto& state = fs->state;
    RandomEngine rng(state.seed);
	BSDFSample sample = fs->bsdf.Sample(state.wo, rng);
    
	if (sample.IsInvalid())
		return AI_BSDF_LOBE_MASK_NONE;

    float pdf = fs->bsdf.PDF(state.wo, sample.wi);
	out_wi = AtVectorDv(toWorld(state.nf, sample.wi));
	out_lobe_index = IsDeltaRay(sample.type) ? 0 : 1;
	out_lobes[out_lobe_index] = AtBSDFLobeSample(sample.f, pdf, pdf);

	return lobe_mask & LobeMask(out_lobe_index);
}

bsdf_eval
{
	auto fs = GetAtBSDFCustomDataPtr<WithState<AsymMicrofacetBSDF>>(bsdf);
	auto& state = fs->state;
    RandomEngine rng(state.seed);
	vec3 wiLocal = toLocal(state.nf, wi);

	AtRGB f = fs->bsdf.F(state.wo, wiLocal, rng);
	float pdf = fs->bsdf.PDF(state.wo, wiLocal);

	if (pdf < 1e-6f || isnan(pdf) || IsInvalid(f) || Luminance(f) > 1e8f)
		return AI_BSDF_LOBE_MASK_NONE;

	int lobe = fs->bsdf.IsDelta() ? 0 : 1;
	out_lobes[lobe] = AtBSDFLobeSample(f, pdf, pdf);
	return lobe_mask & LobeMask(lobe);
}

AtBSDF* AiAsymBSDF(const AtShaderGlobals* sg, const WithState<AsymMicrofacetBSDF>& asymBSDF)
{
	AtBSDF* bsdf = AiBSDF(sg, AI_RGB_WHITE, AsymBSDFMtd, sizeof(WithState<AsymMicrofacetBSDF>));
	GetAtBSDFCustomDataRef<WithState<AsymMicrofacetBSDF>>(bsdf) = asymBSDF;
	return bsdf;
}