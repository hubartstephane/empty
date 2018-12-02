#pragma once

#include <death/Game.h>
#include <death/GameHUD.h>

namespace death
{

	namespace GameHUDKeys
	{
		CHAOS_DECLARE_TAG(WAKENUP_PARTICLE_COUNT_ID);
		CHAOS_DECLARE_TAG(SAVED_PARTICLE_COUNT_ID);
		CHAOS_DECLARE_TAG(LIFE_LAYER_ID);
	};

};

class LudumPlayingHUD : public death::GameHUD
{

public:

	/** constructor */
	LudumPlayingHUD(death::Game * in_game) :
		death::GameHUD(in_game) {}

protected:

	/** override */
	virtual bool DoTick(double delta_time) override;

	void UpdateWakenUpParticleCount(class LudumGame const * ludum_game);

	void UpdateSavedParticleCount(class LudumGame const * ludum_game);

	void UpdateLifeBar(class LudumGame const * ludum_game);
	
protected:

	/** caching particles count  */
	int cached_waken_up_particle_count = -1;
	/** caching saved particles count  */
	int cached_saved_particle_count = -1;
	/** caching the life value */
	float cached_life_value = -1.0f;

};
