#include "bsdfs.h"

#include "microfacet.h"

AtRGB AsymConductorBSDF::F(vec3 wo, vec3 wi, RandomEngine& rng, int order) const
{
    float z = 0;
    vec3 w = normalize(-wo);
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    int i = 0;
    bool outside = true;
    while (i < order)
    {
        float U = Sample1D(rng);
        float sigmaIn = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA) : util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB);
        float sigmaOut = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB) : util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA);
        if (sigmaIn == 0.0f)
        {
            z = 0.0;
            break;
        }
        float deltaZ = w.z / length(w) * (-log(U) / sigmaIn);
        if (z < mat.zs != z + deltaZ < mat.zs)
        {
            deltaZ = mat.zs - z + (deltaZ - (mat.zs - z) * sigmaIn / sigmaOut);
        }
        z += deltaZ;
        if (z > 0) break;

        vec3 p = z > mat.zs ? util_conductor_evalPhaseFunction(-w, wi, mat.alphaXA, mat.alphaYA, mat.albedo) : util_conductor_evalPhaseFunction(-w, wi, mat.alphaXB, mat.alphaYB, mat.albedo);
        float tau_exit = max(z, mat.zs) * util_GGX_lambda(wi, mat.alphaXA, mat.alphaYA) + min(z - mat.zs, 0.0f) * util_GGX_lambda(wi, mat.alphaXB, mat.alphaYB);
        result += throughput * exp(tau_exit) * p;

        vec3 rand3 = Sample3D(rng);
        if(z > mat.zs)
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXA, mat.alphaYA, mat.albedo, (-w).z > 0, outside);
        }
        else
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXB, mat.alphaYB, mat.albedo, (-w).z > 0, outside);
        }


        if ((z != z) || (w.z != w.z))
            return AtRGB(0.0f);
        i++;
    }
    return AtRGB(result.x, result.y, result.z);
}

float AsymConductorBSDF::PDF(vec3 wo, vec3 wi) const
{
    vec3 wh = normalize(wo + wi);
    if(isSmall(wh)) return 0.0f;

    return util_Dwi(wo, wh, mat.alphaXA, mat.alphaXB) / (4 * absDot(wo, wh)) + max(wi.z, 0.0f);
}

BSDFSample AsymConductorBSDF::Sample(vec3 wo, RandomEngine& rng, int order) const
{
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    bool outside = true;
    while (i < order)
    {	
        float U = Sample1D(rng);
        float sigmaIn = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA) : util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB);
        float sigmaOut = z > mat.zs ? util_GGX_extinction_coeff(w, mat.alphaXB, mat.alphaYB) : util_GGX_extinction_coeff(w, mat.alphaXA, mat.alphaYA);
        if (sigmaIn == 0.0f)
        {
            z = 0.0;
            break;
        }
        float deltaZ = w.z / length(w) * (-log(U) / sigmaIn);
        if (z < mat.zs != z + deltaZ < mat.zs)
        {
            deltaZ = (mat.zs - z) + (deltaZ - (mat.zs - z)) * sigmaIn / sigmaOut;
        }
        z += deltaZ;
        if (z > 0) break;
        vec3 rand3 = Sample3D(rng);
        if (z > mat.zs)
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXA, mat.alphaYA, mat.albedo, (-w).z > 0, outside);
        }
        else
        {
            w = util_conductor_samplePhaseFunction(-w, rand3, throughput, mat.alphaXB, mat.alphaYB, mat.albedo, (-w).z > 0, outside);
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

AtRGB AsymDielectricBSDF::F(vec3 wo, vec3 wi, RandomEngine& rng, int order) const
{
    const float eta = mat.ior;
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    bool outside = wo.z > 0;
    float zs = outside ? mat.zs : util_flip_z(mat.zs);
    bool flipped = !outside;
    if (!outside)
    {
        w = -w;
    }
    while (i < order)
    {
        float U = Sample1D(rng);
        float alphaXA = outside ? mat.alphaXA : mat.alphaXB;
        float alphaYA = outside ? mat.alphaYA : mat.alphaYB;
        float alphaXB = outside ? mat.alphaXB : mat.alphaXA;
        float alphaYB = outside ? mat.alphaYB : mat.alphaYA;
        float sigmaIn, sigmaOut, deltaZ;

        sigmaIn = z > zs ? util_GGX_extinction_coeff(w, alphaXA, alphaYA) : util_GGX_extinction_coeff(w, alphaXB, alphaYB);
        sigmaOut = z > zs ? util_GGX_extinction_coeff(w, alphaXB, alphaYB) : util_GGX_extinction_coeff(w, alphaXA, alphaYA);
        if (sigmaIn == 0.0f)
        {
            z = 0.0;
            break;
        }
        deltaZ = w.z * (-log(U) / sigmaIn);

        z += deltaZ;
        if (z > 0) break;
        //use unflipped w's negative as input to eval
        vec3 wo_unflipped = flipped ? w : -w;
        vec3 p = z > zs ? util_dielectric_evalPhaseFunction(wo_unflipped, wi, alphaXA, alphaYA, mat.albedo, outside, wi.z > 0, eta) : util_dielectric_evalPhaseFunction(wo_unflipped, wi, alphaXB, alphaYB, mat.albedo, outside, wi.z > 0, eta);
        float tau_exit, lambdaA, lambdaB, z_t = z, zs_t = zs;
        if (wi.z<0)
        {
            lambdaA = util_GGX_lambda(-wi, alphaXA, alphaYA);
            lambdaB = util_GGX_lambda(-wi, alphaXB, alphaYB);
            if (!flipped)
            {
                z_t = util_flip_z(z_t);
                zs_t = util_flip_z(zs_t);
            }      
        }
        else
        {
            lambdaA = util_GGX_lambda(wi, alphaXA, alphaYA);
            lambdaB = util_GGX_lambda(wi, alphaXB, alphaYB);
            if (flipped)
            {
                z_t = util_flip_z(z_t);
                zs_t = util_flip_z(zs_t);
            }
        }
        tau_exit = max(z_t, zs_t) * lambdaA + min(z_t - zs_t, 0.0f) * lambdaB;
        result += throughput * exp(tau_exit) * p;

        vec3 rand3 = Sample3D(rng);
        bool n_outside = outside;
        if (z > zs)
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXA, alphaYA, mat.albedo, outside, n_outside, eta);
        }
        else
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXB, alphaYB, mat.albedo, outside, n_outside, eta);
        }
        if ((z != z) || (w.z != w.z))
            return AtRGB(0.0, 0.0, 0.0);
        if (n_outside != outside)
        {
            z = util_flip_z(z);
            zs = util_flip_z(zs);
            w = -w;
            outside = !outside;
            flipped = !flipped;
        }
        i++;
    }
    if (z < 0) return AtRGB(0, 0, 0);
    return AtRGB(result.x, result.y, result.z);
}

float AsymDielectricBSDF::MIS_weight(const vec3& wi, const vec3& wo, bool wi_inside, float eta, float alpha_x, float alpha_y) const
{
    return Single_Scatter_PDF(wi, wo, alpha_x, alpha_y);
    eta = wi_inside ? eta : 1.0 / eta;
    if (wi.z * wo.z > 0)
    {
        vec3 wh = normalize(wi + wo);
        return util_D(wh.z > 0 ? wh : -wh, alpha_x, alpha_y);
    }
    else
    {
        const vec3 wh = normalize(wi + wo * eta);
        return util_D(wh.z > 0 ? wh : -wh, alpha_x, alpha_y);
    }
    //return Single_Scatter_PDF(wi, wo, alpha_x, alpha_y);
}


//Start random walk from wo, 
AtRGB AsymDielectricBSDF::F_eval_from_wo(vec3 wo, vec3 wi, RandomEngine& rng, int order) const
{
    const float eta = mat.ior;
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 multiple_scatter = vec3(0.0f, 0.0f, 0.0f);
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    bool outside = wo.z > 0;
    float zs = outside ? mat.zs : util_flip_z(mat.zs);
    bool flipped = !outside;
    if (!outside)
    {
        w = -w;
    }
    vec3 single_scattering = vec3(0, 0, 0);
    float w_a = 1 - exp(mat.zs);
    if (wo.z * wi.z > 0)//reflect
    {
        if (wo.z > 0)
            single_scattering = util_asym_dielectric_single_scattering_F_reflect(wo, wi, mat.alphaXA, mat.alphaYA, mat.alphaXB, mat.alphaYB, w_a, eta);
        else
            single_scattering = util_asym_dielectric_single_scattering_F_reflect(-wo, -wi, mat.alphaXA, mat.alphaYA, mat.alphaXB, mat.alphaYB, w_a, eta);
    }
    else//refract
    {
        if (wo.z > 0)
            single_scattering = util_asym_dielectric_single_scattering_F_refract(wo, wi, mat.alphaXA, mat.alphaYA, mat.alphaXB, mat.alphaYB, w_a, eta);
        else
            single_scattering = util_asym_dielectric_single_scattering_F_refract(-wo, -wi, mat.alphaXA, mat.alphaYA, mat.alphaXB, mat.alphaYB, w_a, eta);
    }
    if (isInvalid(single_scattering))
        return AtRGB(0.0, 0.0, 0.0);
    float wo_misW;
    while (i < order)
    {
        float U = Sample1D(rng);
        float alphaXA = outside ? mat.alphaXA : mat.alphaXB;
        float alphaYA = outside ? mat.alphaYA : mat.alphaYB;
        float alphaXB = outside ? mat.alphaXB : mat.alphaXA;
        float alphaYB = outside ? mat.alphaYB : mat.alphaYA;
        float sigmaIn, sigmaOut, deltaZ;

        sigmaIn = z > zs ? util_GGX_extinction_coeff(w, alphaXA, alphaYA) : util_GGX_extinction_coeff(w, alphaXB, alphaYB);
        sigmaOut = z > zs ? util_GGX_extinction_coeff(w, alphaXB, alphaYB) : util_GGX_extinction_coeff(w, alphaXA, alphaYA);
        if (sigmaIn == 0.0f)
        {
            z = 0.0;
            break;
        }
        deltaZ = w.z * (-log(U) / sigmaIn);

        z += deltaZ;
        if (z > 0) break;
        if (i > 1)
        {
            //use unflipped w's negative as input to eval
            vec3 wo_unflipped = flipped ? w : -w;
            vec3 p = z > zs ? util_dielectric_evalPhaseFunction(wo_unflipped, wi, alphaXA, alphaYA, mat.albedo, outside, wi.z > 0, eta) : util_dielectric_evalPhaseFunction(wo_unflipped, wi, alphaXB, alphaYB, mat.albedo, outside, wi.z > 0, eta);
            float tau_exit, lambdaA, lambdaB, z_t = z, zs_t = zs;
            if (wi.z < 0)
            {
                lambdaA = util_GGX_lambda(-wi, alphaXA, alphaYA);
                lambdaB = util_GGX_lambda(-wi, alphaXB, alphaYB);
                if (!flipped)
                {
                    z_t = util_flip_z(z_t);
                    zs_t = util_flip_z(zs_t);
                }
            }
            else
            {
                lambdaA = util_GGX_lambda(wi, alphaXA, alphaYA);
                lambdaB = util_GGX_lambda(wi, alphaXB, alphaYB);
                if (flipped)
                {
                    z_t = util_flip_z(z_t);
                    zs_t = util_flip_z(zs_t);
                }
            }
            float alphaX = z > zs ? alphaXA : alphaXB;
            float alphaY = z > zs ? alphaYA : alphaYB;
            float wi_misW = MIS_weight(wo_unflipped, wi, outside, eta, alphaX, alphaY);
            tau_exit = max(z_t, zs_t) * lambdaA + min(z_t - zs_t, 0.0f) * lambdaB;
            float denom = wi_misW + wo_misW;
            multiple_scatter += exp(tau_exit) * p * wo_misW / denom;
        }

        vec3 rand3 = Sample3D(rng);
        bool n_outside = outside;
        vec3 nw;
        if (z > zs)
        {
            nw = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXA, alphaYA, mat.albedo, outside, n_outside, eta);
        }
        else
        {
            nw = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXB, alphaYB, mat.albedo, outside, n_outside, eta);
        }
        if ((z != z) || (nw.z != nw.z) || isInvalid(multiple_scatter))
            return AtRGB(0.0, 0.0, 0.0);

        if (i == 0)
        {
            wo_misW = MIS_weight(wo, nw, outside, eta, alphaXA, alphaYA);
        }

        w = nw;
        if (n_outside != outside)
        {
            z = util_flip_z(z);
            zs = util_flip_z(zs);
            w = -nw;
            outside = !outside;
            flipped = !flipped;
        }
        i++;
    }
    if (z < 0) return AtRGB(0, 0, 0);
    vec3 result = 0.5 * single_scattering + multiple_scatter;
    return AtRGB(result.x, result.y, result.z);
}

AtRGB AsymDielectricBSDF::F_bidirectional(vec3 wo, vec3 wi, RandomEngine& rng, int order) const
{
    float u = Sample1D(rng);
    float factor = 1.0f;
    if (wo.z * wi.z < 0)
    {
        factor = wo.z > 0 ? 1.0 / mat.ior : mat.ior;
    }
    if (u > 0.5f)
    {
        return 2.0f * F_eval_from_wo(wo, wi, rng, order) * (factor * factor);
    }
    else
    {
        return 2.0f * F_eval_from_wo(wi, wo, rng, order) * abs(wi.z) / abs(wo.z);
    }
}

float AsymDielectricBSDF::Single_Scatter_PDF(vec3 wo, vec3 wi, float alphaX, float alphaY) const
{
    float cos_wo = wo.z, cos_wi = wi.z;
    bool reflect = cos_wo * cos_wi > 0;
    float etap = 1;
    if (!reflect)
        etap = cos_wo > 0 ? mat.ior : (1 / mat.ior);
    vec3 wm = wi * etap + wo;
    if (abs(cos_wo) < 1e-6 || abs(cos_wi) < 1e-6 || isSmall(wm)) return 0.0;
    //if (dot(wm, wi) * cos_wi < 0 || dot(wm, wo) * cos_wo < 0) return 0.0;
    float R = util_fresnel(wo, faceForward(normalize(wm), wo), etap);
    float T = 1 - R;
    float pdf = 0.0;

    if (reflect)
    {
        float dwm_dwi = 1 / (4 * absDot(wo, wm));
        wo = faceForward(wo, vec3(0, 0, 1));
        wm = faceForward(wm, vec3(0, 0, 1));
        pdf = util_Dwi(wo, wm, alphaX, alphaY) * dwm_dwi * R;
    }
    else
    {
        float denom = dot(wi, wm) + dot(wo, wm) / etap;
        float dwm_dwi = absDot(wi, wm) / (denom * denom);
        wo = faceForward(wo, vec3(0, 0, 1));
        wm = faceForward(wm, vec3(0, 0, 1));
        pdf = util_Dwi(wo, wm, alphaX, alphaY) * dwm_dwi * T;
    }
    return pdf;
}

float AsymDielectricBSDF::PDF(vec3 wo, vec3 wi) const
{
    float pdf = Single_Scatter_PDF(wo, wi, mat.alphaXA, mat.alphaYA);
    //single scatter plus diffuse
    return abs(pdf) + abs(wi.z);
}

BSDFSample AsymDielectricBSDF::Sample(vec3 wo, RandomEngine& rng, int order) const
{
    const float eta = mat.ior;
    float z = 0;
    vec3 w = normalize(-wo);
    int i = 0;
    vec3 throughput = vec3(1.0f, 1.0f, 1.0f);
    bool outside = wo.z > 0;
    float zs = outside ? mat.zs : util_flip_z(mat.zs);
    bool flipped = false;
    if (!outside)
    {
        w = -w;
        flipped = true;
    }

    while (i < order)
    {
        float U = Sample1D(rng);
        float alphaXA = outside ? mat.alphaXA : mat.alphaXB;
        float alphaYA = outside ? mat.alphaYA : mat.alphaYB;
        float alphaXB = outside ? mat.alphaXB : mat.alphaXA;
        float alphaYB = outside ? mat.alphaYB : mat.alphaYA;
        float sigmaIn, sigmaOut, deltaZ;

        sigmaIn = z > zs ? util_GGX_extinction_coeff(w, alphaXA, alphaYA) : util_GGX_extinction_coeff(w, alphaXB, alphaYB);
        sigmaOut = z > zs ? util_GGX_extinction_coeff(w, alphaXB, alphaYB) : util_GGX_extinction_coeff(w, alphaXA, alphaYA);
        if (sigmaIn == 0.0f)
        {
            z = 0.0;
            break;
        }
        deltaZ = w.z * (-log(U) / sigmaIn);

        if (z < zs != z + deltaZ < zs)
        {
            deltaZ = (zs - z) + (deltaZ - (zs - z)) * sigmaIn / sigmaOut;
        }
        z += deltaZ;
        if (z > 0) break;

        vec3 rand3 = Sample3D(rng);
        bool n_outside = outside;
        if (z > zs)
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXA, alphaYA, mat.albedo, outside, n_outside, eta);
        }
        else
        {
            w = util_dielectric_samplePhaseFunction(-w, rand3, throughput, alphaXB, alphaYB, mat.albedo, outside, n_outside, eta);
        }
        if ((z != z) || (w.z != w.z))
            return BSDFSample(vec3(0, 0, 1), AtRGB(0.0, 0.0, 0.0), 1.0, AI_RAY_DIFFUSE_REFLECT);
        if (n_outside != outside)
        {
            z = util_flip_z(z);
            zs = util_flip_z(zs);
            w = -w;
            outside = !outside;
            flipped = !flipped;
        }
        i++;
    }
    if (z < 0) BSDFSample(vec3(0, 0, 1), AtRGB(0, 0, 0), 1.0, AI_RAY_DIFFUSE_REFLECT);
    w = flipped ? -w : w;
    return BSDFSample(w, AtRGB(throughput.x, throughput.y, throughput.z), 1.0, AI_RAY_DIFFUSE_REFLECT);
}