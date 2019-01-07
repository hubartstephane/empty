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
#include <chaos/GPUFramebuffer.h>
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
	virtual void DoDisplay(chaos::Renderer * renderer, chaos::GPUProgramProvider * uniform_provider, chaos::RenderParams const & render_params) override;
	/** override */
	virtual bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path) override;

	/** called whenever a gamepad input is comming */
	virtual bool OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad) override;

	virtual bool OnGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data) override;

	void RegisterEnemiesInRange(glm::vec2 const & center, float radius, std::vector<ParticleEnemy> & enemy_particles, char const * layer_name, bool take_all);

	void NotifyAtomCountChange(int delta);

	virtual chaos::box2 GetWorldBox() const override;

	int GetWakenUpParticleCount() const { return waken_up_particle_count; }

	int GetSavedParticleCount() const { return current_score; }

	float GetLevelTimeout() const { return level_timeout; }

	float GetPlayerLife() const;

	ParticlePlayer * GetPlayerParticle();

	ParticlePlayer const * GetPlayerParticle() const;

protected:

	virtual void SetPlayerAllocation(chaos::ParticleAllocation * in_allocation) override;

	void SetPlayerReverseMode(bool reversed_mode);

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

	void TickHeartBeat(double delta_time);

	/** cooldown the weapon */
	void TickCooldown(double delta_time);
	void TickDashValues(double delta_time);
		
	/** the game main loop */
	virtual bool CheckGameOverCondition() override;

	virtual void OnLevelChanged(death::GameLevel * new_level, death::GameLevel * old_level, death::GameLevelInstance * new_level_instance, death::GameLevelInstance * old_level_instance);

	/** initialize the particle manager */
	virtual int AddParticleLayers() override;
	/** initialize the game variables */
	virtual bool InitializeGameValues(nlohmann::json const & config, boost::filesystem::path const & config_path, bool hot_reload) override;
	
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


	virtual void HandleGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data) override;
	virtual void HandleKeyboardInputs() override;

	/** move the player */
	void UpdatePlayerAcceleration(double delta_time);

	void SetPlayerDashMode(bool dash);

	virtual death::GameHUD * DoCreatePlayingHUD() override;

	bool GenerateFramebuffer(glm::ivec2 const & size, chaos::shared_ptr<chaos::GPUFramebuffer> & in_framebuffer);

protected:

	/** the render buffers */
	chaos::shared_ptr<chaos::GPUFramebuffer> framebuffer_worldlimits;
	chaos::shared_ptr<chaos::GPUFramebuffer> framebuffer_other;

	/** the tiled map manager */
	chaos::shared_ptr<chaos::TiledMap::Manager> tiledmap_manager;

public:

	/** game settings */
	int initial_life = 3;
	float cooldown = 0.1f;

	float dash_duration = 0.5f;
	float dash_cooldown = 1.0f;
	float dash_velocity = 200.0f;

	float player_attraction_minradius = 50.0f;
	float player_attraction_maxradius = 200.0f;
	float player_tangent_force        = 500000.0f;
	float player_attraction_force     = 20.0f;
	float player_repulsion_force      = 20.0f;
	
	float player_slowing_factor       = 0.5f;
	float player_max_velocity         = 20.0f;
	float player_acceleration         = 2000.0f;

	float worldlimits_attraction_maxradius_offset = 0.0f;
	float worldlimits_attraction_minradius_offset = 0.0f;
	float enemy_attraction_maxradius_offset = 200.0f;
	float enemy_attraction_minradius_offset = 50.0f;

	float worldlimits_attraction_maxradius_factor = 1.0f;
	float worldlimits_attraction_minradius_factor = 0.5f;
	float enemy_attraction_maxradius_factor = 1.0f;
	float enemy_attraction_minradius_factor = 1.0f;

	float enemy_tangent_force        = 500000.0f;
	float enemy_attraction_force     = 20.0f;
	float enemy_repulsion_force      = 20.0f;
	


	float particle_slowing_factor     = 0.5f;
	float particle_min_radius_factor = 1.0f;
	float particle_max_radius_factor = 3.0f;
	
	float particle_max_velocity = 20.0f;

	float world_clamp_radius = 3000.0f;

	/** current game values */
	float initial_player_life = 4.0f;
	float initial_particle_life = 1.0f;
	


	float current_cooldown = 0.1f;
	float current_dash_duration = 0.0f;
	float current_dash_cooldown = 0.0f;


	float previous_frame_life = 0.0f;

	/** number of waken up particle */
	int waken_up_particle_count = 0;
	/** the time to run the level */
	float level_timeout = 0.0f;
	/** the default level requirement */
	int level_particle_requirement = 0;
	/** the heart beat time */
	float heart_beat_time = 0.0f;


};

