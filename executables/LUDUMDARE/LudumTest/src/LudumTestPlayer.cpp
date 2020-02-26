#include "LudumTestPlayer.h"
#include "LudumTestLevel.h"
#include "LudumTestLevelInstance.h"
#include "LudumTestGame.h"
#include "LudumTestGameInstance.h"
#include "LudumTestParticles.h"
#include "LudumTestGameCheckpoint.h"

#include <chaos/MathTools.h>
#include <chaos/ParticleLayer.h>

#include <death/Level.h>
#include <death/SoundContext.h>

LudumPlayer::LudumPlayer(death::GameInstance * in_game_instance) : 
	death::Player(in_game_instance)
{
}

bool LudumPlayer::Initialize(death::GameInstance * in_game_instance)
{
	if (!death::Player::Initialize(in_game_instance))
		return false;



	return true;
}

ParticlePlayer * LudumPlayer::GetPlayerParticle()
{
	if (player_allocations == nullptr)
		return nullptr;
	chaos::ParticleAccessor<ParticlePlayer> player_particles = player_allocations->GetParticleAccessor();
	if (player_particles.GetCount() == 0)
		return nullptr;
	return &player_particles[0];
}

ParticlePlayer const * LudumPlayer::GetPlayerParticle() const
{
	if (player_allocations == nullptr)
		return nullptr;
	chaos::ParticleConstAccessor<ParticlePlayer> player_particles = player_allocations->GetParticleAccessor();
	if (player_particles.GetCount() == 0)
		return nullptr;
	return &player_particles[0];
}




void LudumPlayer::TickPlayerDisplacement(float delta_time)
{
    Player::TickPlayerDisplacement(delta_time);




}




void LudumPlayer::InternalHandleGamepadInputs(float delta_time, chaos::MyGLFW::GamepadData const * gpd)
{
	death::Player::InternalHandleGamepadInputs(delta_time, gpd);


}

void LudumPlayer::HandleKeyboardInputs(float delta_time)
{
	death::Player::HandleKeyboardInputs(delta_time);


}

void LudumPlayer::OnLevelChanged(death::Level * new_level, death::Level * old_level, death::LevelInstance * new_level_instance)
{
	death::Player::OnLevelChanged(new_level, old_level, new_level_instance);

}

void LudumPlayer::SetPlayerAllocation(chaos::ParticleAllocationBase * in_allocation)
{
	LudumGame * ludum_game = GetGame();
	if (ludum_game == nullptr)
		return;

	Player::SetPlayerAllocation(in_allocation);
}

death::PlayerCheckpoint * LudumPlayer::DoCreateCheckpoint() const
{
	return new LudumPlayerCheckpoint();
}

bool LudumPlayer::DoLoadFromCheckpoint(death::PlayerCheckpoint const * checkpoint)
{
	LudumPlayerCheckpoint const * ludum_checkpoint = auto_cast(checkpoint);
	if (ludum_checkpoint == nullptr)
		return false;

	if (!death::Player::DoLoadFromCheckpoint(checkpoint))
		return false;





	return true;
}

bool LudumPlayer::DoSaveIntoCheckpoint(death::PlayerCheckpoint * checkpoint) const
{
	LudumPlayerCheckpoint * ludum_checkpoint = auto_cast(checkpoint);
	if (ludum_checkpoint == nullptr)
		return false;

	if (!death::Player::DoSaveIntoCheckpoint(checkpoint))
		return false;



	return true;
}
