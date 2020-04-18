#pragma once


#include "Ludum46Particles.h"
#include "Ludum46Game.h"
#include "Ludum46GameInstance.h"
#include "Ludum46Player.h"
#include "Ludum46LevelInstance.h"
#include "Ludum46Level.h"

#include <chaos/CollisionFramework.h>
#include <chaos/ClassTools.h>
#include <chaos/ParticleTools.h>

#include <death/SoundContext.h>



// ===========================================================================
// ParticleSoulTrait
// ===========================================================================

ParticleSoulUpdateData ParticleSoulTrait::BeginUpdateParticles(float delta_time, chaos::ParticleAccessor<ParticleSoul>& particle_accessor, LayerTrait const* layer_trait) const
{
	ParticleSoulUpdateData result;

	LudumLevelInstance* ludum_level_instance = layer_trait->game->GetLevelInstance();
	if (ludum_level_instance != nullptr)
	{
		result.level_bounding_box = ludum_level_instance->GetBoundingBox();
		result.fire_layer_instance = ludum_level_instance->FindLayerInstance("Fire");
		result.ludum_level_instance = ludum_level_instance;

		// store triggers
		death::TiledMapLayerInstance* layer_instance = ludum_level_instance->FindLayerInstance("Objects");
		if (layer_instance != nullptr)
		{
			size_t count = layer_instance->GetGeometricObjectCount();
			for (size_t i = 0; i < count; ++i)
			{
				SoulTriggerObject* trigger = auto_cast(layer_instance->GetGeometricObject(i));
				if (trigger == nullptr)
					continue;
				result.soul_triggers.push_back(trigger);
			}
		}
	}
	return result;
}

void ParticleSoulTrait::ParticleToPrimitives(ParticleSoul const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const
{
	chaos::ParticleDefault::ParticleTrait::ParticleToPrimitives(particle, output);
}

bool ParticleSoulTrait::UpdateParticle(float delta_time, ParticleSoul* particle, ParticleSoulUpdateData & update_data, LayerTrait const* layer_trait) const
{
	particle->bounding_box.position += delta_time * particle->velocity;

	// out of world
	if (!chaos::Collide(particle->bounding_box, update_data.level_bounding_box))
		return true;

	// out of health
	if (particle->health <= 0.0f)
		return true;

	// out of lifetime
	if (particle->duration > 0.0f)
	{
		particle->life += delta_time;
		if (particle->life > particle->duration)
			return true;

		particle->color.a = 1.0f - (particle->life / particle->duration);	
	}

	// check fire particles

		// the fire elements
	if (update_data.fire_layer_instance != nullptr && update_data.fire_layer_instance->GetParticleLayer() != nullptr)
	{
		size_t count = update_data.fire_layer_instance->GetParticleLayer()->GetAllocationCount();
		for (size_t i = 0; i < count; ++i)
		{
			chaos::ParticleConstAccessor<ParticleFire> accessor = update_data.fire_layer_instance->GetParticleLayer()->GetAllocation(i)->GetParticleAccessor();
			for (ParticleFire const& fire_particle : accessor)
			{
				if (chaos::Collide(fire_particle.bounding_box, particle->bounding_box))
				{
					if (update_data.ludum_level_instance != nullptr)
						update_data.ludum_level_instance->SpawnBloodParticles(particle->bounding_box, 10);

					return true; // touching fire 
				}
			}
		}
	}

	// checking triggers
	for (SoulTriggerObject* trigger : update_data.soul_triggers)
	{
		if (chaos::Collide(trigger->GetBoundingBox(true), particle->bounding_box))
		{
			if (trigger->AddTriggerCount())
			{
				if (update_data.ludum_level_instance != nullptr)
					update_data.ludum_level_instance->SpawnBurnedSoulParticles(particle->bounding_box, 10);

				return true;
			}
		}
	}
	return false;
}




// ===========================================================================
// ParticleFireTrait
// ===========================================================================

ParticleFireUpdateData ParticleFireTrait::BeginUpdateParticles(float delta_time, chaos::ParticleAccessor<ParticleFire>& particle_accessor, LayerTrait const* layer_trait) const
{
	ParticleFireUpdateData result;

	LudumLevelInstance * ludum_level_instance = layer_trait->game->GetLevelInstance();
	if (ludum_level_instance != nullptr)
	{
		result.level_bounding_box = ludum_level_instance->GetBoundingBox();

	}
	return result;
}

void ParticleFireTrait::ParticleToPrimitives(ParticleFire const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const
{
	chaos::ParticleDefault::ParticleTrait::ParticleToPrimitives(particle, output);
}

bool ParticleFireTrait::UpdateParticle(float delta_time, ParticleFire* particle, ParticleFireUpdateData & update_data, LayerTrait const* layer_trait) const
{
	particle->bounding_box.position += delta_time * particle->velocity;

	if (!chaos::Collide(particle->bounding_box, update_data.level_bounding_box))
		return true;

	if (particle->duration > 0.0f)
	{
		particle->life += delta_time;
		if (particle->life > particle->duration)
			return true;

		particle->color.a = 1.0f - (particle->life / particle->duration);
	}
	return false;
}

// ===========================================================================
// ParticleBloodTrait
// ===========================================================================

void ParticleBloodTrait::ParticleToPrimitives(ParticleBlood const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const
{
	chaos::QuadPrimitive<VertexBase> primitive = output.AddPrimitive();

	chaos::box2 box = particle.bounding_box;

	box.half_size *= 1.0f + (particle.life / particle.duration);

	// generate particle corners and texcoords
	chaos::ParticleTools::GenerateBoxParticle(box, particle.texcoords, primitive);
	// copy the color in all triangles vertex
	for (size_t i = 0; i < primitive.count; ++i)
		primitive[i].color = particle.color;
}

bool ParticleBloodTrait::UpdateParticle(float delta_time, ParticleBlood* particle, LayerTrait const* layer_trait) const
{
	particle->bounding_box.position += delta_time * particle->velocity;

	particle->velocity += delta_time * particle->acceleration;

	
	if (particle->duration > 0.0f)
	{
		particle->life += delta_time;
		if (particle->life > particle->duration)
			return true;

		particle->color.a = 1.0f - (particle->life / particle->duration);
	}

	return false;
}


// ===========================================================================
// ParticleBurnedSoulTrait
// ===========================================================================

void ParticleBurnedSoulTrait::ParticleToPrimitives(ParticleBurnedSoul const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const
{
	chaos::QuadPrimitive<VertexBase> primitive = output.AddPrimitive();

	chaos::box2 box = particle.bounding_box;
	box.position.x += 50.0f * std::sin(particle.offset_t);

	// generate particle corners and texcoords
	chaos::ParticleTools::GenerateBoxParticle(box, particle.texcoords, primitive);
	// copy the color in all triangles vertex
	for (size_t i = 0; i < primitive.count; ++i)
		primitive[i].color = particle.color;
}

bool ParticleBurnedSoulTrait::UpdateParticle(float delta_time, ParticleBurnedSoul* particle, LayerTrait const* layer_trait) const
{
	particle->bounding_box.position += delta_time * particle->velocity;

	particle->velocity += delta_time * particle->acceleration;

	if (particle->duration > 0.0f)
	{
		particle->life += delta_time;
		if (particle->life > particle->duration)
			return true;

		particle->color.a = 1.0f - (particle->life / particle->duration);
	}

	particle->offset_t += delta_time;


	return false;
}





// ===========================================================================
// ParticlePlayerTrait
// ===========================================================================

void ParticlePlayerTrait::ParticleToPrimitives(ParticlePlayer const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const
{
    chaos::ParticleDefault::ParticleTrait::ParticleToPrimitives(particle, output);



}

bool ParticlePlayerTrait::UpdateParticle(float delta_time, ParticlePlayer* particle, LayerTrait const* layer_trait) const
{


	return false;
}



