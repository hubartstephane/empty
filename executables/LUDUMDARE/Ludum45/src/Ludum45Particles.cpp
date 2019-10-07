#pragma once


#include "Ludum45Particles.h"
#include "Ludum45Game.h"
#include "Ludum45GameInstance.h"
#include "Ludum45Player.h"
#include "Ludum45LevelInstance.h"
#include "Ludum45Enemies.h"

#include <chaos/CollisionFramework.h>
#include <chaos/ClassTools.h>
#include <chaos/ParticleTools.h>

#include <death/SoundContext.h>

/*
chaos::GPUVertexDeclaration GetTypedVertexDeclaration(boost::mpl::identity<VertexBase>)
{
	chaos::GPUVertexDeclaration result;
	result.Push(chaos::SEMANTIC_POSITION, 0, chaos::TYPE_FLOAT2);
	result.Push(chaos::SEMANTIC_TEXCOORD, 0, chaos::TYPE_FLOAT3);
	result.Push(chaos::SEMANTIC_COLOR, 0, chaos::TYPE_FLOAT4);
	return result;
}
*/






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



// ===========================================================================
// Utility
// ===========================================================================

static bool ObjectBesideCamera(chaos::box2 const & camera_box, chaos::box2 const & object_box)
{
	float cam_y = camera_box.position.y - camera_box.half_size.y;
	float obj_y = object_box.position.y + object_box.half_size.y;
	if (obj_y < cam_y)
		return true;
	return false;
}




// ===========================================================================
// ParticleEnemyTrait
// ===========================================================================


chaos::box2 ParticleEnemyTrait::BeginUpdateParticles(float delta_time, ParticleEnemy * particle, size_t count, LayerTrait const * layer_trait) const
{
	chaos::box2 result;

	result = layer_trait->game->GetPlayer(0)->GetPlayerBox();

	return result;
}


size_t ParticleEnemyTrait::ParticleToVertices(ParticleEnemy const * p, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	// generate particle corners and texcoords
	chaos::ParticleTools::GenerateBoxParticle(p->bounding_box, p->texcoords, vertices);



	glm::vec4 color = p->color;

	if (p->touched_count_down > 0)
		color.a = 0.0f;
	else
		color.a = 1.0f;

	// copy the color in all triangles vertex
	for (size_t i = 0 ; i < 6 ; ++i)
		vertices[i].color =  p->color;

	return vertices_per_particle;
}


bool ParticleEnemyTrait::UpdateParticle(float delta_time, ParticleEnemy * particle, chaos::box2 const & player_box, LayerTrait const * layer_trait) const
{
	if (particle->enemy_life <= 0.0f)
		return true;

	chaos::box2 bb = particle->bounding_box;
	bb.half_size *= 0.50f;

	// collision with player

	// update blinking effect
	if (particle->touched_count_down > 0)
		--particle->touched_count_down;




	if (chaos::Collide(bb, player_box))
	{
		LudumPlayer * ludum_player = auto_cast(layer_trait->game->GetPlayer(0));
		if (ludum_player != nullptr)			
		{
			if (ludum_player->dash_timer <= 0.0f ||! ludum_player->GetGhostLevel())
			{
				ludum_player->OnDamagedReceived(particle->enemy_damage);
				return true;
			}
		}
	}

	// bitmap animation
	if (particle->bitmap_info != nullptr && particle->bitmap_info->HasGridAnimation())
	{
		particle->image_timer += delta_time;
		if (particle->image_timer > 0.3f)
		{
			particle->image_timer = 0.0f;
			if (++particle->current_frame >= particle->bitmap_info->GetAnimationImageCount())
				particle->current_frame = 0;
		}
	}

	particle->time += delta_time;
	if (particle->pattern != nullptr)
		return particle->pattern->UpdateParticle(delta_time, particle, player_box, layer_trait->game->GetLevelInstance()->GetCameraBox(0));


	return false;
}




































// ===========================================================================
// ParticleBonusTrait
// ===========================================================================

#if 0

static void FindBonusOnMap(LudumGame * game, std::vector<ParticleBonus*> & result)
{
	// get the enemies
	death::TiledMap::LayerInstance * layer_instance = game->GetLudumLevelInstance()->FindLayerInstance("Bonus");
	if (layer_instance != nullptr)
	{
		chaos::ParticleLayerBase * layer = layer_instance->GetParticleLayer();
		if (layer != nullptr)
		{
			size_t allocation_count = layer->GetAllocationCount();
			for (size_t i = 0 ; i < allocation_count ; ++i)
			{
				chaos::ParticleAllocationBase * allocation = layer->GetAllocation(i);
				if (allocation != nullptr)
				{
					chaos::ParticleAccessor<ParticleBonus> bonus = allocation->GetParticleAccessor<ParticleBonus>();
					size_t count = bonus.GetCount();
					for (size_t j = 0 ; j < count ; ++j)
						result.push_back(&bonus[j]);
				}				
			}			
		}
	}
}

#endif


chaos::box2 ParticleBonusTrait::BeginUpdateParticles(float delta_time, ParticleBonus * particle, size_t count, LayerTrait const * layer_trait) const
{
	chaos::box2 result;

	result = layer_trait->game->GetPlayer(0)->GetPlayerBox();

#if 0
	ParticleFireUpdateData result;
	if (count > 0)
	{
		// get the camera box 
		result.camera_box = layer_trait->game->GetLudumLevelInstance()->GetCameraBox(0);
		//result.camera_box.half_size *= 3.0f;
		// get the enemies
		FindEnemiesOnMap(layer_trait->game, result.enemies);
		// get the players
		result.player = layer_trait->game->GetLudumPlayer(0);
	}
#endif
	return result;
}


size_t ParticleBonusTrait::ParticleToVertices(ParticleBonus const * p, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	// generate particle corners and texcoords
	chaos::ParticleTools::GenerateBoxParticle(p->bounding_box, p->texcoords, vertices);
	// copy the color in all triangles vertex
	for (size_t i = 0 ; i < 6 ; ++i)
		vertices[i].color =  p->color;

	return vertices_per_particle;
}


bool ParticleBonusTrait::UpdateParticle(float delta_time, ParticleBonus * particle, chaos::box2 const & player_box, LayerTrait const * layer_trait) const
{
	chaos::box2 bb = particle->bounding_box;
	bb.half_size *= 0.70f;

	if (chaos::Collide(bb, player_box))
	{
		LudumPlayer * ludum_player = auto_cast(layer_trait->game->GetPlayer(0));
		if (ludum_player != nullptr)
			ludum_player->OnPlayerUpgrade(particle->bonus_type);

		return true;
	}
	return false;
}





// ===========================================================================
// ParticlePlayerTrait
// ===========================================================================

size_t ParticlePlayerTrait::ParticleToVertices(ParticlePlayer const * p, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{

	chaos::ParticleTexcoords texcoords = p->texcoords;

	if (p->bitmap_info != nullptr && p->bitmap_info->HasGridAnimation())
	{
		chaos::BitmapAtlas::BitmapLayout layout = p->bitmap_info->GetAnimationLayout(p->current_frame, chaos::BitmapAtlas::GetBitmapLayoutFlag::wrap);

		texcoords =  chaos::ParticleTools::GetParticleTexcoords(layout, glm::ivec2(2048, 2048)); // HARDCODED atlas_size !!!! shuludum => should we not store the coordinate in [0..1] instead (or both) ???

	}

	// generate particle corners and texcoords
	chaos::ParticleTools::GenerateBoxParticle(p->bounding_box, texcoords, vertices, p->orientation);

	glm::vec4 boost_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	LudumPlayer const * player = layer_trait->game->GetLudumPlayer(0);
	if (player != nullptr && player->dash_timer > 0.0f && player->GetGhostLevel() > 0)
	{
		float alpha = 1.0f;

		if (layer_trait->game->player_dash_duration > 0.0f)
			alpha = player->dash_timer / layer_trait->game->player_dash_duration;
		
		alpha = 00.1f;

		boost_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f - alpha);
	}

	// copy the color in all triangles vertex
	for (size_t i = 0 ; i < 6 ; ++i)
		vertices[i].color = boost_color * p->color;

	return vertices_per_particle;
}


bool ParticlePlayerTrait::UpdateParticle(float delta_time, ParticlePlayer * particle, LayerTrait const * layer_trait) const
{
	if (particle->bitmap_info != nullptr && particle->bitmap_info->HasGridAnimation())
	{
		particle->image_timer += delta_time;
		if (particle->image_timer > 0.3f)
		{
			particle->image_timer = 0.0f;
			if (++particle->current_frame >= particle->bitmap_info->GetAnimationImageCount())
				particle->current_frame = 0;
		}
	}
	return false;
}




// ===========================================================================
// ParticleFireTrait
// ===========================================================================

static float OnCollisionWithEnemy(ParticleEnemy * enemy, float damage, LudumGame * game, bool collision_with_player, chaos::box2 const & ref_box) // returns the life damage produced by the enemy collision (its life)
{
	float result = collision_with_player? enemy->enemy_damage : enemy->enemy_life;

	// update life from both size
	enemy->enemy_life -= damage;
	enemy->touched_count_down = 20;

	// play sound
	if (enemy->enemy_life > 0.0f)
		game->Play("metallic", false, false, 0.0f, death::SoundContext::LEVEL);
	else 
	{
		if (!collision_with_player)
			game->GetPlayer(0)->SetScore(10, true);
		game->Play("explosion", false, false, 0.0f, death::SoundContext::LEVEL);
		game->GetLudumGameInstance()->FireExplosion(ref_box);
	}
	return result;
}

ParticleFireUpdateData ParticleFireTrait::BeginUpdateParticles(float delta_time, ParticleFire * particle, size_t count, LayerTrait const * layer_trait) const
{
	ParticleFireUpdateData result;
	if (count > 0)
	{
		// get the camera box 
		result.camera_box = layer_trait->game->GetLudumLevelInstance()->GetCameraBox(0);
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

	particle->lifetime -= delta_time;
	if (particle->lifetime <= 0.0f)
		return true;

	if (particle->player_ownership && !chaos::Collide(update_data.camera_box, particle->bounding_box)) // destroy the particle outside the camera frustum (works for empty camera)
		return true;	


	// search for collisions
	if (particle->player_ownership)
	{
		size_t count = update_data.enemies.size();
		for (size_t i = 0 ; i < count ; ++i)
		{
			ParticleEnemy * enemy = update_data.enemies[i];
			if (enemy->enemy_life > 0.0f)
			{
				if (chaos::Collide(particle->bounding_box, enemy->bounding_box))
				{
					particle->damage -= OnCollisionWithEnemy(enemy, particle->damage, layer_trait->game, false, enemy->bounding_box);

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
	chaos::box2 player_box = update_data.player->GetPlayerBox();
	player_box.half_size *= 0.7f;
	if (!particle->player_ownership && update_data.player != nullptr)
	{
		if (chaos::Collide(particle->bounding_box, player_box)) // destroy the particle outside the camera frustum (works for empty camera)
		{			
			update_data.player->OnDamagedReceived(particle->damage);
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

	float alpha = 1.0f;
	if (particle->lifetime < 1.0f)
		alpha = particle->lifetime;


	for (size_t i = 0; i < 6; ++i)
	{
		vertices[i].color = particle->color;
		vertices[i].color.a = alpha;
	}

	return 6;
}

// ===========================================================================
// ParticleShroudLife
// ===========================================================================

bool ParticleShroudLifeTrait::UpdateParticle(float delta_time, ParticleShroudLife * particle, LayerTrait const * layer_trait) const
{
	if (particle->bitmap_info == nullptr)
		return false;
	LudumPlayer * ludum_player = layer_trait->game->GetLudumPlayer(0);
	if (ludum_player == nullptr)
		return false;

	float current_life = ludum_player->GetCurrentLife();
	float max_life = ludum_player->GetCurrentMaxLife();

	float image_count = (float)particle->bitmap_info->GetAnimationImageCount();

	int index = (int)(image_count * (1.0 - (current_life / max_life)));

	chaos::BitmapAtlas::BitmapLayout layout = particle->bitmap_info->GetAnimationLayout(index, chaos::BitmapAtlas::GetBitmapLayoutFlag::clamp);

	particle->texcoords = chaos::ParticleTools::GetParticleTexcoords(layout, glm::ivec2(2048, 2048));


	return false; // never destroy it
}

size_t ParticleShroudLifeTrait::ParticleToVertices(ParticleShroudLife const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{

	return chaos::ParticleDefault::ParticleTrait::ParticleToVertices(particle, vertices, vertices_per_particle);
}





// ===========================================================================
// ParticleLifeTrait
// ===========================================================================


// shuludum .... duplication AGAIN !!!


bool ParticleLifeTrait::UpdateParticle(float delta_time, ParticleLife * particle) const
{

	return false;
}

size_t ParticleLifeTrait::ParticleToVertices(ParticleLife const * particle, VertexBase * vertices, size_t vertices_per_particle) const
{
	return chaos::ParticleDefault::ParticleTrait::ParticleToVertices(particle, vertices, vertices_per_particle);
}



// ===========================================================================
// ParticleExplosionTrait
// ===========================================================================


bool ParticleExplosionTrait::UpdateParticle(float delta_time, ParticleExplosion * particle, LayerTrait const * layer_trait) const
{
	if (particle->explosion_info == nullptr) // delete the particle
		return true;

	int image_count = particle->explosion_info->GetAnimationImageCount();
	float frame_time = (float)particle->explosion_info->GetFrameTime();

	if (frame_time == 0)
		frame_time = 1 / 16.0f;

	int image_index = (int)(particle->age / frame_time);

	chaos::BitmapAtlas::BitmapLayout bitmap_layout = particle->explosion_info->GetAnimationLayout(image_index, chaos::BitmapAtlas::GetBitmapLayoutFlag::none);
	if (bitmap_layout.bitmap_index < 0)
		return true;

	particle->age += delta_time;

#if 0
	particle->texcoords = bitmap_layout;

	// compute the bounding box for all particles
	chaos::box2 particle_box = ref_box;

	particle_box.half_size = ratio_to_box * ref_box.half_size;

	particle_box.position = ref_box.position;

	// compute texcoords for all particles
#endif
	particle->texcoords = chaos::ParticleTools::GetParticleTexcoords(bitmap_layout, layer_trait->game->GetTextureAtlas()->GetAtlasDimension());



	return false;
}

size_t ParticleExplosionTrait::ParticleToVertices(ParticleExplosion const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{


	return chaos::ParticleDefault::ParticleTrait::ParticleToVertices(particle, vertices, vertices_per_particle);
}
