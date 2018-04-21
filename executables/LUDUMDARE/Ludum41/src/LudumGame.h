#pragma once

#include "LudumStateMachine.h"
#include "LudumSequenceChallenge.h"

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>
#include <chaos/GeometryFramework.h>
#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/SoundManager.h>
#include <chaos/MyGLFWSingleWindowApplication.h>
#include <chaos/ParticleManager.h>


// =================================================
// LudumGame
// =================================================

class LudumGame : public chaos::ReferencedObject
{
	friend class LudumSequenceChallenge;
	friend class LudumWindow;
	friend class LudumAutomata;
	friend class MainMenuState;
	friend class PlayingToPauseTransition;
	friend class PauseToPlayingTransition;
	friend class MainMenuToPlayingTransition;
	friend class PlayingToMainMenuTransition;
	friend class PlayingState;
	friend class PlayingToGameOverTransition;

public:

	/** the tick method */
	void Tick(double delta_time);
	/** whenever a key event is received */
	bool OnKeyEvent(int key, int action);
	/** whenever a mouse event is received */
	void OnMouseButton(int button, int action, int modifier);
	/** the rendering method */
	void Display(chaos::box2 const & viewport);
	/** called whenever a gamepad input is comming */
	bool OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad);

	/** basic initialization */
	bool InitializeGame(GLFWwindow * in_glfw_window);
	/** initialization from the config file */
	bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path);

protected:

	/** getter on the application */
	chaos::MyGLFW::SingleWindowApplication * GetApplication();
	/** getter on the sound manager */
	chaos::SoundManager * GetSoundManager();

	/** internal method to update the state of the game */
	void UpdateGameState(double delta_time);

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

	/** create the music used in the game */
	void CreateAllMusics();
	/** create one sound */
	chaos::Sound * PlaySound(char const * name, bool paused, bool looping);
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
	bool InitializeGamepadButtonInfo();
	/** initialize the particle manager */
	bool InitializeParticleManager();

	/** called whenever the input mode changes */
	void OnInputModeChanged(int new_mode, int old_mode);

	

	/** test whether a button is being pressed and whether it correspond to the current challenge */
	void SendGamepadButtonToChallenge(chaos::MyGLFW::PhysicalGamepad * physical_gamepad);
	/** test whether a key is being pressed and whether it correspond to the current challenge */
	void SendKeyboardButtonToChallenge(int key);

	/** create a challenge for a given name */
	LudumSequenceChallenge * CreateSequenceChallenge(size_t len);

	/** called whenever a challenge is completed */
	void OnChallengeCompleted(LudumSequenceChallenge * challenge);

protected:

	/** the window in GLFW library */
	GLFWwindow * glfw_window = nullptr;

	/** the automata corresponding to the game */
	boost::intrusive_ptr<LudumAutomata> game_automata;

	/** the current stick position */
	glm::vec2 left_stick_position  = glm::vec2(0.0f, 0.0f);
	glm::vec2 right_stick_position = glm::vec2(0.0f, 0.0f);

	/** the sounds being played */
	boost::intrusive_ptr<chaos::Sound> menu_music;
	boost::intrusive_ptr<chaos::Sound> game_music;
	boost::intrusive_ptr<chaos::Sound> pause_music;

	/** the current gamepad manager */
	boost::intrusive_ptr<chaos::MyGLFW::GamepadManager> gamepad_manager;

	/** the texture atlas */
	boost::intrusive_ptr<chaos::BitmapAtlas::TextureArrayAtlas> texture_atlas;

	/** the dictionnary */
	std::map<size_t, std::vector<std::string>> dictionnary;
	/** the min and max size */
	int min_word_size = 0;
	int max_word_size = 0;


	/** the challenge */
	boost::intrusive_ptr<LudumSequenceChallenge> sequence_challenge;

	/** a mapping between the button index and its resource name */
	std::map<int, std::string> gamepad_button_map;

	/** the particle manager */
	boost::intrusive_ptr<chaos::ParticleManager> particle_manager;
};

// =================================================
// LudumGamepadManager
// =================================================

class LudumGamepadManager : public chaos::MyGLFW::GamepadManager
{
public:

	/** the constructor */
	LudumGamepadManager(LudumGame * in_game) : game(in_game) {}

protected:

	/** the gamepad manager */
	virtual bool DoPoolGamepad(chaos::MyGLFW::PhysicalGamepad * physical_gamepad) override;

protected:

	/** pointer on the game */
	LudumGame * game = nullptr;
};