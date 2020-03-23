#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/ParticleManager.h>
#include <chaos/Tickable.h>
#include <chaos/InputEventReceiver.h>

#include <death/GameGettersDecl.h>
#include <death/GameFramework.h>
#include <death/Game.h>
#include <death/PlayerPawn.h>

namespace death
{

	// =============================================
	// Player
	// =============================================

	class Player : public chaos::Tickable, public chaos::InputEventReceiver, public CheckpointObject<PlayerCheckpoint>
	{
		friend class PlayerGamepadCallbacks;

		DEATH_GAMEFRAMEWORK_ALLFRIENDS()
		
	public:

		DEATH_GAMEGETTERS_DECLARE();

		/** constructor */
		Player(GameInstance * in_game_instance);
		/** destructor */
		virtual ~Player();

		/** get the index of the player */
		size_t GetPlayerIndex() const { return player_index; }

		/** get the current score */
		int GetScore() const { return score; }
		/** get the life count */
		int GetLifeCount() const { return life_count; }
        /** get the current health value */
        float GetHealth() const { return health; }
        /** get the maximum health value health value */
        float GetMaxHealth() const { return max_health; }

		/** update player score */
		void SetScore(int in_score, bool increment = false);
		/** update player life */
		void SetLifeCount(int in_life, bool increment = false);
        /** update player health */
        void SetHealth(float in_value, bool in_increment = false);

		/** check whether the player is dead */
		virtual bool IsDead() const;

		/** try to give physical device to the player */
		bool CapturePhysicalGamepad(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad);



#if 0

		// XXX : player allocation is not necessarly in one of the game particle_manager's layer
		//       it can be set from a level instance's particle_manager

		/** get the player allocation */
		chaos::ParticleAllocationBase* GetPlayerAllocation() { return player_allocations.get(); }
		/** get the player allocation */
		chaos::ParticleAllocationBase const* GetPlayerAllocation() const { return player_allocations.get(); }
		/** set the player allocation */
		virtual void SetPlayerAllocation(chaos::ParticleAllocationBase* in_allocation);

		/** get player particle */
		chaos::ParticleDefault::Particle * GetPlayerParticle();
		chaos::ParticleDefault::Particle const * GetPlayerParticle() const;

		/** get player position */
		glm::vec2 GetPlayerPosition() const;
		/** get player box */
		chaos::box2 GetPlayerBox() const;
		/** set the player position */
		bool SetPlayerPosition(glm::vec2 const & position);
		/** set the player box */
		bool SetPlayerBox(chaos::box2 const & box);

#endif
		/** gets the pawn */
		PlayerPawn* GetPawn() { return pawn.get(); }
		/** gets the pawn */
		PlayerPawn const * GetPawn() const { return pawn.get(); }

		/** gets the left stick position */
		glm::vec2 GetLeftStickPosition() const { return left_stick_position; }
		/** gets the right stick position */
		glm::vec2 GeRightStickPosition() const { return right_stick_position; }

		/** gets the left trigger */
		float GetLeftTrigger() const { return left_trigger; }
		/** gets the right trigger */
		float GetRightTrigger() const { return right_trigger; }

        /** utility function to check whether a keyboard key or gamepad button is down */
        bool CheckButtonPressed(int const* keyboard_buttons, int gamepad_button);

		/** player initialization from config */
		virtual bool InitializeGameValues(nlohmann::json const& config, boost::filesystem::path const& config_path, bool hot_reload);
		/** called after player configuration has been (re)loaded */
		virtual void OnGameValuesChanged(bool hot_reload);

	protected:

		/** initialize the game instance */
		virtual bool Initialize(death::GameInstance * in_game_instance);

		/** called whenever the level is being changed */
		virtual void OnLevelChanged(death::Level * new_level, death::Level * old_level, death::LevelInstance * new_level_instance);

		/** override */
		virtual bool DoTick(float delta_time) override;
		/** override */
		virtual GLFWwindow* GetGLFWWindow() const override;

		/** extract stick values from keyboard state */
		virtual void CacheKeyboardPlayerDisplacementInputs();
		/** extract stick values from gamepad state */
		virtual void CacheGamepadPlayerDisplacementInputs();
		/** handle the player input */
		virtual void HandleKeyboardInputs(float delta_time);
		/** handle the player input (top level function) */
		virtual void HandleGamepadInputs(float delta_time);
		/** handle the special commands for gamepad (like pause ...) */
		virtual void HandleSpecialGamepadCommands(float delta_time);

		/** handle the input for our own purpose (consuming any data) */
		virtual void InternalHandleGamepadInputs(float delta_time, chaos::MyGLFW::GamepadData const * gamepad_data);		
		/** reset the cached inputs (exists because we emulate keyboard arrow combinaison as un stick) */
		virtual void ResetCachedInputs();

		/** called whenever player gamepad is disconnected */
		virtual void OnGamepadDisconnected();

		/** tick the player displacement */
		virtual void TickPlayerDisplacement(float delta_time);
		/** tick the invulnerability */
		virtual void TickInvulnerability(float delta_time);

		/** called whenever a life is lost */
		virtual void OnLifeLost();

		/** override */
		virtual bool DoSaveIntoCheckpoint(PlayerCheckpoint * checkpoint) const override;
		/** override */
		virtual bool DoLoadFromCheckpoint(PlayerCheckpoint const * checkpoint) override;

        /** called whenever health is changed */
        virtual void OnHealthChanged(float old_health, float new_health, bool invulnerable);

	protected:

		/** the game instance owning the player */
		GameInstance* game_instance = nullptr;

		/** a gamepad that can be given to the player */
		chaos::shared_ptr<chaos::MyGLFW::Gamepad> gamepad;

		/** the player pawn */
		chaos::shared_ptr<PlayerPawn> pawn;

#if 0
		/** the allocation for the player */
		chaos::shared_ptr<chaos::ParticleAllocationBase> player_allocations;
#endif

		/** the index of the player */
		size_t player_index = 0;

		/** the current life for the player */
		int life_count = 3;
		/** the current health of the player */
		float health = 1.0f;
		/** the maximum health of the player */
		float max_health = 1.0f;
		/** the current timer of invulnerability */
		float invulnerability_timer = 0.0f;
		/** the invulnerability after some health is lost */
		float invulnerability_duration = 1.0f;
		/** the score for the player */
		int score = 0;

		/** the current stick position */
		glm::vec2 left_stick_position = glm::vec2(0.0f, 0.0f);
		glm::vec2 right_stick_position = glm::vec2(0.0f, 0.0f);
		float left_trigger = 0.0f;
		float right_trigger = 0.0f;
	};

	// =============================================
	// A callback object for gamepad disconnection
	// =============================================

	class PlayerGamepadCallbacks : public chaos::MyGLFW::GamepadCallbacks
	{

	public:

		/** constructor */
		PlayerGamepadCallbacks(Player * in_player);
		/** override */
		virtual bool OnGamepadDisconnected(chaos::MyGLFW::Gamepad * in_gamepad) override;

	protected:

		/** player of interrest */
		Player * player = nullptr;
	};


}; // namespace death
