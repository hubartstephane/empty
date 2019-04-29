#pragma once


#include "Ludum44Particles.h"
#include "Ludum44Game.h"
#include "Ludum44GameInstance.h"
#include "Ludum44LevelInstance.h"

#include <chaos/CollisionFramework.h>
#include <chaos/ClassTools.h>
#include <chaos/ParticleTools.h>

chaos::GPUVertexDeclaration GetTypedVertexDeclaration(boost::mpl::identity<VertexBase>)
{
	chaos::GPUVertexDeclaration result;
	result.Push(chaos::SEMANTIC_POSITION, 0, chaos::TYPE_FLOAT2);
	result.Push(chaos::SEMANTIC_TEXCOORD, 0, chaos::TYPE_FLOAT3);
	result.Push(chaos::SEMANTIC_COLOR, 0, chaos::TYPE_FLOAT4);
	return result;
}


// ===========================================================================
// FindEnemiesOnMap
// ===========================================================================


static void FindEnemiesOnMap(LudumGame * game, std::vector<ParticleEnemy*> & result)
{
	// get the enemies
	death::TiledMap::LayerInstance * enemies_layer_instance = game->GetLudumLevelInstance()->FindLayerInstance("Enemies");
	if (enemies_layer_instance != nullptr)
	{
		chaos::ParticleLayerBase * layer = enemies_layer_instance->GetParticleLayer();
		if (layer != nullptr)
		{
			size_t allocation_count = layer->GetAllocationCount();
			for (size_t i = 0 ; i < allocation_count ; ++i)
			{
				chaos::ParticleAllocationBase * allocation = layer->GetAllocation(i);
				if (allocation != nullptr)
				{
					chaos::ParticleAccessor<ParticleEnemy> enemies = allocation->GetParticleAccessor<ParticleEnemy>();
					size_t count = enemies.GetCount();
					for (size_t j = 0 ; j < count ; ++j)
						result.push_back(&enemies[j]);
				}				
			}			
		}
	}
}

static float OnCollisionWithEnemy(ParticleEnemy * enemy, float damage, LudumGame * game, bool collision_with_player) // returns the life damage produced by the enemy collision (its life)
{
	float result = collision_with_player? enemy->damage_for_player : enemy->life;

	// update life from both size
	enemy->life -= damage;
	enemy->touched_count_down = 2;

	// play sound
	if (enemy->life > 0.0f)
		game->PlaySound("metallic", false, false);
	else 
	{
		if (!collision_with_player)
			game->GetPlayer(0)->SetScore(enemy->score, true);
		game->PlaySound("explosion", false, false);
	}
	return result;
}

// ===========================================================================
// ParticlePlayerTrait
// ===========================================================================

size_t ParticlePlayerTrait::ParticleToVertices(ParticlePlayer const * p, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	return chaos::ParticleDefault::ParticleTrait::ParticleToVertices(p, vertices, vertices_per_particle);
}


bool ParticlePlayerTrait::UpdateParticle(float delta_time, ParticlePlayer * particle, LayerTrait const * layer_trait) const
{
	// find all enemies
	std::vector<ParticleEnemy*> enemies;
	FindEnemiesOnMap(layer_trait->game, enemies);

	// seach a collision for all enemies
	size_t count = enemies.size();
	for (size_t i = 0 ; i < count ; ++i)
	{
		ParticleEnemy * enemy = enemies[i];
		if (enemy->life > 0.0f)
		{
			if (chaos::Collide(particle->bounding_box, enemy->bounding_box))
			{
				float life_lost = OnCollisionWithEnemy(enemy, enemy->life, layer_trait->game, true); // destroy the enemy always
			
				LudumPlayer * player = layer_trait->game->GetLudumPlayer(0);
				player->SetLifeBarValue(-life_lost, true);

			}
		}
	}

	// displace the player
	particle->bounding_box.position += delta_time * particle->velocity;

	return false;
}



// =====================================
// TileParticleTraitExt
// =====================================

bool TileParticleTraitExt::UpdateParticle(float delta_time, death::TiledMap::TileParticle * particle)
{
	if (particle->gid == 0)
	{
		particle->color.a -= delta_time;
		if (particle->color.a <= 0.0f) // fade out the particle
			return true;	
	}
	return false;
}








// ===========================================================================
// ParticleLifeTrait
// ===========================================================================


bool ParticleLifeTrait::UpdateParticle(float delta_time, ParticleLife * particle, LayerTrait const * layer_trait) const
{

	return false;
}

size_t ParticleLifeTrait::ParticleToVertices(ParticleLife const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	return chaos::ParticleDefault::ParticleTrait::ParticleToVertices(particle, vertices, vertices_per_particle);
}


// ===========================================================================
// ParticleFireTrait
// ===========================================================================


ParticleFireUpdateData ParticleFireTrait::BeginUpdateParticles(float delta_time, ParticleFire * particle, size_t count, LayerTrait const * layer_trait) const
{
	ParticleFireUpdateData result;
	if (count > 0)
	{
		// get the camera box 
		result.camera_box = layer_trait->game->GetLudumLevelInstance()->GetCameraBox();
		result.camera_box.half_size *= 3.0f;
		// get the enemies
		FindEnemiesOnMap(layer_trait->game, result.enemies);
		// get the players
		result.player = layer_trait->game->GetLudumPlayer(0);
	}
	return result;
}


bool ParticleFireTrait::UpdateParticle(float delta_time, ParticleFire * particle, ParticleFireUpdateData const & update_data, LayerTrait const * layer_trait) const
{
	// all damage consummed
	if (particle->damage <= 0.0f)
		return true;
	// outside the camera
	if (!chaos::Collide(update_data.camera_box, particle->bounding_box)) // destroy the particle outside the camera frustum (works for empty camera)
		return true;	
	// search for collisions
	if (particle->player_ownership)
	{
		size_t count = update_data.enemies.size();
		for (size_t i = 0 ; i < count ; ++i)
		{
			ParticleEnemy * enemy = update_data.enemies[i];
			if (enemy->life > 0.0f)
			{
				if (chaos::Collide(particle->bounding_box, enemy->bounding_box))
				{
					particle->damage -= OnCollisionWithEnemy(enemy, particle->damage, layer_trait->game, false);

					// kill bullet ?
					if (particle->damage <= 0.0f)
						return true;
					if (!particle->trample)
						return true;
				}			
			}
		}	
	}
	// enemy bullet
	if (!particle->player_ownership && update_data.player != nullptr)
	{
		if (chaos::Collide(particle->bounding_box, update_data.player->GetPlayerBox())) // destroy the particle outside the camera frustum (works for empty camera)
		{				
			update_data.player->SetLifeBarValue(-particle->damage, true);
			particle->damage = 0.0f;
		}	
	}

	// update position velocity
	particle->bounding_box.position += delta_time * particle->velocity;

	return false; // do not destroy the particle
}

size_t ParticleFireTrait::ParticleToVertices(ParticleFire const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	chaos::ParticleTools::GenerateBoxParticle(particle->bounding_box, particle->texcoords, vertices, particle->rotation);
	// copy the color in all triangles vertex
	for (size_t i = 0; i < 6; ++i)
		vertices[i].color = particle->color;
	return 6;
}

// ===========================================================================
// ParticleEnemyTrait
// ===========================================================================

ParticleEnemyUpdateData ParticleEnemyTrait::BeginUpdateParticles(float delta_time, ParticleEnemy * particle, size_t count, LayerTrait const * layer_trait) const
{
	ParticleEnemyUpdateData result;
	if (count > 0)
	{
		result.camera_box = layer_trait->game->GetLudumLevelInstance()->GetCameraBox();
		result.camera_box.half_size *= 3.0f;

		// search some bitmap layout information
		chaos::BitmapAtlas::FolderInfo const * bitmap_set = layer_trait->game->GetTextureAtlas()->GetFolderInfo("sprites");
		if (bitmap_set != nullptr)
		{
			chaos::BitmapAtlas::BitmapInfo const * fire_info = bitmap_set->GetBitmapInfo("fire");
			if (fire_info != nullptr)
				result.fire_layout = *fire_info;
		}	

		// get the layer of interrest
		death::TiledMap::LayerInstance * layer_instance = layer_trait->game->GetLudumLevelInstance()->FindLayerInstance("fire");
		if (layer_instance != nullptr)
		{
			chaos::ParticleLayerBase * fire_layer = layer_instance->GetParticleLayer();
			if (fire_layer != nullptr)
			{
				if (fire_layer->GetAllocationCount() > 0)
					result.fire_allocation = fire_layer->GetAllocation(0);
				else
					result.fire_allocation = fire_layer->SpawnParticles(0);
			}
		}
	}
	return result;
}

bool ParticleEnemyTrait::UpdateParticle(float delta_time, ParticleEnemy * particle, ParticleEnemyUpdateData const & update_data, LayerTrait const * layer_trait) const
{
	// destroy the particle if no life
	if (particle->life <= 0.0f)
		return true;
	// destroy the particle if outside a BIG camera box
	if (!chaos::Collide(update_data.camera_box, particle->bounding_box)) // destroy the particle outside the camera frustum (works for empty camera)
		return true;	

	// apply velocity
	particle->bounding_box.position += delta_time * particle->velocity;
	// apply rotation
	
	if (particle->rotation_following_player)
	{
		LudumPlayer * player = layer_trait->game->GetLudumPlayer(0);
		if (player != nullptr)
		{		
			glm::vec2 delta_pos = player->GetPlayerPosition() - particle->bounding_box.position;
			particle->rotation = atan2f(delta_pos.y, delta_pos.x) - (float)M_PI_2; 
		}
	}
	else
	{
		particle->rotation += delta_time * particle->rotation_speed;	
	}

	// update blinking effect
	if (particle->touched_count_down > 0)
		--particle->touched_count_down;



	if (particle->fire_frequency > 0.0f)
	{
		particle->current_fire_timer += delta_time;
		if (particle->current_fire_timer >= particle->fire_frequency)
		{
			
			int count = 4;
			float delta_angle = 2.0f * (float)M_PI / (float)count;
			ParticleFire * p = layer_trait->game->GetLudumGameInstance()->FireProjectile(update_data.fire_allocation.get(), particle->bounding_box, update_data.fire_layout, 0.3f, count, nullptr, delta_angle, false, layer_trait->game->enemy_fire_velocity, particle->rotation);
			if (p != nullptr)
			{
				for (int i = 0 ; i < count ; ++i)
				{
					p[i].damage = layer_trait->game->enemy_fire_damage;
					p[i].trample = false;
				}
			}
			particle->current_fire_timer = 0.0f;		
		}
	}

	return false; // do not destroy the particle
}

size_t ParticleEnemyTrait::ParticleToVertices(ParticleEnemy const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	chaos::ParticleTools::GenerateBoxParticle(particle->bounding_box, particle->texcoords, vertices, particle->rotation);
	// select wanted color
	glm::vec4 color = particle->color;

	if (particle->touched_count_down > 0)
		color.a = 0.0f;
	else
		color.a = 1.0f;
	
	// copy the color in all triangles vertex
	for (size_t i = 0; i < 6; ++i)
		vertices[i].color = color;
	return 6;
}

