#include "Ludum42Game.h"
#include "Ludum42Particles.h"
#include "Ludum42Level.h"

#include <chaos/JSONTools.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/BitmapAtlasGenerator.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/FileTools.h>
#include <chaos/WinTools.h>
#include <chaos/Application.h>
#include <chaos/InputMode.h>
#include <chaos/GeometryFramework.h>
#include <chaos/CollisionFramework.h>



LudumGame::LudumGame()
{		
	game_name = "Escape Paouf 3";
}



void LudumGame::OnEnterMainMenu(bool very_first)
{
	death::Game::OnEnterMainMenu(very_first);


}

bool LudumGame::OnEnterPause()
{
	death::Game::OnEnterPause();
	return true;
}

bool LudumGame::OnLeavePause()
{
	death::Game::OnLeavePause();
	return true;
}

bool LudumGame::OnEnterGame()
{
	death::Game::OnEnterGame();
	CreateAllGameObjects(0);
	return true;
}

bool LudumGame::OnLeaveGame(bool gameover)
{
	death::Game::OnLeaveGame(gameover);
	return true;
}

bool LudumGame::OnAbordGame()
{
	death::Game::OnAbordGame();
	DestroyGameObjects();
	return true;
}

bool LudumGame::OnCharEvent(unsigned int c)
{

	return true;
}

bool LudumGame::OnKeyEvent(int key, int action)
{
	if (death::Game::OnKeyEvent(key, action))
		return true;

	// FORCE GAMEOVER
#if _DEBUG
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			cheat_next_level = true;
#endif

	return false;
}

bool LudumGame::OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad)
{
	// ignore invalid gamepad : should never happen
	if (!physical_gamepad->IsAnyAction())
		return true;

	// change the application mode
	chaos::Application::SetApplicationInputMode(chaos::InputMode::Gamepad);

	// cache the stick position
	glm::vec2 lsp = physical_gamepad->GetXBOXStickDirection(chaos::MyGLFW::XBOX_LEFT_AXIS);
	if (glm::length2(lsp) > 0.0f)
		left_stick_position = gamepad_sensitivity * lsp;
	else
	{
		if (physical_gamepad->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_LEFT))
			left_stick_position.x = -gamepad_sensitivity * 1.0f;
		else if (physical_gamepad->IsButtonPressed(chaos::MyGLFW::XBOX_BUTTON_RIGHT))
			left_stick_position.x =  gamepad_sensitivity * 1.0f;
	}

	glm::vec2 rsp = physical_gamepad->GetXBOXStickDirection(chaos::MyGLFW::XBOX_RIGHT_AXIS);
	if (glm::length2(rsp) > 0.0f)
		right_stick_position = gamepad_sensitivity * rsp;

	// maybe a start game
	if (physical_gamepad->IsAnyButtonPressed())
		if (game_automata->main_menu_to_playing->TriggerTransition(true))
			return true;

	// maybe a game/pause resume
	if (
		(physical_gamepad->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_SELECT) == chaos::MyGLFW::BUTTON_BECOME_PRESSED) ||
		(physical_gamepad->GetButtonChanges(chaos::MyGLFW::XBOX_BUTTON_START) == chaos::MyGLFW::BUTTON_BECOME_PRESSED))
	{
		if (RequireTogglePause())
			return true;
	}

	return true;
}

void LudumGame::DoDisplay(chaos::box2 const & viewport, chaos::GPUProgramProvider & uniform_provider)
{
	// clear the color buffers
	glm::vec4 clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);

	// clear the depth buffers
	float far_plane = 1000.0f;
	glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);

	// some states
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// draw particle system
	if (particle_manager != nullptr)
		particle_manager->Display(&uniform_provider);
}

void LudumGame::OnInputModeChanged(int new_mode, int old_mode)
{

}

void LudumGame::ResetGameVariables()
{
	death::Game::ResetGameVariables();
	current_life  = initial_life;
	current_level = 0;
}

void LudumGame::OnGameOver()
{
	death::Game::OnGameOver();
	DestroyGameObjects();
}

void LudumGame::ChangeLife(int delta_life)
{
	if (delta_life == 0)
		return;
	current_life = chaos::MathTools::Maximum(current_life + delta_life, 0);
}

bool LudumGame::CheckGameOverCondition(double delta_time)
{
	if (current_life <= 0)
	{
		RequireGameOver();
		return true;
	}
	return false;
}

bool LudumGame::IsLevelCompleted()
{

	return false;
}

bool LudumGame::TickGameLoop(double delta_time)
{
	// super call
	if (!death::Game::TickGameLoop(delta_time))
		return false;
	// displace the player
	DisplacePlayer(delta_time);
	// test whether current level is terminated
	TickLevelCompleted(delta_time);



	return true;
}


void LudumGame::OnMouseMove(double x, double y)
{
	left_stick_position.x = mouse_sensitivity * (float)x;
}

void LudumGame::DestroyGameObjects()
{
	player_allocations = nullptr;
	life_allocations = nullptr;
}

chaos::ParticleAllocation * LudumGame::CreateGameObjects(char const * name, size_t count, int layer_id)
{
	// find layer of concern
	chaos::ParticleLayer * layer = particle_manager->FindLayer(layer_id);
	if (layer == nullptr)
		return nullptr;

	// find bitmap set
	chaos::BitmapAtlas::BitmapSet const * bitmap_set = texture_atlas->GetBitmapSet("sprites");
	if (bitmap_set == nullptr)
		return nullptr;

	// find bitmap entry
	chaos::BitmapAtlas::BitmapEntry const * entry = bitmap_set->GetEntry(name);
	if (entry == nullptr)
		return nullptr;

	// allocate the objects
	chaos::ParticleAllocation * allocation = layer->SpawnParticles(count);
	if (allocation == nullptr)
		return nullptr;

	chaos::ParticleAccessor<ParticleObject> particles = allocation->GetParticleAccessor<ParticleObject>();

	for (size_t i = 0 ; i < count ; ++i)
	{
		ParticleObject & particle = particles[i];
		particle.texcoords = chaos::ParticleTools::GetParticleTexcoords(*entry, texture_atlas->GetAtlasDimension());
	}
		
	return allocation;
}


chaos::ParticleAllocation * LudumGame::CreatePlayer()
{
	// create the object
	chaos::ParticleAllocation * result = CreateGameObjects("player", 1);
	if (result == nullptr)
		return nullptr;

	// set the color
	chaos::ParticleAccessor<ParticleObject> particles = result->GetParticleAccessor<ParticleObject>();
	if (particles.GetCount() == 0)
		return nullptr;

	particles->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	particles->bounding_box.position  = glm::vec2(0.0f, 0.0f);
	particles->bounding_box.half_size = glm::vec2(0.0f, 0.0f);
	
	return result;
}








glm::vec2 LudumGame::GetPlayerPosition() const
{
	chaos::box2 b = GetPlayerBox();
	return b.position;
}

bool LudumGame::SetPlayerPosition(glm::vec2 const & position)
{
	chaos::box2 b = GetPlayerBox();
	b.position = position;
	if (SetPlayerBox(b))
	{
		RestrictedPlayerToScreen();
		return true;
	}
	return false;
}


void LudumGame::RestrictedObjectToScreen(chaos::ParticleAllocation * allocation, size_t index)
{
#if 0
	chaos::box2 box = particle->bounding_box;
	chaos::box2 world = GetWorldBox();
	chaos::RestrictToInside(world, box, false);
#endif




}

void LudumGame::RestrictedPlayerToScreen()
{
	RestrictedObjectToScreen(player_allocations.get(), 0);
}



void LudumGame::CreateAllGameObjects(int level)
{
#if 0
	if (player_allocations == nullptr)
	{
		player_allocations = CreatePlayer();
		SetPlayerPosition(0.0f);
	}
#endif
}

bool LudumGame::FillAtlasGenerationInputWithTileSets(chaos::BitmapAtlas::AtlasInput & input, nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	return chaos::TiledMapTools::GenerateAtlasInput(tiledmap_manager.get(), input, "sprites");
}

bool LudumGame::FillAtlasGenerationInput(chaos::BitmapAtlas::AtlasInput & input, nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	if (!death::Game::FillAtlasGenerationInput(input, config, config_path))
		return false;
	if (!FillAtlasGenerationInputWithTileSets(input, config, config_path))
		return false;
	return true;
}

bool LudumGame::CreateGameAutomata()
{
	game_automata = new LudumAutomata(this);
	if (game_automata == nullptr)
		return false;
	return true;
}

bool LudumGame::DeclareParticleClasses()
{
	chaos::ClassTools::DeclareClass<ParticleObject>("ParticleObject");
	chaos::ClassTools::DeclareClass<ParticleBackground>("ParticleBackground");
	return true;
}

bool LudumGame::InitializeGameValues(nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	if (!death::Game::InitializeGameValues(config, config_path))
		return false;
	DEATHGAME_JSON_ATTRIBUTE(initial_life);
	return true;
}

bool LudumGame::DoLoadLevelInitialize(LudumNarrativeLevel * level, nlohmann::json const & json_level)
{



	return true;
}

bool LudumGame::DoLoadLevelInitialize(LudumGameplayLevel * level, chaos::TiledMap::Map * tiled_map)
{
	// initialize level
	level->tiled_map = tiled_map;


	return true;
}


death::GameLevel * LudumGame::DoLoadLevel(int level_number, chaos::FilePathParam const & path)
{
	boost::filesystem::path const & resolved_path = path.GetResolvedPath();

	if (chaos::FileTools::IsTypedFile(resolved_path, "json"))
	{
		nlohmann::json json_level;
		if (!chaos::JSONTools::LoadJSONFile(path, json_level, false))
			return nullptr;

		// allocate a level
		LudumNarrativeLevel * ludum_result = new LudumNarrativeLevel(this);
		if (ludum_result == nullptr)
			return false;
		// some additionnal computation
		if (!DoLoadLevelInitialize(ludum_result, json_level))
		{
			delete(ludum_result);
			return nullptr;
		}	
		return ludum_result;	
	}
	else if (chaos::FileTools::IsTypedFile(resolved_path, "tmx"))
	{
		// load the resource
		chaos::TiledMap::Map * tiled_map = tiledmap_manager->LoadMap(path);
		if (tiled_map == nullptr)
			return false;	
	
		// allocate a level
		LudumGameplayLevel * ludum_result = new LudumGameplayLevel(this);
		if (ludum_result == nullptr)
			return false;
		// some additionnal computation
		if (!DoLoadLevelInitialize(ludum_result, tiled_map))
		{
			delete(ludum_result);
			return nullptr;
		}	
		return ludum_result;
	}
	return nullptr;
}

bool LudumGame::LoadLevels()
{
	// create the manager
	tiledmap_manager = new chaos::TiledMap::Manager;
	if (tiledmap_manager == nullptr)
		return false;
	// super call
	return death::Game::LoadLevels();
}

void LudumGame::FillBackgroundLayer()
{
	chaos::ParticleLayer * layer = particle_manager->FindLayer(BACKGROUND_LAYER_ID);
	if (layer == nullptr)
		return;

	background_allocations = layer->SpawnParticles(1);
	if (background_allocations == nullptr)
		return;

	chaos::ParticleAccessor<ParticleBackground> particles = background_allocations->GetParticleAccessor<ParticleBackground>();
	if (particles.GetCount() == 0)
		return;

	particles->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

bool LudumGame::InitializeParticleManager()
{
	if (!death::Game::InitializeParticleManager())
		return false;

	int render_order = 0;

	particle_manager->AddLayer<ParticleBackgroundTrait>(++render_order, BACKGROUND_LAYER_ID, "background");
	particle_manager->AddLayer<ParticleObjectTrait>(++render_order, GROUND_LAYER_ID, "gameobject");
	particle_manager->AddLayer<ParticleObjectTrait>(++render_order, WALLS_LAYER_ID, "gameobject");
	particle_manager->AddLayer<ParticleObjectTrait>(++render_order, GAMEOBJECT_LAYER_ID, "gameobject");
	particle_manager->AddLayer<ParticleObjectTrait>(++render_order, PLAYER_LAYER_ID, "gameobject");
	particle_manager->AddLayer<ParticleObjectTrait>(++render_order, TEXT_LAYER_ID, "text");

	// fill the background
	FillBackgroundLayer();

	return true;
}

bool LudumGame::InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path)
{
	if (!death::Game::InitializeFromConfiguration(config, config_path))
		return false;

	return true;
}

void LudumGame::OnLevelChanged(death::GameLevel * new_level, death::GameLevel * old_level, death::GameLevelInstance * new_level_instance, death::GameLevelInstance * old_level_instance)
{
	death::Game::OnLevelChanged(new_level, old_level, new_level_instance, old_level_instance);

}

bool LudumGame::SpawnPlayer(ParticleObject const & particle_object)
{
	if (player_allocations != nullptr) // already existing
		return false;

	chaos::ParticleLayer * layer = GetParticleManager()->FindLayer(LudumGame::PLAYER_LAYER_ID);
	if (layer == nullptr)
		return false;

	player_allocations = layer->SpawnParticles(1);
	if (player_allocations == nullptr)
		return false;

	chaos::ParticleAccessor<ParticleObject> particles = player_allocations->GetParticleAccessor<ParticleObject>();
	particles[0] = particle_object;

	return true;
}

void LudumGame::UnSpawnPlayer()
{
	player_allocations = nullptr;
}


chaos::box2 LudumGame::GetPlayerBox() const
{
	return GetObjectBox(player_allocations.get(), 0);
}

bool LudumGame::SetPlayerBox(chaos::box2 const & in_player_box)
{

	return SetObjectBox(player_allocations.get(), 0, in_player_box);
}

chaos::box2 LudumGame::GetObjectBox(chaos::ParticleAllocation const * allocation, size_t index) const
{
	if (allocation == nullptr)
		return chaos::box2();

	chaos::ParticleConstAccessor<ParticleObject> particles = allocation->GetParticleConstAccessor<ParticleObject>();
	if (index >= particles.GetCount())
		return chaos::box2();

	return particles[index].bounding_box;
}


bool LudumGame::SetObjectBox(chaos::ParticleAllocation * allocation, size_t index, chaos::box2 const & b)
{
	if (allocation == nullptr)
		return false;

	chaos::ParticleAccessor<ParticleObject> particles = allocation->GetParticleAccessor<ParticleObject>();
	if (index >= particles.GetCount())
		return false;

	particles[index].bounding_box = b;
	return true;
}


void LudumGame::TickLevelCompleted(double delta_time)
{

#if _DEBUG
	if (cheat_next_level)
	{
		SetNextLevel(false);
		return;
	}
#endif

	LudumLevelInstance const * level_instance = dynamic_cast<LudumLevelInstance const *>(GetCurrentLevelInstance());
	if (level_instance == nullptr)
		return;


}

void LudumGame::DisplacePlayer(double delta_time)
{
	glm::vec2 value;

	value.x = (abs(right_stick_position.x) > abs(left_stick_position.x))?
		right_stick_position.x:
		left_stick_position.x;

	value.y = (abs(right_stick_position.y) > abs(left_stick_position.y))?
		-right_stick_position.y:
		-left_stick_position.y;

	glm::vec2 position = GetPlayerPosition();
	SetPlayerPosition(position + value);
}