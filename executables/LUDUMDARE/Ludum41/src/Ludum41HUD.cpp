#include "Ludum41HUD.h"
#include "Ludum41Game.h"

bool LudumPlayingHUD::DoTick(double delta_time)
{
	// call super method
	PlayingHUD::DoTick(delta_time);
	
	LudumGame * ludum_game = dynamic_cast<LudumGame *>(game); // Game::PlaySound() in TickHeartWarning(..) requires a non const pointer
	if (ludum_game != nullptr)
	{
		UpdateComboParticles(ludum_game);
		UpdateLifeParticles(ludum_game);		
		TickHeartWarning(ludum_game, delta_time);
	}
	return true;
}

void LudumPlayingHUD::UpdateComboParticles(LudumGame const * ludum_game)
{
	int current_combo = ludum_game->GetCurrentComboMultiplier();
	if (current_combo != cached_combo_value)
	{
		if (current_combo < 2)
			UnregisterParticles(death::GameHUDKeys::COMBO_ID);
		else
			RegisterParticles(death::GameHUDKeys::COMBO_ID, game->CreateScoringText("Combo : %d x", current_combo, 60.0f, particle_manager.get()));

		cached_combo_value = current_combo;
	}
}

void LudumPlayingHUD::UpdateLifeParticles(LudumGame const * ludum_game)
{
	int current_life = ludum_game->GetCurrentLife();
	if (current_life != cached_life_value)
	{
		if (current_life < 0)
			UnregisterParticles(death::GameHUDKeys::LIFE_ID);
		else
		{


		}
		cached_life_value = current_life;
	}
}






#if 0

void LudumGame::UpdateLifeParticles()
{
	// get the number of particles already existing
	size_t life_particles = 0;
	if (life_allocations != nullptr)
		life_particles = life_allocations->GetParticleCount();

	// no changes ?
	if (life_particles == current_life)
		return;

	// some life lost (destroy particles)
	if ((size_t)current_life < life_particles)
	{
		life_allocations->Resize(current_life);
		return;
	}

	// some life gained
	assert(life_particles < (size_t)current_life);

	//if (life_allocations == nullptr)

	life_allocations = CreateGameObjects("life", current_life, death::GameHUDKeys::LIFE_LAYER_ID);
	if (life_allocations == nullptr)
		return;

	// set the color
	chaos::ParticleAccessor<ParticleObject> particles = life_allocations->GetParticleAccessor<ParticleObject>();
	if (particles.GetCount() == 0)
		return;

	glm::vec2 view_size = GetViewSize();

	glm::vec2 particle_size;
	particle_size.x = 35.0f;
	particle_size.y = 20.0f;

	for (size_t i = 0; i < (size_t)current_life; ++i)
	{
		glm::vec2 position;
		position.x = -view_size.x * 0.5f + 20.0f + (particle_size.x + 5.0f) * (float)i;
		position.y = -view_size.y * 0.5f + 15.0f;

		particles[i].bounding_box.position = chaos::Hotpoint::Convert(position, particle_size, chaos::Hotpoint::BOTTOM_LEFT, chaos::Hotpoint::CENTER);
		particles[i].bounding_box.half_size = 0.5f * particle_size;

		particles[i].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

#endif








void LudumPlayingHUD::TickHeartWarning(LudumGame * ludum_game, double delta_time)
{
	if (ludum_game->GetCurrentLife() == 1)
	{
		heart_warning -= heart_beat_speed * (float)delta_time;
		if (heart_warning <= 0.0f)
		{
			ludum_game->PlaySound("heartbeat", false, false);

			float fractionnal_part, integer_part;
			fractionnal_part = modf(heart_warning, &integer_part);

			heart_warning = (1.0f + fractionnal_part);
		}
	}
	else
		heart_warning = 1.0f;
}

bool LudumPlayingHUD::InitializeHUD()
{
	// call super method
	if (!death::PlayingHUD::InitializeHUD())
		return false;
	// create a layer for the life bar
	LudumGame * ludum_game = dynamic_cast<LudumGame *>(game);
	if (ludum_game != nullptr)
	{
		int render_order = -1;
		ParticleLifeObjectTrait life_trait;
		life_trait.game = ludum_game;
		particle_manager->AddLayer<ParticleLifeObjectTrait>(++render_order, death::GameHUDKeys::LIFE_LAYER_ID, "gameobject", life_trait);
	}
	return true;
}



