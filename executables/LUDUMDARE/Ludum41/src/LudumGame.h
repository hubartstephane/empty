#pragma once

#include "LudumStateMachine.h"
#include "LudumChallenge.h"
#include "LudumParticles.h"
#include "LudumChallengeRewardPunishment.h"

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>
#include <chaos/GeometryFramework.h>
#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/SoundManager.h>
#include <chaos/MyGLFWSingleWindowApplication.h>
#include <chaos/ParticleManager.h>
#include <chaos/ParticleTextGenerator.h>

#include <death/Game.h>

// =================================================
// Levels
// =================================================

class LudumLevel : public chaos::ReferencedObject
{

public:

	static int const NONE = -1;
	static int const INDESTRUCTIBLE = -2;

	/** get the number of brick in the level */
	size_t GetBrickCount() const;

public:

	/** the number of the level */
	int level_number = 0;
	/** indestructible brick count */
	size_t indestructible_brick_count = 0;
	/** the brick types */
	std::vector<std::vector<int>> bricks;
};


// =================================================
// LudumGame
// =================================================

class LudumGame : public death::Game
{
	friend class LudumChallenge;
	friend class LudumWindow;
	friend class LudumAutomata;
	friend class MainMenuState;
	friend class PlayingToPauseTransition;
	friend class PauseToPlayingTransition;
	friend class MainMenuToPlayingTransition;
	friend class PlayingToMainMenuTransition;
	friend class PlayingState;
	friend class PlayingToGameOverTransition;

	friend class LudumChallengeRewardPunishment_ExtraLife;
	friend class LudumChallengeRewardPunishment_BarSize;
	friend class LudumChallengeRewardPunishment_BrickLife;
	friend class LudumChallengeRewardPunishment_SpeedDownBall;
	friend class LudumChallengeRewardPunishment_SplitBall;
	friend class LudumChallengeRewardPunishment_BallPower;
	friend class LudumChallengeRewardPunishment_BrickOffset;

	friend class ParticleMovableObjectTrait;
	friend class ParticleLifeObjectTrait;
	friend class ParticleBrickTrait;

protected:

	/** some aspect constant */
	float CHALLENGE_SIZE = 100.0f;
	float CHALLENGE_PLACEMENT_Y = 0;

	float TITLE_SIZE = 150.0f;
	float TITLE_PLACEMENT_Y = 0;

	float PLAYER_Y = -385.0f;
	float PLAYER_HEIGHT = 35.0f;

public:

	/** destructor */
	~LudumGame();

	/** override */
	virtual void Tick(double delta_time) override;
	/** override */
	virtual bool OnKeyEvent(int key, int action) override;
	/** override */
	virtual bool OnCharEvent(unsigned int c) override;
	/** override */
	virtual void OnMouseButton(int button, int action, int modifier) override;
	/** override */
	virtual void OnMouseMove(double x, double y) override;
	/** override */
	virtual void Display(glm::ivec2 const & size) override;
	/** override */
	virtual bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path) override;

protected:

	/** internal methods to generate the atlas for sprites */
	bool GenerateAtlas(nlohmann::json const & config, boost::filesystem::path const & config_path);

	/** internal method called to reset cached inputs */
	void ResetPlayerCachedInputs();
	/** internal method to test for keys and update stick position */
	void HandleKeyboardInputs();

	/** require a pause or resume */
	bool RequireTogglePause();
	/** require a game Start */
	bool RequireStartGame();
	/** require a game exit */
	bool RequireExitGame();
	/** require a game over */
	bool RequireGameOver();

	/** creating all object in the game */
	void CreateAllGameObjects(int level);
	/** destroying game objects*/
	void DestroyGameObjects();

	/** override */
	virtual bool CreateGameAutomata() override;
	/** override */
	virtual bool CreateAllMusics() override;
	/** override */
	virtual bool DeclareParticleClasses() override;
	/** override */
	virtual void HandleGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data) override;
	/** override */
	virtual void OnGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data) override;

	

	/** blend out a music */
	void BlendMusic(chaos::Sound * music, bool blend_in);
	/** start music[0], stop all others */
	void ChangeMusic(chaos::Sound ** musics, size_t count, bool restart_first);


	/** called on the very first time the game is started */
	void OnStartGame(bool very_first);
	/** called whenever the game is lost */
	void OnGameOver();
	/** called whenever we enter in pause mode */
	bool OnEnterPause();
	/** called whenever we leave pause mode */
	bool OnLeavePause();

	/** called whenever we enter in game mode */
	bool OnEnterGame();
	/** called whenever we leave game mode */
	bool OnLeaveGame();

	/** returns true if the pause if fully set */
	bool IsPauseEnterComplete();
	/** returns true if the game if fully restored from pause */
	bool IsPauseLeaveComplete();

	/** returns true if the game enter if fully set */
	bool IsGameEnterComplete();
	/** returns true if the game leave is fully completed */
	bool IsGameLeaveComplete();

	/** the game main loop */
	void TickGameLoop(double delta_time);
	
	void TickBrickOffset(double delta_time);

	void TickLevelCompleted(double delta_time);

	void TickChallenge(double delta_time);

	void TickBallSplit(double delta_time);

	void TickHeartWarning(double delta_time);

	bool TickGameOverDetection(double delta_time);

	/** change the game music */
	void StartMainMenuMusic(bool restart_first);
	void StartGameMusic(bool restart_first);
	void StartPauseMusic(bool restart_first);

	/** get current state ID */
	int GetCurrentStateID() const;

	/** initialize the dictionnary */
	bool InitializeDictionnary(nlohmann::json const & config, boost::filesystem::path const & config_path);
	/** replace some special */
	void ReplaceSpecialLetters(std::string & word) const;
	/** test whether a word only has the common letters (no accent) */
	bool IsWordValid(std::string const & word) const;

	/** initialize the mapping between button index and resource name */
	virtual bool InitializeGamepadButtonInfo() override;
	/** initialize the particle manager */
	virtual bool InitializeParticleManager() override;
	/** initialize the game variables */
	bool InitializeGameValues(nlohmann::json const & config, boost::filesystem::path const & config_path) override;
	/** fullfill the lists of rewards an punishments */
	bool InitializeRewardsAndPunishments();
	
	/** loading the levels */
	virtual bool LoadLevels() override;
	/** load one level */
	bool DoLoadLevel(int level_number, std::vector<std::string> & level_content);


	
	/** get a random button in existing list */
	int GetRandomButtonID() const;


	/** get the size of the world */
	glm::vec2 GetWorldSize() const;
	/** get the box world */
	chaos::box2 GetWorldBox() const;

	/** override */
	virtual void OnInputModeChanged(int new_mode, int old_mode) override;

	

	/** test whether a button is being pressed and whether it correspond to the current challenge */
	void SendGamepadButtonToChallenge(chaos::MyGLFW::GamepadData * in_gamepad_data);
	/** test whether a key is being pressed and whether it correspond to the current challenge */
	void SendKeyboardButtonToChallenge(unsigned int C);

	/** create a challenge for a given name */
	LudumChallenge * CreateSequenceChallenge(size_t len);

	/** called whenever a challenge is completed */
	void OnChallengeCompleted(LudumChallenge * challenge, bool success, size_t challenge_size);

	/** create one particle for the background */
	void FillBackgroundLayer();
	/** create a text for the challenge */
	chaos::ParticleAllocation * CreateChallengeParticles(LudumChallenge * challenge);

	/** create a string for a gamepad challenge */
	std::string GenerateGamepadChallengeString(std::vector<int> const & gamepad_challenge);

	/** create particles in the text layer */
	chaos::ParticleAllocation * CreateTextParticles(char const * text, chaos::ParticleTextGenerator::GeneratorParams const & params);

	/** create a number of game object */
	chaos::ParticleAllocation * CreateGameObjects(char const * name, size_t count, int layer_id = GAMEOBJECT_LAYER_ID);
	/** create the player */
	chaos::ParticleAllocation * CreatePlayer();

	/** create the ball */
	chaos::ParticleAllocation * CreateBalls(size_t count, bool full_init);
	/** create the bricks */
	chaos::ParticleAllocation * CreateBricks(int level_number);

	/** create the title of the game */
	void CreateGameTitle();
	/** create the title */
	void CreateTitle(char const * title, bool normal);
	/** create the title */
	void DestroyTitle();

	/** update the score */
	void IncrementScore(int delta);

	/** reset the game variables */
	void ResetGameVariables();

	/** called whenever a ball collide */
	void OnBallCollide(bool collide_brick);
	/** change the player position */
	void SetPlayerPosition(float position);
	/** change the length */
	void SetPlayerLength(float length);




	/** get an object particle */
	ParticleObject * GetObjectParticle(chaos::ParticleAllocation * allocation, size_t index);
	/** get an object particle */
	ParticleObject const * GetObjectParticle(chaos::ParticleAllocation * allocation, size_t index) const;

	/** get the player particle */
	ParticleObject * GetPlayerParticle();
	/** get the player particle */
	ParticleObject const * GetPlayerParticle() const;

	/** get the balls */
	ParticleMovableObject * GetBallParticles();
	/** get the balls */
	ParticleMovableObject const * GetBallParticles() const;
	/** get the number of balls */	
	size_t GetBallCount() const;

	/** get the bricks */
	ParticleBrick * GetBricks();
	/** get the bricks */
	ParticleBrick const * GetBricks() const;

	/** get the number of bricks */
	size_t GetBrickCount() const;


	/** returns true whether we can start a challenge (returns index of a valid ball) */
	size_t CanStartChallengeBallIndex(bool reverse = false) const;



	/** get the box for a given object */
	chaos::box2 GetObjectBox(chaos::ParticleAllocation * allocation, size_t index) const;
	/** get the box for player */
	chaos::box2 GetPlayerBox() const;

	/** change one object box */
	void SetObjectBox(chaos::ParticleAllocation * allocation, size_t index, chaos::box2 const & box);
	/** change player box */
	void SetPlayerBox(chaos::box2 const & box);

	/** get the position for given object */
	glm::vec2 GetObjectPosition(chaos::ParticleAllocation * allocation, size_t index) const;
	/** get the player position */
	glm::vec2 GetPlayerPosition() const;








	/** change an object position */
	void SetObjectPosition(chaos::ParticleAllocation * allocation, size_t index, glm::vec2 const & position);

	/** ensure object is inside the world */
	void RestrictedObjectToScreen(chaos::ParticleAllocation * allocation, size_t index);
	/** ensure player is inside the world */
	void RestrictedPlayerToScreen();


	/** generate a direction updward random for the ball */
	glm::vec2 GenerateBallRandomDirection() const;

	/** move the player */
	void DisplacePlayer(double delta_time);


	/** some challenges */
	void OnBrickLifeChallenge(bool success);
	bool IsBrickLifeChallengeValid(bool success);
	/** some challenges */
	void OnBallSpeedChallenge(bool success);
	bool IsBallSpeedChallengeValid(bool success);
	/** some challenges */
	void OnBrickOffsetChallenge(bool success);
	bool IsBrickOffsetChallengeValid(bool success);
	/** some challenges */
	void OnBallPowerChallenge(bool success);
	bool IsBallPowerChallengeValid(bool success);
	/** some challenges */
	void OnSplitBallChallenge(bool success);
	bool IsSplitBallChallengeValid(bool success);
	/** some challenges */
	void OnExtraBallChallenge(bool success);
	bool IsExtraBallChallengeValid(bool success);
	/** some challenges */
	void OnLongBarChallenge(bool success);
	bool IsLongBarChallengeValid(bool success);

	/** returns true whether we are playing */
	bool IsPlaying() const;

	/** create the score allocation */
	void UpdateScoreParticles();
	/** create the combo allocation */
	void UpdateComboParticles();
	/** create the life allocation */
	void UpdateLifeParticles();

	void ChangeLife(int delta_life);

	/** internal method to create score/combo */
	chaos::ParticleAllocation * CreateScoringParticles(bool & update_flag, char const * format, int value, float Y);


	/** get currently played level */
	LudumLevel * GetCurrentLevel();
	/** get currently played level */
	LudumLevel const * GetCurrentLevel() const;

	/** get currently played level */
	LudumLevel * GetLevel(int level_number);
	/** get currently played level */
	LudumLevel const * GetLevel(int level_number) const;

	/** override */
	virtual bool LoadBestScore(std::ifstream & file) override;
	/** override */
	virtual bool SaveBestScore(std::ofstream & file) override;

protected:

	/** the automata corresponding to the game */
	boost::intrusive_ptr<LudumAutomata> game_automata;

	/** the current stick position */
	glm::vec2 left_stick_position  = glm::vec2(0.0f, 0.0f);
	glm::vec2 right_stick_position = glm::vec2(0.0f, 0.0f);

	/** the sounds being played */
	boost::intrusive_ptr<chaos::Sound> menu_music;
	boost::intrusive_ptr<chaos::Sound> game_music;
	boost::intrusive_ptr<chaos::Sound> pause_music;

	/** the dictionnary */
	std::map<size_t, std::vector<std::string>> dictionnary;
	/** the min and max size */
	int min_word_size = 0;
	int max_word_size = 0;


	/** the challenge */
	boost::intrusive_ptr<LudumChallenge> sequence_challenge;

	/** all the existing button */
	std::vector<int> gamepad_buttons;




	/** initial game values */
	int initial_life = 3;
	int max_life = 10;

	int max_ball_count = 3;

	float brick_offset = 0.0f;
	float target_brick_offset = 0.0f;
	float max_brick_offset = 0.0f;
	float brick_offset_speed = 0.0f;
	float brick_offset_increment = 0.0f;

	float player_max_length       = 200.0f; 
	float player_min_length       = 70.0f; 
	float player_initial_length   = 100.0f; 
	float player_length_increment = 50.0f; 
	float player_length_decrement = 50.0f;

	float ball_size          = 25.0f;

	float ball_power         = 1.0f;
	float ball_speed         = 5.0f;
	float ball_max_speed     = 5.0f;
	float ball_initial_speed = 1.0f;
	float ball_speed_increment = 0.5f;

	float ball_collision_speed    = 0.5f;
	float ball_collision_speed_increment = 0.5f;
	float ball_collision_max_speed = 0.5f;
	
	float mouse_sensitivity   = 1.0f;
	float gamepad_sensitivity = 1.0f;

	float challenge_time_dilation = 0.5f;
	float challenge_frequency = 10.0f;
	float challenge_duration   = 5.0f;
	
	float delay_before_ball_move = 2.0f;

	float heart_beat_speed = 2.0f;

	int min_brick_life = 1;
	float max_brick_life = 5.0f;

	int pending_split_count = 0;

	int brick_per_line   = 5;
	int brick_line_count = 5;

	int points_per_brick = 1;
	int points_per_challenge = 10;

	float split_angle = 0.1f;
	float ball_angle_limit = 0.1f;

	/** current game values */
	int   current_life  = 3;
	float player_length = 70.0f;
	
	float ball_time_dilation = 1.0f;
	float challenge_timer = 0.0f;

	

	int current_score = 0;
	int combo_multiplier = 1;
	int best_score = 0;
	int current_level = 0;

	bool should_update_score = false;

	bool should_update_combo = false;

	float heart_warning = 0.0f;

#if _DEBUG
	bool cheat_next_level = false;
#endif

	/** some sprites */
	boost::intrusive_ptr<chaos::ParticleAllocation> player_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> bricks_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> life_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> balls_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> text_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> score_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> combo_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> best_score_allocations;
	boost::intrusive_ptr<chaos::ParticleAllocation> background_allocations;

	/** the possible rewards */
	std::vector<boost::intrusive_ptr<LudumChallengeRewardPunishment>> rewards;
	/** the possible punishment */
	std::vector<boost::intrusive_ptr<LudumChallengeRewardPunishment>> punishments;

	/** the levels */
	std::vector<boost::intrusive_ptr<LudumLevel>> levels;

};

