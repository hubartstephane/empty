﻿#include <chaos/GPURenderParams.h>

namespace chaos
{
	// ========================================================
	// GPURenderParams implementation
	// ========================================================

	GPURenderMaterial const * GPURenderParams::GetMaterial(GPURenderable const * renderable, GPURenderMaterial const * default_material) const
	{
		if (material_provider == nullptr)
			return default_material;
		return material_provider->GetMaterial(renderable, default_material, *this);
	}

}; // namespace chaos
