#pragma once

#include <chaos/Chaos.h>

#include "Ludum48Game.h"

// =================================================
// Levels
// =================================================

class LudumLevel : public chaos::TMLevel
{
	friend class LudumLevelInstance;

	CHAOS_DECLARE_OBJECT_CLASS2(LudumLevel, chaos::TMLevel);

	LudumLevel();

protected:

	virtual chaos::ParticleLayerBase * DoCreateParticleLayer(chaos::TMLayerInstance * layer_instance) override;

	virtual chaos::TMObjectFactory DoGetObjectFactory(chaos::TMLayerInstance * in_layer_instance, chaos::TiledMap::TypedObject const * in_typed_object) override;

	virtual bool Initialize(chaos::TiledMap::Map* in_tiled_map) override;

	virtual bool FinalizeLayerParticles(chaos::TMLayerInstance* layer_instance, chaos::ParticleAllocationBase* allocation) override;
};
