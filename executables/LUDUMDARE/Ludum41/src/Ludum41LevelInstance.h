#pragma once

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>

#include <death/Game.h>
#include <death/GameLevel.h>
#include <death/GameLevelInstance.h>
#include <death/GameFramework.h>

#include "Ludum41Particles.h"

// =================================================
// LevelInstance
// =================================================

class LudumLevelInstance : public death::GameLevelInstance
{

	friend class ParticleMovableObjectTrait;

public:

	DEATH_GAMEFRAMEWORK_DECLARE_LEVELINSTANCE(Ludum);

	/** override */
	virtual bool IsLevelCompleted(bool & loop_levels) const override;
	/** override */
	virtual bool CanCompleteLevel() const override;
	/** override */
	virtual bool Initialize(death::Game * in_game, death::GameLevel * in_level) override;
	/** override */
	virtual bool DoTick(double delta_time) override;
	/** override */
	virtual chaos::box2 GetBoundingBox() const override;

protected:

	chaos::ParticleAllocationBase * CreateBricks();
	
	/** get the bricks */
	ParticleBrick * GetBricks();
	/** get the bricks */
	ParticleBrick const * GetBricks() const;

	/** get the number of bricks */
	size_t GetBrickCount() const;	
	
public:

	/** some sprites */	
	chaos::shared_ptr<chaos::ParticleAllocationBase> bricks_allocations;	
};