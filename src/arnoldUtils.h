#pragma once

#include <ai.h>
#include <ai_shader_bsdf.h>
#include <ai_shaderglobals.h>

const char NodeParamTypeName[] = "type_name";
const char ConductorNodeName[] = "AsymMicrofacetConductorNode";

template<typename T>
T* GetNodeLocalDataPtr(const AtNode* node)
{
	return reinterpret_cast<T*>(AiNodeGetLocalData(node));
}

template<typename T>
T& GetNodeLocalDataRef(const AtNode* node)
{
	return *reinterpret_cast<T*>(AiNodeGetLocalData(node));
}

template<typename T>
T* GetAtBSDFCustomDataPtr(const AtBSDF* bsdf)
{
	return reinterpret_cast<T*>(AiBSDFGetData(bsdf));
}

template<typename T>
T& GetAtBSDFCustomDataRef(const AtBSDF* bsdf)
{
	return *reinterpret_cast<T*>(AiBSDFGetData(bsdf));
}


inline AtBSDFLobeMask LobeMask(int idx) {
	return 1 << idx;
}