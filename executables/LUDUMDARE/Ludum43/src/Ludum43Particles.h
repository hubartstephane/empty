#pragma once

#include <chaos/StandardHeaders.h> 
#include <chaos/ParticleTools.h> 
#include <chaos/ParticleManager.h> 
#include <chaos/ParticleDefault.h> 
#include <chaos/GPUVertexDeclaration.h> 


#include <death/TiledMapParticle.h> 


static float const DANGER_RADIUS_RATIO = 0.3f;
static float const WAKEN_RADIUS_RATIO  = 1.0f;

// ===========================================================================
// VertexBase and Particle
// ===========================================================================

class VertexBase : public chaos::ParticleDefault::Vertex
{
public:

	glm::vec2 bounding_box_position;
	glm::vec2 bounding_box_half_size;
};

chaos::GPUVertexDeclaration GetTypedVertexDeclaration(boost::mpl::identity<VertexBase>);

// shuxxx : 
//   should be that
//using ParticleBase = chaos::ParticleDefault::Particle;
//   but, due to required refactor of TiledMapSystem, replace with that

class ParticleBase : public death::TiledMap::TileParticle
{
public:

	glm::vec2 velocity = glm::vec2(0.0f, 0.0f);


};




// ===========================================================================
// ParticleAffector
// ===========================================================================

class ParticleAffector : public ParticleBase
{
public:

	float attraction_minradius = 0.0f;
	float attraction_maxradius = 0.0f;
	float attraction_force     = 0.0f;
	float repulsion_force      = 0.0f;
	float tangent_force        = 0.0f;

	bool reversed = false;
	bool world_limits = false;
};


// ===========================================================================
// ParticleEnemy
// ===========================================================================

class ParticleEnemy : public ParticleAffector
{
public:

	glm::vec2 rotation_center;

	float rotation_radius = 0.0f;
	float rotation_alpha  = 0.0f;


	

};



class ParticleEnemyTrait : public chaos::ParticleLayerTrait<ParticleEnemy, VertexBase>
{
public:

	class UpdateEnemyData
	{
	public:

	
	};

	bool UpdateParticle(float delta_time, ParticleEnemy * particle, chaos::ParticleAllocation * allocation, UpdateEnemyData const & update_data) const;

	size_t ParticleToVertices(ParticleEnemy const * particle, VertexBase * vertices, size_t vertices_per_particle, chaos::ParticleAllocation * allocation) const;

	UpdateEnemyData BeginUpdateParticles(float delta_time, ParticleEnemy * particles, size_t count, chaos::ParticleAllocation * allocation) const;

public:

	class LudumGame * game = nullptr;
};

// ===========================================================================
// ParticlePlayer
// ===========================================================================

class ParticlePlayer : public ParticleAffector
{
public:

	glm::vec2 acceleration = glm::vec2(0.0f, 0.0f);
	float particle_radius_factor = 2.0f;          // very usefull to prevent floating atoms between player and a field
	float life = 0.0f;

	bool  level_end_reached = false;
	float level_end_timer   = 0.0f;
	bool  dash = false;
};

class ParticlePlayerTrait : public chaos::ParticleLayerTrait<ParticlePlayer, VertexBase>
{
public:

	class UpdatePlayerData
	{
	public:


		std::vector<ParticleEnemy> enemy_particles;
	};


	bool UpdateParticle(float delta_time, ParticlePlayer * particle, chaos::ParticleAllocation * allocation, UpdatePlayerData const & update_data) const;

	size_t ParticleToVertices(ParticlePlayer const * particle, VertexBase * vertices, size_t vertices_per_particle, chaos::ParticleAllocation * allocation) const;

	UpdatePlayerData BeginUpdateParticles(float delta_time, ParticlePlayer * particles, size_t count, chaos::ParticleAllocation * allocation) const;

public:

	class LudumGame * game = nullptr;
};


// ===========================================================================
// ParticleAtom
// ===========================================================================

class ParticleAtom : public ParticleBase
{
public:

	float particle_radius_factor = 1.0f;
	float life = 0.0f;
	bool  waken_up = false;

};

class ParticleAtomTrait : public chaos::ParticleLayerTrait<ParticleAtom, VertexBase>
{
public:

	class UpdateAtomData
	{
	public:

		ParticlePlayer player_particle;

		float slowing_factor;
		float max_velocity;
		float world_clamp_radius;

		std::vector<ParticleEnemy> enemy_particles;
	};

	bool UpdateParticle(float delta_time, ParticleAtom * particle, chaos::ParticleAllocation * allocation, UpdateAtomData const & update_data) const;

	size_t ParticleToVertices(ParticleAtom const * particle, VertexBase * vertices, size_t vertices_per_particle, chaos::ParticleAllocation * allocation) const;

	UpdateAtomData BeginUpdateParticles(float delta_time, ParticleAtom * particles, size_t count, chaos::ParticleAllocation * allocation) const;

public:

	class LudumGame * game = nullptr;
};






// ===========================================================================
// Object particle system
// ===========================================================================

class ParticleLife : public chaos::ParticleDefault::Particle
{

};

class ParticleLifeTrait : public chaos::ParticleLayerTrait<ParticleLife, VertexBase>
{
public:

	bool UpdateParticle(float delta_time, ParticleLife * particle, chaos::ParticleAllocation * allocation) const;

	size_t ParticleToVertices(ParticleLife const * particle, VertexBase * vertices, size_t vertices_per_particle, chaos::ParticleAllocation * allocation) const;

public:

	class LudumGame * game = nullptr;
};


