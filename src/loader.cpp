#include <cstring>
#include <ai.h>

#include "arnoldUtils.h"

#define DECL_METHOD(method, number) \
    extern const AtNodeMethods* method; \
    namespace NodeMethod { const int method = number; }

#define DECL_CASE(method, nname) \
	case NodeMethod::method: { \
		node->methods = method; \
		node->output_type = AI_TYPE_CLOSURE; \
		node->name = nname; \
		node->node_type = AI_NODE_SHADER; \
		break; \
	}

DECL_METHOD(ConductorNodeMtd, 0);

//node_loader
node_loader
{
	switch (i)
	{
	DECL_CASE(ConductorNodeMtd, ConductorNodeName);

	default:
		return false;
	}

	strcpy(node->version, AI_VERSION);
	return true;
}