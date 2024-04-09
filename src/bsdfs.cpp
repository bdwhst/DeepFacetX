#include "bsdfs.h"

#include "microfacet.h"

AtRGB AsymMicrofacetBSDF::F(vec3 wo, vec3 wi, RandomEngine& rng, int order) const
{
    float z = 0;
    vec3 w = normalize(-wo);
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    int i = 0;
    while (i < order)
    {
        float U = Sample1D(rng);
        float sigmaIn = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA) : util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB);
        float sigmaOut = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB) : util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA);
        float deltaZ = w.z / length(w) * (-log(U) / sigmaIn);
        if (z < mat.zs != z + deltaZ < mat.zs)
        {
            deltaZ = (mat.zs - z) + (deltaZ - (mat.zs - z)) * sigmaIn / sigmaOut;
        }
        z += deltaZ;
        if (z > 0) break;
        vec3 p = z > mat.zs ? mat.fEval(-w, wi, mat.alphaXA, mat.alphaYA, mat.albedo) : mat.fEval(-w, wi, mat.alphaXB, mat.alphaYB, mat.albedo);
        float tau_exit = max(z, mat.zs) * util_GGX_lambda(wi, mat.alphaXA, mat.alphaYA) + min(z - mat.zs, 0.0f) * util_GGX_lambda(wi, mat.alphaXB, mat.alphaYB);
        result += throughput * exp(tau_exit) * p;
        vec3 rand3 = Sample3D(rng);
        if(z > mat.zs)
        {
            w = mat.fSample(-w, rand3, throughput, mat.alphaXA, mat.alphaYA, mat.albedo);
        }
        else
        {
            w = mat.fSample(-w, rand3, throughput, mat.alphaXB, mat.alphaYB, mat.albedo);
        }


        if ((z != z) || (w.z != w.z))
            return AtRGB(0.0f);
        i++;
    }
    return AtRGB(result.x, result.y, result.z);
}

float AsymMicrofacetBSDF::PDF(vec3 wo, vec3 wi) const
{
    vec3 wh = normalize(wo + wi);
    if(isSmall(wh)) return 0.0f;

    return util_Dwi(wi, wh, mat.alphaXA, mat.alphaXB) / (4 * absDot(wo, wh)) + satDot(wi, vec3(0.0, 0.0, 1.0));
}

BSDFSample AsymMicrofacetBSDF::Sample(vec3 wo, RandomEngine& rng, int order) const
{
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    while (i < order)
    {	
        float U = Sample1D(rng);
        float sigmaIn = max(z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA) : util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB),0.0f);
        float sigmaOut = max(z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB) : util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA),0.0f);
        float deltaZ = w.z / length(w) * (-log(U) / sigmaIn);
        if (z < mat.zs != z + deltaZ < mat.zs)
        {
            deltaZ = (mat.zs - z) + (deltaZ - (mat.zs - z)) * sigmaIn / sigmaOut;
        }
        z += deltaZ;
        if (z > 0) break;
        vec3 rand3 = Sample3D(rng);
        vec3 nw;
        if (z > mat.zs)
        {
            w = mat.fSample(-w, rand3, throughput, mat.alphaXA, mat.alphaYA, mat.albedo);
        }
        else
        {
            w = mat.fSample(-w, rand3, throughput, mat.alphaXB, mat.alphaYB, mat.albedo);
        }
        if ((z != z) || (w.z != w.z))
        {
            w = vec3(0.0f, 0.0f, 1.0f);
            break;
        }
        i++;
    }

    return BSDFSample(w, AtRGB(throughput.x, throughput.y, throughput.z), 1.0, AI_RAY_DIFFUSE_REFLECT);
}