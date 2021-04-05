#include <chaos/Chaos.h>

#include "Ludum41HUD.h"
#include "Ludum41Game.h"
#include "Ludum41GameInstance.h"
#include "Ludum41Level.h"
#include "Ludum41LevelInstance.h"
#include "Ludum41Player.h"



// ====================================================================
// GameHUDComboComponent
// ====================================================================

GameHUDComboComponent::GameHUDComboComponent() :
	chaos::GameHUDCacheValueComponent<int>("Combo: %d x") 
{
	generator_params.line_height = 60.0f;
	generator_params.font_info_name = "normal";
	generator_params.position = glm::vec2(20.0f, -80.0f);
	generator_params.hotpoint = chaos::Hotpoint::TOP_LEFT;
}

int GameHUDComboComponent::QueryValue() const
{
	LudumGameInstance const* ludum_game_instance = GetGameInstance();
	if (ludum_game_instance == nullptr)
		return -1;
	return ludum_game_instance->GetCurrentComboMultiplier();
}

void GameHUDComboComponent::UpdateTextMesh()
{
	if (cached_value <= 0)
		mesh = nullptr;
	else
		chaos::GameHUDCacheValueComponent<int>::UpdateTextMesh();
}

// ====================================================================
// LudumPlayingHUD
// ====================================================================

bool LudumPlayingHUD::FillHUDContent()
{
	if (!chaos::PlayingHUD::FillHUDContent())
		return false;
	RegisterComponent(chaos::GameHUDKeys::COMBO_ID, new GameHUDComboComponent());
	RegisterComponent(chaos::GameHUDKeys::LIFE_ID, new chaos::GameHUDLifeComponent());
	return true;
}
