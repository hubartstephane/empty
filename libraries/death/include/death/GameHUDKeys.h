#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/NamedObject.h>

namespace death
{
	// Create a gamespace for unique idenfiers
	namespace GameHUDKeys
	{
		// some allocation ID's
		CHAOS_DECLARE_TAG(LIFE_ID);
		CHAOS_DECLARE_TAG(TITLE_ID);
		CHAOS_DECLARE_TAG(INSTRUCTIONS_ID);
		CHAOS_DECLARE_TAG(SCORE_ID);
		CHAOS_DECLARE_TAG(BEST_SCORE_ID);
		CHAOS_DECLARE_TAG(BACKGROUND_ID);
		// some layer ID's
		CHAOS_DECLARE_TAG(GAME_LAYER_ID);
		CHAOS_DECLARE_TAG(PLAYER_LAYER_ID);
		CHAOS_DECLARE_TAG(HUD_LAYER_ID);
		CHAOS_DECLARE_TAG(TEXT_LAYER_ID);
		CHAOS_DECLARE_TAG(BACKGROUND_LAYER_ID);
	};

}; // namespace death
