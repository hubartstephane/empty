#include "Ludum47Player.h"
#include "Ludum47Level.h"
#include "Ludum47LevelInstance.h"
#include "Ludum47Game.h"
#include "Ludum47GameInstance.h"
#include "Ludum47Particles.h"

#include <chaos/MathTools.h>
#include <chaos/ParticleLayer.h>

#include <death/Level.h>
#include <death/SoundContext.h>

ParticlePlayer* LudumPlayer::GetPlayerParticle()
{
	if (pawn == nullptr)
		return nullptr;
	return pawn->GetParticle<ParticlePlayer>(0);
}

ParticlePlayer const * LudumPlayer::GetPlayerParticle() const
{
	if (pawn == nullptr)
		return nullptr;
	return pawn->GetParticle<ParticlePlayer>(0);
}

bool LudumPlayer::Initialize(death::GameInstance * in_game_instance)
{
	if (!death::Player::Initialize(in_game_instance))
		return false;



	return true;
}

bool LudumPlayer::InitializeGameValues(nlohmann::json const& config, boost::filesystem::path const& config_path, bool hot_reload)
{
	if (!Player::InitializeGameValues(config, config_path, hot_reload))
		return false;
	DEATHGAME_JSON_ATTRIBUTE(max_velocity);
	DEATHGAME_JSON_ATTRIBUTE(acceleration);
	DEATHGAME_JSON_ATTRIBUTE(angular_velocity);
	DEATHGAME_JSON_ATTRIBUTE(normal_deceleration);
	DEATHGAME_JSON_ATTRIBUTE(break_deceleration);
	return true;
}

void LudumPlayer::InternalHandleGamepadInputs(float delta_time, chaos::MyGLFW::GamepadData const * gpd)
{
	death::Player::InternalHandleGamepadInputs(delta_time, gpd);


}

void LudumPlayer::HandleKeyboardInputs(float delta_time)
{
	death::Player::HandleKeyboardInputs(delta_time);


}

void LudumPlayer::OnLifeLost()
{
	death::Player::OnLifeLost();
	
}

void LudumPlayer::OnLevelChanged(death::Level * new_level, death::Level * old_level, death::LevelInstance * new_level_instance)
{
	death::Player::OnLevelChanged(new_level, old_level, new_level_instance);

	if (new_level_instance != nullptr)
	{
		LudumLevelInstance* li = auto_cast(new_level_instance);
		if (li != nullptr)
		{
			road = li->road;
		}
	}
	// reset the race
	race_position = {};
}

bool LudumPlayer::DoTick(float delta_time)
{
	if (!death::Player::DoTick(delta_time))
		return false;

	if (road != nullptr)
	{
		ParticlePlayer const * particle = GetPlayerParticle();
		if (particle != nullptr)
		{
			road->UpdateRacePosition(race_position, particle->bounding_box.position, true);



		}
	}
	return true;
}
