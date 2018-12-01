#pragma once

#include "Ludum43StateMachine.h"
#include "Ludum43Particles.h"
#include "Ludum43Level.h"

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>
#include <chaos/GeometryFramework.h>
#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/SoundManager.h>
#include <chaos/MyGLFWSingleWindowApplication.h>
#include <chaos/ParticleManager.h>
#include <chaos/ParticleTextGenerator.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/BitmapAtlasGenerator.h>
#include <chaos/TiledMap.h>
#include <chaos/TiledMapTools.h>

#include <death/Game.h>

// =================================================
// LudumGame
// =================================================

namespace death
{
	namespace GameHUDKeys
	{
		CHAOS_DECLARE_TAG(PLANETS_LAYER_ID);
		CHAOS_DECLARE_TAG(GAMEOBJECT_LAYER_ID);
	};
};

class LudumGame : public death::Game
{
	friend class LudumWindow;
	friend class LudumStateMachine;
	friend class MainMenuState;
	friend class PlayingToPauseTransition;
	friend class PauseToPlayingTransition;
	friend class MainMenuToPlayingTransition;
	friend class PlayingToMainMenuTransition;
	friend class PlayingState;
	friend class PlayingToGameOverTransition;

	friend class LudumLevelInstance;

public:

	/** constructor */
	LudumGame();

	/** override */
	virtual bool OnKeyEvent(int key, int action) override;
	/** override */
	virtual bool OnCharEvent(unsigned int c) override;
	/** override */
	virtual void OnMouseMove(double x, double y) override;
	/** override */
	virtual void DoDisplay(chaos::RenderParams const & render_params, chaos::GPUProgramProvider & uniform_provider) override;
	/** override */
	virtual bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path) override;

	/** called whenever a gamepad input is comming */
	virtual bool OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad) override;

	virtual bool OnGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data) override;

protected:

	/** override */
	virtual chaos::SM::StateMachine * DoCreateGameStateMachine() override;
	/** override */
	virtual bool DeclareParticleClasses() override;

	/** creating all object in the game */
	void CreateAllGameObjects(int level);
	/** destroying game objects*/
	void DestroyGameObjects();

	/** override */
	virtual void OnEnterMainMenu(bool very_first) override;
	/** override */
	virtual void OnGameOver() override;
	/** override */
	virtual bool OnEnterPause() override;
	/** override */
	virtual bool OnLeavePause() override;

	/** override */
	virtual bool OnEnterGame() override;
	/** override */
	virtual bool OnLeaveGame(bool gameover) override;
	/** override */
	virtual bool OnAbordGame() override;

	/** override */
	virtual bool TickGameLoop(double delta_time) override;

	/** cooldown the weapon */
	void TickCooldown(double delta_time);
	void TickDashValues(double delta_time);
		
	/** the game main loop */
	virtual bool CheckGameOverCondition() override;

	/** initialize the particle manager */
	virtual int AddParticleLayers() override;
	/** initialize the game variables */
	virtual bool InitializeGameValues(nlohmann::json const & config, boost::filesystem::path const & config_path) override;
	
	/** override level creation */
	death::TiledMap::Level * CreateTiledMapLevel() override;

	/** called whenever the input mode changes */
	virtual void OnInputModeChanged(int new_mode, int old_mode) override;


	/** reset the game variables */
	virtual void ResetGameVariables() override;

	/** ensure object is inside the world */
	void RestrictObjectToWorld(chaos::ParticleAllocation * allocation, size_t index);
	/** ensure player is inside the world */
	void RestrictPlayerToWorld();


	virtual void TickGameInputs(double delta_time) override;
	virtual void HandleGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data) override;
	virtual void HandleKeyboardInputs() override;

	/** move the player */
	void DisplacePlayer(double delta_time);

	void ChangeLife(int delta_life);

	/** override */
	virtual void OnLevelChanged(death::GameLevel * new_level, death::GameLevel * old_level, death::GameLevelInstance * new_level_instance, death::GameLevelInstance * old_level_instance) override;

	void ConditionnalStartDash();

protected:

	/** the tiled map manager */
	boost::intrusive_ptr<chaos::TiledMap::Manager> tiledmap_manager;

public:

	/** game settings */
	int initial_life = 3;
	float cooldown = 0.1f;

	float dash_duration = 0.5f;
	float dash_cooldown = 2.0f;
	float dash_speed_multiplier = 2.0f;

	float player_attraction_minradius = 50.0f;
	float player_attraction_maxradius = 200.0f;
	float particle_slowing_factor     = 0.5f;
	float particle_radius_rand_factor = 1.2f;
	float tangent_force               = 500000.0f;
	float player_attraction_force     = 20.0f;
	float enemy_attraction_radius = 200.0f;

	float particle_max_velocity = 20.0f;

	/** current game values */
	int current_life     = 3;
	float current_cooldown = 0.1f;

	float current_dash_cooldown   = 0.0f;
	float current_dash_time = 0.0f;
	glm::vec2 current_dash_direction = glm::vec2(0.0f, 0.0f);


};

