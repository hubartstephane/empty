#pragma once

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>
#include <chaos/TiledMap.h>
#include <chaos/TiledMapTools.h>

#include <death/Level.h>
#include <death/TiledMapLevel.h>
#include <death/Game.h>
#include <death/Player.h>
#include <death/GameFramework.h>

#include "Ludum44Game.h"

// =================================================
// LudumLevelInstance
// =================================================

class LudumLevelInstance : public death::TiledMapLevelInstance
{
public:

	DEATH_GAMEFRAMEWORK_DECLARE_FRIENDSHIPS(Ludum);

	/** constructor */
	LudumLevelInstance(class LudumGame * in_game);

	/** Set current scroll factor */
	void SetScrollFactor(float in_scroll_factor){ scroll_factor = in_scroll_factor;}
	/** Get current scroll factor */
	float GetScrollFactor() const { return scroll_factor; }

protected:

	/** override */
	virtual bool DoTick(float delta_time) override;
	/** override */
	virtual void OnLevelStarted() override;
	/** override */
	virtual bool Initialize(death::Game * in_game, death::Level * in_level) override;
	/** override */
	virtual void CreateGameCameras() override;

	/** override */
	virtual death::LevelCheckpoint * DoCreateCheckpoint() const;
	/** override */
	virtual bool DoLoadFromCheckpoint(death::LevelCheckpoint const * checkpoint) override;
	/** override */
	virtual bool DoSaveIntoCheckpoint(death::LevelCheckpoint * checkpoint) const override;


protected:

	/** pointer on game */
	class LudumGame * game = nullptr;

	/** the horizontal camera speed */
	float camera_speed = 100.0f;
	/** an additional factor that can be applyed on demand */
	float scroll_factor = 1.0f;
};
