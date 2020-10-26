#pragma once

#include <chaos/Chaos.h>

#include <death/Level.h>
#include <death/TM.h>
#include <death/Game.h>
#include <death/Player.h>
#include <death/GameFramework.h>

#include "Ludum45Game.h"

// =================================================
// LudumLevelInstance
// =================================================

class LudumLevelInstance : public death::TMLevelInstance
{
public:

	DEATH_GAMEFRAMEWORK_DECLARE_FRIENDSHIPS(Ludum);

	CHAOS_OBJECT_DECLARE_CLASS2(LudumLevelInstance, death::TMLevelInstance);

protected:

	/** override */
	virtual bool DoTick(float delta_time) override;
	/** override */
	virtual bool Initialize(death::Game * in_game, death::Level * in_level) override;
	/** override */
	virtual void OnPlayerLeaved(death::Player * player) override;
	/** override */
	virtual void CreateCameras() override;

protected:

	/** pointer on game */
	class LudumGame * game = nullptr;


	float camera_speed = 100.0f;

};
