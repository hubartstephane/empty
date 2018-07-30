#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/MyGLFWwindow.h>
#include <chaos/MyGLFWGamepadManager.h>
#include <chaos/Application.h>
#include <chaos/MyGLFWSingleWindowApplication.h>
#include <chaos/SoundManager.h>
#include <chaos/GeometryFramework.h>

namespace death
{
	class Game : public chaos::ReferencedObject
	{
		friend class GamepadManager;
		friend class GameWindow;

	public:

		/** initialization of the game */
		virtual bool InitializeGame(GLFWwindow * in_glfw_window);

	protected:

		/** the tick method */
		virtual void Tick(double delta_time);
		/** whenever a key event is received */
		virtual bool OnKeyEvent(int key, int action);
		/** whenever a char event is received */
		virtual bool OnCharEvent(unsigned int c);
		/** whenever a mouse event is received */
		virtual void OnMouseButton(int button, int action, int modifier);
		/** whenever mouse is displaced */
		virtual void OnMouseMove(double x, double y);
		/** the rendering method */
		virtual void Display(chaos::box2 const & viewport);
		/** initialization from the config file */
		virtual bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path);

		/** utility function to get the application */
		chaos::MyGLFW::SingleWindowApplication * GetApplication();
		/** utility function to get the sound manager */
		chaos::SoundManager * GetSoundManager();

		/** play some sound */
		chaos::Sound * PlaySound(char const * name, bool paused, bool looping);

		/** save the best score */
		bool SerializeBestScore(bool save);
		/** data internal method serialization */
		virtual bool LoadBestScore(std::ifstream & file);
		/** data internal method serialization */
		virtual bool SaveBestScore(std::ofstream & file);

		/** create all the music of the game */
		virtual bool CreateAllMusics();
		/** declare all particle types (for safety) */
		virtual bool DeclareParticleClasses();
		/** create the game automata */
		virtual bool CreateGameAutomata();
		/** handle the gamepad input */
		virtual void HandleGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data);
		/** special action on gamepad input reception */
		virtual void OnGamepadInput(chaos::MyGLFW::GamepadData & in_gamepad_data);

		/** called whenever a gamepad input is comming */
		virtual bool OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad);

		/** called whenever the input mode changes */
		virtual void OnInputModeChanged(int new_mode, int old_mode);

	protected:

		/** the window in GLFW library */
		GLFWwindow * glfw_window = nullptr;

		/** the current gamepad manager */
		boost::intrusive_ptr<chaos::MyGLFW::GamepadManager> gamepad_manager;
		/** the gamepad data */
		chaos::MyGLFW::GamepadData gamepad_data;
	};

}; // namespace death
