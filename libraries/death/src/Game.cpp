#include <death/Game.h>
#include <death/GameStateMachine.h>
#include <death/GamepadManager.h>
#include <death/Level.h>
#include <death/TiledMapLevel.h>
#include <death/SoundContext.h>
#include <death/GameGettersImpl.h>
#include <death/SoundListenerCameraComponent.h>
#include <death/FreeCameraComponent.h>
#include <death/GameParticles.h>

#include <chaos/KeyDefinition.h>
#include <chaos/LogTools.h>

#include <chaos/InputMode.h>
#include <chaos/FileTools.h>
#include <chaos/TiledMap.h>
#include <chaos/TiledMapTools.h>
#include <chaos/CollisionFramework.h>
#include <chaos/ParticleDefault.h>



namespace death
{
	DEATH_GAMEGETTERS_IMPLEMENT(Game);

	Game::Game(GLFWwindow* in_glfw_window):
		glfw_window(in_glfw_window)
	{
		assert(in_glfw_window != nullptr);
	}

	int Game::GetBestPlayerScore() const
	{
		if (game_instance != nullptr)
			return game_instance->GetBestPlayerScore();
		return 0;
	}

	void Game::OnInputModeChanged(chaos::InputMode new_mode, chaos::InputMode old_mode)
	{

	}

#if _DEBUG
	void Game::SetCheatSkipLevelRequired(bool value)
	{
		cheat_skip_level_required = value;
	}

	bool Game::GetCheatSkipLevelRequired() const
	{
		return cheat_skip_level_required;
	}

	void Game::SetCheatMode(bool value)
	{
		cheat_mode = value;
	}

	bool Game::GetCheatMode() const
	{
		if (chaos::Application::HasApplicationCommandLineFlag("-CheatMode")) // CMDLINE
			return true;
		if (IsFreeCameraMode())
			return true;
		return cheat_mode;
	}
#endif // _DEBUG

	void Game::TickGameInputs(float delta_time)
	{
		// catch all stick inputs
		if (gamepad_manager != nullptr)
			gamepad_manager->Tick(delta_time);
	}

	void Game::Tick(float delta_time)
	{
		// update player inputs
		TickGameInputs(delta_time);
		// tick the free camera
		if (free_camera != nullptr)
			free_camera->Tick(delta_time);
		// update the game state_machine
		if (game_sm_instance != nullptr)
			game_sm_instance->Tick(delta_time, nullptr);
		// update the game instance
		if (game_instance != nullptr)
			game_instance->Tick(delta_time);		
		// tick the particle manager
		if (particle_manager != nullptr)
			particle_manager->Tick(delta_time);
		// tick the hud
		if (hud != nullptr)
			hud->Tick(delta_time);
	}

	bool Game::OnKeyEventImpl(chaos::KeyEvent const& event)
	{
		// try start the game
		if (game_instance == nullptr)
		{
			if (event.IsKeyPressed())
				RequireStartGame(nullptr);
			return true;
		}

		// give opportunity to game instance to response		
		if (game_instance != nullptr)
			if (game_instance->OnKeyEvent(event))
				return true;

		// PLAYING to PAUSE
		if (event.IsKeyPressed(GLFW_KEY_KP_ENTER) || event.IsKeyPressed(GLFW_KEY_ENTER))
			if (RequireTogglePause())
				return true;
		// QUIT GAME
		if (event.IsKeyPressed(GLFW_KEY_ESCAPE, GLFW_MOD_SHIFT))
		{
			if (RequireExitGame())
				return true;
		}
		else if (event.IsKeyPressed(GLFW_KEY_ESCAPE))
		{
			if (RequireTogglePause())
				return true;
		}
		// CHEAT CODE TO SKIP LEVEL
#if _DEBUG
			
		// CMD GLFW_KEY_F1  : SetCheatSkipLevelRequired(...)	
		if (event.IsKeyPressed(GLFW_KEY_F1))
		{
			SetCheatSkipLevelRequired(true);
			return true;
		}
		// CMD GLFW_KEY_F2  : SetCheatMode(...)
		if (event.IsKeyPressed(GLFW_KEY_F2))
		{
			SetCheatMode(!GetCheatMode());
			return true;
		}
		// CMD GLFW_KEY_F3  : ReloadGameConfiguration(...)
		if (event.IsKeyPressed(GLFW_KEY_F3))
		{
			ReloadGameConfiguration();
			return true;
		}
		// CMD GLFW_KEY_F4  : ReloadCurrentLevel(...)
		if (event.IsKeyPressed(GLFW_KEY_F4))
		{
			ReloadCurrentLevel();
			return true;
		}
		// CMD GLFW_KEY_F5  : SetFreeCameraMode(...)
		if (event.IsKeyPressed(GLFW_KEY_F5))
		{
			SetFreeCameraMode(!IsFreeCameraMode());
			return true;
		}
		// CMD GLFW_KEY_F6  : SaveToCheckpoint(...)
		if (event.IsKeyPressed(GLFW_KEY_F6))
		{
			SaveIntoCheckpoint();
			return true;
		}
#endif

		return false;
	}

	bool Game::OnCharEventImpl(unsigned int c)
	{
		// give the game instance opportunity to capture the input
		if (game_instance != nullptr)
			if (game_instance->OnCharEvent(c))
				return true;
		return false;
	}

	bool Game::OnMouseButtonImpl(int button, int action, int modifier)
	{
		// try start the game
		if (game_instance == nullptr)
		{
			if (action == GLFW_PRESS)
				RequireStartGame(nullptr);
			return true;
		}
		// give opportunity to game instance to response
		if (game_instance->OnMouseButton(button, action, modifier))
			return true;
		return false;
	}

	bool Game::OnMouseMoveImpl(double x, double y)
	{
		// give the game instance opportunity to capture the input
		if (game_instance != nullptr)
			if (game_instance->OnMouseMove(x, y))
				return true;
		return false;
	}

	chaos::box2 Game::GetRequiredViewport(glm::ivec2 const & size) const
	{
		chaos::box2 viewport = chaos::box2(std::make_pair(
			glm::vec2(0.0f, 0.0f),
			chaos::RecastVector<glm::vec2>(size)
		));
		return ShrinkBoxToAspect(viewport, viewport_wanted_aspect);
	}

	void Game::Display(chaos::GPURenderer * renderer, chaos::GPUProgramProvider * uniform_provider, chaos::GPURenderParams const & render_params)
	{
		// a variable provider
		chaos::GPUProgramProviderChain main_uniform_provider(uniform_provider);
		
		// the view box
		chaos::box2 view = GetCanvasBox();
		main_uniform_provider.AddVariableValue("canvas_box", chaos::EncodeBoxToVector(view));
		// the world
		chaos::box2 world = GetWorldBox();
		main_uniform_provider.AddVariableValue("world_box", chaos::EncodeBoxToVector(world));
		// the time
		double root_time = GetRootClockTime();
		main_uniform_provider.AddVariableValue("root_time", root_time);

		// user values
		FillUniformProvider(main_uniform_provider);

		// rendering
		DoDisplay(renderer, &main_uniform_provider, render_params);
	}

	void Game::FillUniformProvider(chaos::GPUProgramProvider & main_uniform_provider)
	{
		if (game_instance != nullptr)
			game_instance->FillUniformProvider(main_uniform_provider);
		if (level_instance != nullptr)
			level_instance->FillUniformProvider(main_uniform_provider);
	}

	void Game::DoDisplay(chaos::GPURenderer * renderer, chaos::GPUProgramProvider * uniform_provider, chaos::GPURenderParams const & render_params)
	{
		// clear the main render target
		DoPreDisplay(renderer, uniform_provider, render_params);
		// display the level instance
		DoDisplayGame(renderer, uniform_provider, render_params);
		// display the hud (AFTER the level)
		DoDisplayHUD(renderer, uniform_provider, render_params);
	}


	void Game::DoPreDisplay(chaos::GPURenderer * renderer, chaos::GPUProgramProvider * uniform_provider, chaos::GPURenderParams const & render_params)
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
	}

	void Game::DoDisplayGame(chaos::GPURenderer * renderer, chaos::GPUProgramProvider * uniform_provider, chaos::GPURenderParams const & render_params)
	{		

		// shuwww   root_render_layer ??
		if (particle_manager != nullptr)
			particle_manager->Display(renderer, uniform_provider, render_params);
		if (level_instance != nullptr)
			level_instance->Display(renderer, uniform_provider, render_params);
	}

	void Game::DoDisplayHUD(chaos::GPURenderer * renderer, chaos::GPUProgramProvider * uniform_provider, chaos::GPURenderParams const & render_params)
	{	
		if (hud != nullptr)
			hud->Display(renderer, uniform_provider, render_params);
	}

	bool Game::FillAtlasGeneratorInput(chaos::BitmapAtlas::AtlasInput & input, nlohmann::json const & config, boost::filesystem::path const & config_path)
	{
		if (!FillAtlasGeneratorInputSprites(input, config, config_path))
			return false;
		if (!FillAtlasGeneratorInputFonts(input, config, config_path))
			return false;
		if (!FillAtlasGeneratorInputTiledMapManager(input, config, config_path))
			return false;
		return true;
	}

	bool Game::FillAtlasGeneratorInputTiledMapManager(chaos::BitmapAtlas::AtlasInput & input, nlohmann::json const & config, boost::filesystem::path const & config_path)
	{
		if (tiled_map_manager != nullptr)
		{
			// find or create folder
			chaos::BitmapAtlas::FolderInfoInput * folder_input = input.AddFolder("sprites", 0);
			if (folder_input == nullptr)
				return false;
			// add sprites from TiledMap
			if (!chaos::TiledMapTools::AddIntoAtlasInput(tiled_map_manager.get(), folder_input))
				return false;
		}
		return true;
	}

	bool Game::FillAtlasGeneratorInputSprites(chaos::BitmapAtlas::AtlasInput & input, nlohmann::json const & config, boost::filesystem::path const & config_path)
	{
		// get the directory where the sprites are
		std::string sprite_directory;
		if (!chaos::JSONTools::GetAttribute(config, "sprite_directory", sprite_directory))
			return true;
		// find or create folder
		chaos::BitmapAtlas::FolderInfoInput * folder_info = input.AddFolder("sprites", 0);
		if (folder_info == nullptr)
			return false;
		// Add sprites
		folder_info->AddBitmapFilesFromDirectory(sprite_directory, true);

		return true;
	}

	bool Game::FillAtlasGeneratorInputFonts(chaos::BitmapAtlas::AtlasInput & input, nlohmann::json const & config, boost::filesystem::path const & config_path)
	{
		nlohmann::json const * fonts_config = chaos::JSONTools::GetStructure(config, "fonts");
		if (fonts_config != nullptr)
		{
			// read the default font parameters
			chaos::BitmapAtlas::FontInfoInputParams font_params;

			nlohmann::json const* default_font_param_json = chaos::JSONTools::GetStructure(*fonts_config, "default_font_param");
			if (default_font_param_json != nullptr)
				LoadFromJSON(*default_font_param_json, font_params);

			// Add the fonts
			nlohmann::json const * fonts_json = chaos::JSONTools::GetStructure(*fonts_config, "fonts");
			if (fonts_json != nullptr && fonts_json->is_object())
			{
				for (nlohmann::json::const_iterator it = fonts_json->begin(); it != fonts_json->end(); ++it)
				{
					if (!it->is_string())
						continue;
					// read information
					std::string font_name = it.key();
					std::string font_path = it->get<std::string>();
					if (input.AddFont(font_path.c_str(), nullptr, true, font_name.c_str(), 0, font_params) == nullptr)
						return false;
				}			
			}		
		}

		return true;
	}

	bool Game::GenerateAtlas(nlohmann::json const & config, boost::filesystem::path const & config_path)
	{
		char const* CachedAtlasFilename = "CachedAtlas";

		// Try to load already computed data 
		if (chaos::Application::HasApplicationCommandLineFlag("-UseCachedAtlas")) // CMDLINE
		{
			chaos::BitmapAtlas::TextureArrayAtlas* tmp_texture_atlas = new chaos::BitmapAtlas::TextureArrayAtlas;
			if (tmp_texture_atlas != nullptr)
			{
				chaos::Application* application = chaos::Application::GetInstance();
				if (application != nullptr)
				{
					if (tmp_texture_atlas->LoadAtlas(application->GetUserLocalTempPath() / CachedAtlasFilename))
					{
						texture_atlas = tmp_texture_atlas;
						return true;
					}
					delete(tmp_texture_atlas);
				}
			}
		}

		// fill sub images for atlas generation
		chaos::BitmapAtlas::AtlasInput input;
		if (!FillAtlasGeneratorInput(input, config, config_path))
			return false;

		// atlas generation params
		int const DEFAULT_ATLAS_SIZE    = 1024;
		int const DEFAULT_ATLAS_PADDING = 10;

		chaos::BitmapAtlas::AtlasGeneratorParams params = chaos::BitmapAtlas::AtlasGeneratorParams(DEFAULT_ATLAS_SIZE, DEFAULT_ATLAS_SIZE, DEFAULT_ATLAS_PADDING, chaos::PixelFormatMergeParams());
		
		nlohmann::json const * atlas_json = chaos::JSONTools::GetStructure(config, "atlas");
		if (atlas_json != nullptr)
			LoadFromJSON(*atlas_json, params);

		// atlas generation params : maybe a dump
		char const * dump_atlas_dirname = nullptr;
#if _DEBUG
		dump_atlas_dirname = CachedAtlasFilename;
#else
		if (chaos::Application::HasApplicationCommandLineFlag("-DumpCachedAtlas")) // CMDLINE
			dump_atlas_dirname = CachedAtlasFilename;
#endif

		// generate the atlas
		chaos::BitmapAtlas::TextureArrayAtlasGenerator generator;
		texture_atlas = generator.ComputeResult(input, params, dump_atlas_dirname);
		if (texture_atlas == nullptr)
			return false;

		return true;
	}

	death::TiledMapLevel * Game::CreateTiledMapLevel()
	{
		return nullptr;
	}

	death::Level * Game::DoLoadLevel(chaos::FilePathParam const & path)
	{
		boost::filesystem::path const & resolved_path = path.GetResolvedPath();

		if (chaos::FileTools::IsTypedFile(resolved_path, "tmx"))
		{
			// create the tiledmap manager if necessary
			if (tiled_map_manager == nullptr)
			{
				tiled_map_manager = new chaos::TiledMap::Manager;
				if (tiled_map_manager == nullptr)
					return nullptr;
			}
			// load the resource
			chaos::TiledMap::Map * tiled_map = tiled_map_manager->LoadMap(path, false); // false : the map is not kept in the manager
			if (tiled_map == nullptr)
				return false;
			// allocate a level
			death::TiledMapLevel * result = CreateTiledMapLevel();
			if (result == nullptr)
				return false;
			// some additionnal computation
			if (!result->Initialize(tiled_map))
			{
				delete(result);
				return nullptr;
			}
			return result;
		}

		return nullptr;
	}

	boost::filesystem::directory_iterator Game::GetResourceDirectoryIteratorFromConfig(nlohmann::json const & config, char const * config_name, char const * default_path) const
	{
		// read in the config file the whole path
		std::string directory;
		if (chaos::JSONTools::GetAttribute(config, config_name, directory))
			return chaos::FileTools::GetDirectoryIterator(directory);
		// concat the argument path with 'resources'
		if (default_path != nullptr)
		{
			// get the application
			chaos::MyGLFW::SingleWindowApplication * application = chaos::Application::GetInstance();
			if (application != nullptr)
			{
				// compute resource path
				boost::filesystem::path resources_path = application->GetResourcesPath();
				boost::filesystem::path result_path = resources_path / default_path;

				return chaos::FileTools::GetDirectoryIterator(result_path);
			}
		}
		return boost::filesystem::directory_iterator();
	}

	template<typename FUNC>
	bool Game::DoGenerateTiledMapEntity(nlohmann::json const & config, char const * property_name, char const * default_value, char const * extension, FUNC func)
	{
		// iterate the files and load the tilesets
		boost::filesystem::directory_iterator end;

		for (boost::filesystem::directory_iterator it = GetResourceDirectoryIteratorFromConfig(config, property_name, default_value); it != end; ++it)
		{
			boost::filesystem::path p = it->path();

			if (chaos::FileTools::IsTypedFile(p, extension))
			{
				// create the tiledmap manager if necessary
				if (tiled_map_manager == nullptr)
				{
					tiled_map_manager = new chaos::TiledMap::Manager;
					if (tiled_map_manager == nullptr)
						return false;
				}
				if (!func(tiled_map_manager.get(), p))
					return false;
			}
		}
		return true;
	}

	bool Game::GenerateObjectTypeSets(nlohmann::json const & config, boost::filesystem::path const& config_path)
	{
		return DoGenerateTiledMapEntity(config, "objecttypesets_directory", "objecttypesets", "xml", [](chaos::TiledMap::Manager * manager, boost::filesystem::path const & path) {
			if (!manager->LoadObjectTypeSet(path))
				return false; 
			return true;
		});
	}

	bool Game::GenerateTileSets(nlohmann::json const & config, boost::filesystem::path const& config_path)
	{
		return DoGenerateTiledMapEntity(config, "tilesets_directory", "tilesets", "tsx", [](chaos::TiledMap::Manager * manager, boost::filesystem::path const & path) {
			if (!manager->LoadTileSet(path))
				return false;
			return true;
		});
	}

	bool Game::LoadLevels(nlohmann::json const & config, boost::filesystem::path const& config_path)
	{
		// iterate the files and load the levels
		boost::filesystem::directory_iterator end;
		for (boost::filesystem::directory_iterator it = GetResourceDirectoryIteratorFromConfig(config, "levels_directory", "levels"); it != end; ++it)
		{
			int level_index = chaos::StringTools::SkipAndAtoi(it->path().filename().string().c_str());

			// create the level
			death::Level * level = DoLoadLevel(it->path());
			if (level == nullptr)
				continue;
			// initialize it
			level->SetPath(it->path());
			level->level_index = level_index;
			// store it
			levels.push_back(level);
		}

		// sort the levels
		std::sort(levels.begin(), levels.end(),
			[](chaos::shared_ptr<Level> l1, chaos::shared_ptr<Level> l2)
		{
			return (l1->level_index < l2->level_index);
		});
		return true;
	}

#define DEATH_FIND_RENDERABLE_CHILD(result, funcname, constness, param_type)\
	result constness * Game::funcname(param_type param, chaos::GPURenderableLayerSystem constness * root) constness\
	{\
		if (root == nullptr)\
		{\
			root = root_render_layer.get();\
			if (root == nullptr)\
				return nullptr;\
		}\
		return auto_cast(root->FindChildRenderable(param));\
	}
#define DEATH_FIND_RENDERABLE_CHILD_ALL(result, funcname)\
	DEATH_FIND_RENDERABLE_CHILD(result, funcname, BOOST_PP_EMPTY(), char const *);\
	DEATH_FIND_RENDERABLE_CHILD(result, funcname, BOOST_PP_EMPTY(), chaos::TagType);\
	DEATH_FIND_RENDERABLE_CHILD(result, funcname, const, char const *);\
	DEATH_FIND_RENDERABLE_CHILD(result, funcname, const, chaos::TagType);\

	DEATH_FIND_RENDERABLE_CHILD_ALL(chaos::GPURenderableLayerSystem, FindRenderableLayer);
	DEATH_FIND_RENDERABLE_CHILD_ALL(chaos::ParticleLayerBase, FindParticleLayer);

#undef DEATH_FIND_RENDERABLE_CHILD_ALL
#undef DEATH_FIND_RENDERABLE_CHILD

	chaos::GPURenderableLayerSystem * Game::AddChildRenderLayer(char const * layer_name, chaos::TagType layer_tag, int render_order)
	{
		chaos::GPURenderableLayerSystem * result = new chaos::GPURenderableLayerSystem();
		if (root_render_layer == nullptr)
			return result;
		result->SetName(layer_name);
		result->SetTag(layer_tag);
		root_render_layer->AddChildRenderable(result, render_order);
		return result;
	}

	bool Game::InitializeRootRenderLayer()
	{
		root_render_layer = new chaos::GPURenderableLayerSystem();
		if (root_render_layer == nullptr)
			return false;



		// shuwww



		if (AddChildRenderLayer("GAME", death::GameHUDKeys::GAME_LAYER_ID, 1) == nullptr)
			return false;
		if (AddChildRenderLayer("PLAYER", death::GameHUDKeys::PLAYER_LAYER_ID, 2) == nullptr) // maybe the player will go in another layer
			return false;
		if (AddChildRenderLayer("HUD", death::GameHUDKeys::HUD_LAYER_ID, 3) == nullptr)
			return false;

		return true;
	}

	bool Game::InitializeParticleManager()
	{

		// shuwww on pourrait faire CreateParticleManager(...) appelable depuis l exterieur


		// create the manager
		particle_manager = new chaos::ParticleManager();
		if (particle_manager == nullptr)
			return false;
		particle_manager->SetTextureAtlas(texture_atlas.get());
		if (AddParticleLayers() < 0)
			return false;
		return true;
	}

	int Game::AddParticleLayers()
	{
		int render_order = 0;



		particle_manager->AddLayer<death::ParticleBackgroundTrait>(render_order++, death::GameHUDKeys::BACKGROUND_LAYER_ID, "background"); // shuwww est ce que ca doit etre dans le particle_manager
		return render_order;
	}

	bool Game::CreateBackgroundImage(char const * material_name, char const * texture_name)
	{
		if (material_name == nullptr)
			material_name = "background";

		// create the particle allocation if necessary
		if (background_allocations == nullptr)
		{
            background_allocations = GetGameParticleCreator().SpawnParticles(death::GameHUDKeys::BACKGROUND_LAYER_ID, nullptr, 1, true, [](chaos::ParticleAccessor<death::ParticleBackground> accessor)
            {
                for (death::ParticleBackground & particle : accessor)
                    particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            });
			if (background_allocations == nullptr)
				return false;
		}

		// create a material
		chaos::GPUResourceManager * resource_manager = chaos::MyGLFW::SingleWindowApplication::GetGPUResourceManagerInstance();
		if (resource_manager != nullptr)
		{
			// search declared material
			chaos::GPURenderMaterial * result = resource_manager->FindRenderMaterial(material_name);
			if (result != nullptr)
			{
				if (texture_name != nullptr)
				{
					// search the corresponding texture
					chaos::GPUTexture * texture = resource_manager->FindTexture(texture_name);
					if (texture == nullptr)
						return false;
					// create a child material
					chaos::GPURenderMaterial * child_material = new chaos::GPURenderMaterial();
					if (child_material == nullptr)
						return false;
					// initialize the material with parent ande texture
					child_material->SetParentMaterial(result);
					child_material->GetUniformProvider().AddVariableTexture("background", texture);
					result = child_material;
				}
				// assign the material to the background
				chaos::ParticleLayerBase * layer = particle_manager->FindLayer(death::GameHUDKeys::BACKGROUND_LAYER_ID);
				if (layer != nullptr)
					layer->SetRenderMaterial(result);
			}
		}

		return true;
	}

	bool Game::InitializeClocks(nlohmann::json const& config, boost::filesystem::path const& config_path)
	{
		chaos::Clock* application_clock = GetApplicationClock();
		if (application_clock == nullptr)
			return false;

		root_clock = application_clock->CreateChildClock("root_clock");
		if (root_clock == nullptr)
			return false;
		return true;
	}

	bool Game::CreateGamepadManager(nlohmann::json const& config, boost::filesystem::path const& config_path)
	{
		bool gamepad_enabled = true;
		chaos::JSONTools::GetAttribute(config, "gamepad_enabled", gamepad_enabled, true);
		if (gamepad_enabled)
		{
			gamepad_manager = new GamepadManager(this);
			if (gamepad_manager == nullptr)
				return false;
		}
		return true;
	}

	bool Game::InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path)
	{
		// initialize the gamepad manager
		if (!CreateGamepadManager(config, config_path))
			return false;
		// create game state_machine
		if (!CreateGameStateMachine(config, config_path))
			return false;
		// create the sound manager
		if (!InitializeSoundManager(config, config_path))
			return false;
		// initialize clocks
		if (!InitializeClocks(config, config_path))
			return false;
		// initialize the button map
		if (!InitializeGamepadButtonInfo())
			return false;
		// initialize game values
		if (!InitializeGameValues(config, config_path, false)) // false => not hot_reload
			return false;
		OnGameValuesChanged(false);
		// loading object type sets
		if (!GenerateObjectTypeSets(config, config_path))
			return false;
		// loading tilemapset
		if (!GenerateTileSets(config, config_path))
			return false;
		// the atlas
		if (!GenerateAtlas(config, config_path))  // require to have loaded level first
			return false;
		// load exisiting levels
		if (!LoadLevels(config, config_path))
			return false;
		// initialize the root render system
		if (!InitializeRootRenderLayer())
			return false;
		// initialize the particle manager
		if (!InitializeParticleManager())
			return false;
		// initialize the particle text generator manager
		if (!InitializeParticleTextGenerator(config, config_path))
			return false;
		// initialize game particles creator
		if (!InitializeGameParticleCreator())
			return false;
		// create the game background
		if (!CreateBackgroundImage(nullptr, nullptr))
			return false;

		// load the best score if any
		SerializePersistentGameData(false);
		return true;
	}

	chaos::SoundManager * Game::GetSoundManager()
	{
		chaos::MyGLFW::SingleWindowApplication * application = chaos::Application::GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetSoundManager();
	}

	chaos::Clock * Game::GetApplicationClock()
	{
		chaos::MyGLFW::SingleWindowApplication * application = chaos::Application::GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetMainClock();
	}

	chaos::Clock const * Game::GetApplicationClock() const
	{
		chaos::MyGLFW::SingleWindowApplication const * application = chaos::Application::GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetMainClock();
	}

	double Game::GetRootClockTime() const
	{
		if (root_clock == nullptr)
			return 0.0;
		return root_clock->GetClockTime();
	}

	void Game::UpdatePersistentGameData()
	{
		best_score = std::max(best_score, GetBestPlayerScore());
	}

	bool Game::LoadPersistentGameData(nlohmann::json const& game_data)
	{
		chaos::JSONTools::GetAttribute(game_data, "best_score", best_score);
		return true;
	}

	bool Game::SavePersistentGameData(nlohmann::json & game_data) const
	{
		chaos::JSONTools::SetAttribute(game_data, "best_score", best_score);
		return true;
	}

	bool Game::SerializePersistentGameData(bool save)
	{
		// get application
		chaos::Application * application = chaos::Application::GetInstance();
		if (application == nullptr)
			return false;
		// get user temp directory
		boost::filesystem::path filepath = application->GetUserLocalTempPath() / "game_data.json";

		nlohmann::json game_data;
		// save the game data
		if (save)
		{
			if (!SavePersistentGameData(game_data))
				return false;
			std::ofstream file(filepath.string().c_str());
			if (!file)
				return false;
			file << game_data.dump();
			return true;
		}
		// load the game data
		else
		{	
			if (!chaos::JSONTools::LoadJSONFile(filepath, game_data, false))
				return false;
			return LoadPersistentGameData(game_data);
		}
	}

	chaos::Sound * Game::SetInGameMusic(char const * music_name)
	{
		assert(music_name != nullptr);

#if _DEBUG
		if (chaos::Application::HasApplicationCommandLineFlag("-MuteMusic")) // CMDLINE
			return nullptr;
#endif
		
		// ensure there is a real music change
		if (game_music != nullptr && !game_music->IsPendingKill())
		{
			chaos::SoundManager * sound_manager = GetSoundManager();
			if (sound_manager == nullptr)
				return nullptr;

			chaos::SoundSource * new_source = sound_manager->FindSource(music_name);

			if (new_source == game_music->GetSource())
				return game_music.get();
		}

		// effectively change the music
		float BLEND_TIME = 2.0f;

		// destroy previous music
		bool previous_music = false;
		if (game_music != nullptr)
		{
			game_music->FadeOut(BLEND_TIME, true);
			game_music = nullptr;
			previous_music = true;
		}
		// start new music
		game_music = PlaySound(music_name, false, true, (previous_music) ? BLEND_TIME : 0.0f, SoundContext::GAME);
		return game_music.get();
	}

	chaos::Sound * Game::PlaySound(char const * name, chaos::PlaySoundDesc play_desc, chaos::TagType category_tag)
	{
		// search manager
		chaos::SoundManager * sound_manager = GetSoundManager();
		if (sound_manager == nullptr)
			return nullptr;
		// search source
		chaos::SoundSource * source = sound_manager->FindSource(name);
		if (source == nullptr)
			return nullptr;

		// Add some categories depending on the wanted sound context
		if (category_tag == SoundContext::GAME || category_tag == SoundContext::LEVEL)
		{
			if (game_instance != nullptr)
				if (game_instance->sound_category != nullptr && !game_instance->sound_category->IsPendingKill())
					play_desc.categories.push_back(game_instance->sound_category.get());

			if (category_tag == SoundContext::LEVEL)
			{
				if (level_instance != nullptr)
					if (level_instance->sound_category != nullptr && !level_instance->sound_category->IsPendingKill())
						play_desc.categories.push_back(level_instance->sound_category.get());
			}
		}

		return source->Play(play_desc);
	}

	chaos::Sound * Game::PlaySound(char const * name, bool paused, bool looping, float blend_in_time, chaos::TagType category_tag)
	{
		chaos::PlaySoundDesc play_desc;
		play_desc.paused = paused;
		play_desc.looping = looping;
		play_desc.blend_in_time = blend_in_time;

		return PlaySound(name, play_desc, category_tag);
	}

	bool Game::InitializeSoundManager(nlohmann::json const& config, boost::filesystem::path const& config_path)
	{

		return true;
	}

	bool Game::CreateGameStateMachine(nlohmann::json const& config, boost::filesystem::path const& config_path)
	{
		game_sm = DoCreateGameStateMachine();
		if (game_sm == nullptr)
			return false;
		if (!game_sm->InitializeStateMachine()) // create all internal states and transition
			return false;
		game_sm_instance = DoCreateGameStateMachineInstance(game_sm.get());
		if (game_sm_instance == nullptr)
			return false;
		return true;
	}

	chaos::SM::StateMachine * Game::DoCreateGameStateMachine()
	{
		return nullptr;
	}

	chaos::SM::StateMachineInstance * Game::DoCreateGameStateMachineInstance(chaos::SM::StateMachine * state_machine)
	{
		return new GameStateMachineInstance(this, state_machine);
	}

	bool Game::OnGamepadInput(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad) // an uncatched gamepad input incomming
	{
		assert(in_physical_gamepad != nullptr && !in_physical_gamepad->IsAllocated());

		// try start the game
		if (game_instance == nullptr)
		{
			if (in_physical_gamepad->IsAnyButtonJustPressed())
				RequireStartGame(in_physical_gamepad);
			return true;
		}
		// give opportunity to game instance to response
		if (game_instance->OnGamepadInput(in_physical_gamepad))
			return true;
		return false;
	}

	bool Game::OnPhysicalGamepadInput(chaos::MyGLFW::PhysicalGamepad * physical_gamepad)
	{
		// ignore invalid gamepad : should never happen
		if (!physical_gamepad->IsAnyAction())
			return true;
		// change the application mode
		SetInputMode(chaos::InputMode::GAMEPAD);
		// special action on gamepad input
		OnGamepadInput(physical_gamepad);

		return true;
	}

	bool Game::InitializeGamepadButtonInfo()
	{
		// the map [button ID] => [bitmap name + text generator alias]
#define DEATHGAME_ADD_BUTTONMAP(x, y) gamepad_button_map[chaos::XBoxButton::x] = std::pair<std::string, std::string>("xboxController" #y, #y)
		DEATHGAME_ADD_BUTTONMAP(BUTTON_A, ButtonA);
		DEATHGAME_ADD_BUTTONMAP(BUTTON_B, ButtonB);
		DEATHGAME_ADD_BUTTONMAP(BUTTON_X, ButtonX);
		DEATHGAME_ADD_BUTTONMAP(BUTTON_Y, ButtonY);
		DEATHGAME_ADD_BUTTONMAP(BUTTON_LEFTBUT, LeftShoulder);
		DEATHGAME_ADD_BUTTONMAP(BUTTON_RIGHTBUT, RightShoulder);
		DEATHGAME_ADD_BUTTONMAP(BUTTON_LEFTTRIGGER, LeftTrigger);
		DEATHGAME_ADD_BUTTONMAP(BUTTON_RIGHTTRIGGER, RightTrigger);
#undef LUDUMGAME_ADDTO_BUTTONMAP

		return true;
	}

	bool Game::InitializeParticleTextGenerator(nlohmann::json const & config, boost::filesystem::path const & config_path)
	{
		// get the font sub objects
		nlohmann::json const * fonts_config = chaos::JSONTools::GetStructure(config, "fonts");

		// create the generator
		particle_text_generator = new chaos::ParticleTextGenerator::Generator(*texture_atlas);
		if (particle_text_generator == nullptr)
			return false;

		// bitmaps in generator
		chaos::BitmapAtlas::FolderInfo const * folder_info = texture_atlas->GetFolderInfo("sprites");
		if (folder_info != nullptr)
		{
			// for each bitmap, that correspond to a button, register a [NAME] in the generator	
			for (auto it = gamepad_button_map.begin(); it != gamepad_button_map.end(); ++it)
			{
				std::string const & bitmap_name = it->second.first;
				chaos::BitmapAtlas::BitmapInfo const * info = folder_info->GetBitmapInfo(bitmap_name.c_str());
				if (info == nullptr)
					continue;
				std::string const & generator_alias = it->second.second;
				particle_text_generator->AddBitmap(generator_alias.c_str(), info);
			}
			// embedded sprites
			if (fonts_config != nullptr)
			{
				nlohmann::json const * font_bitmaps_json = chaos::JSONTools::GetStructure(*fonts_config, "bitmaps");
				if (font_bitmaps_json != nullptr && font_bitmaps_json->is_object())
				{
					for (nlohmann::json::const_iterator it = font_bitmaps_json->begin(); it != font_bitmaps_json->end(); ++it)
					{
						if (!it->is_string())
							continue;
						std::string bitmap_name = it.key();
						std::string bitmap_path = it->get<std::string>();
						chaos::BitmapAtlas::BitmapInfo const * info = folder_info->GetBitmapInfo(bitmap_path.c_str());
						if (info == nullptr)
							continue;
						particle_text_generator->AddBitmap(bitmap_name.c_str(), info);
					}
				}
			}
		}

		// the colors
		if (fonts_config != nullptr)
		{
			nlohmann::json const * font_colors_json = chaos::JSONTools::GetStructure(*fonts_config, "colors");
			if (font_colors_json != nullptr && font_colors_json->is_object())
			{
				for (nlohmann::json::const_iterator it = font_colors_json->begin(); it != font_colors_json->end(); ++it)
				{
					glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // initialization for if input is smaller than 4
					if (!chaos::LoadFromJSON(*it, color))
						continue;
					std::string color_name = it.key();
					particle_text_generator->AddColor(color_name.c_str(), color);
				}
			}		
		}

		return true;
	}

	bool Game::InitializeGameParticleCreator()
	{

		// shuwww

		return particle_creator.Initialize(particle_manager.get(), particle_text_generator.get(), texture_atlas.get());
	}


	bool Game::InitializeGameValues(nlohmann::json const & config, boost::filesystem::path const & config_path, bool hot_reload)
	{
		// keep a reference on the configuration path
		configuration_path = config_path;

		// capture the game instance configuration
		nlohmann::json const* gi_config = chaos::JSONTools::GetStructure(config, "game_instance");
		if (gi_config != nullptr && gi_config->is_object())
			game_instance_configuration = *gi_config;
		else
			game_instance_configuration = nlohmann::json();

		// read dedicated game values
		DEATHGAME_JSON_ATTRIBUTE(mouse_sensitivity);
		DEATHGAME_JSON_ATTRIBUTE(gamepad_sensitivity);
		DEATHGAME_JSON_ATTRIBUTE(viewport_wanted_aspect);
		return true;
	}

	void Game::OnGameValuesChanged(bool hot_reload)
	{
		if (game_instance != nullptr)
			if (game_instance->InitializeGameValues(game_instance_configuration, configuration_path, hot_reload))
				game_instance->OnGameValuesChanged(hot_reload);
	}

	void Game::OnEnterMainMenu(bool very_first)
	{
		// start the music
#if _DEBUG
		if (chaos::Application::HasApplicationCommandLineFlag("-MuteMusic")) // CMDLINE
			menu_music = nullptr;
		else
#endif
		menu_music = PlaySound("menu_music", false, true, 0.0f, SoundContext::MAINMENU);
		game_music = nullptr;
		pause_music = nullptr;
		// restore the background image
		CreateBackgroundImage(nullptr, nullptr);
		// create the main menu HUD
		CreateMainMenuHUD();
	}

	void Game::OnLeaveMainMenu()
	{
		// stop the music
		if (menu_music != nullptr)
		{
			menu_music->FadeOut(0.5f, true);
			menu_music = nullptr;
		}
	}

	void Game::OnGameOver()
	{
		// internal code
		CreateGameOverHUD();
		// give opportunity to other game classes to respond
		if (level_instance != nullptr)
			level_instance->OnGameOver();
		if (game_instance != nullptr)
			game_instance->OnGameOver();		
	}

	bool Game::OnEnterPause()
	{
		// start sound
#if _DEBUG
		if (chaos::Application::HasApplicationCommandLineFlag("-MuteMusic")) // CMDLINE
			pause_music = nullptr;
		else
#endif
		pause_music = PlaySound("pause_music", false, true, 0.0f, SoundContext::PAUSEMENU);
		// internal code
		CreatePauseMenuHUD();
		// give opportunity to other game classes to respond
		if (level_instance != nullptr)
			level_instance->OnEnterPause();
		if (game_instance != nullptr)
			game_instance->OnEnterPause();
		// pause in-game sounds
		SetInGameSoundPause(true);
		return true;
	}

	bool Game::OnLeavePause()
	{
		// destroy the pause music
		if (pause_music != nullptr)
		{
			pause_music->FadeOut(0.5f, true);
			pause_music = nullptr;
		}
		// internal code
		CreatePlayingHUD();
		// give opportunity to other game classes to respond
		if (level_instance != nullptr)
			level_instance->OnLeavePause();
		if (game_instance != nullptr)
			game_instance->OnLeavePause();
		// resume in-game sounds
		SetInGameSoundPause(false);

		return true;
	}

	void Game::SetInGameSoundPause(bool in_paused)
	{
		chaos::SoundCategory * categories[2] = { nullptr, nullptr };

		if (game_instance != nullptr)
			categories[0] = game_instance->GetSoundCategory();
		if (level_instance != nullptr)
			categories[1] = level_instance->GetSoundCategory();

		for (int i = 0; i < 2; ++i)
		{
			chaos::SoundCategory * category = categories[i];
			if (category == nullptr)
				continue;
			if (in_paused)
				category->StartBlend(chaos::BlendVolumeDesc::BlendOut(0.5f, true, false), true);
			else
			{
				category->StartBlend(chaos::BlendVolumeDesc::BlendIn(0.5f), true);
				category->Pause(false);
			}
		}
	}

	GameInstance* Game::CreateGameInstance()
	{
		// create the game instance
		GameInstance * result = DoCreateGameInstance();
		if (result == nullptr)
			return false;
		// initialie the instance
		if (!result->Initialize(this))
		{
			delete(result);
			return false;
		}
		return result;
	}

	bool Game::OnEnterGame(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad)
	{
		assert(game_instance == nullptr);
		// create the game instance
		game_instance = CreateGameInstance();
		if (game_instance == nullptr)
			return false;
		// create other resources
		game_instance->OnEnterGame();
		// game entered			
		CreatePlayingHUD();
		// start the level
		SetNextLevel(true);
		// create a first player and insert it
		game_instance->CreatePlayer(in_physical_gamepad);

		return true;
	}

	bool Game::OnLeaveGame()
	{
		// update game data that must me prevent from destruction
		UpdatePersistentGameData();
		// save the best score (and other values)
		SerializePersistentGameData(true);
		// restore main menu condition (level, music ...)
		SetCurrentLevel(nullptr);	
		// game instance stop
		game_instance->OnLeaveGame();
		// destroy the game instance 
		game_instance = nullptr;
		return true;
	}

	bool Game::CheckGameOverCondition()
	{
		// check for game over in game instance
		if (game_instance != nullptr)
			if (game_instance->DoCheckGameOverCondition())
				return true;
		return false; // no gameover
	}


	bool Game::CheckLevelCompleted()
	{
		// cheat code
#if _DEBUG
		if (GetCheatSkipLevelRequired())
			return true;
#endif
		// level knows about that
		death::LevelInstance const * level_instance = GetLevelInstance();
		if (level_instance != nullptr)
			if (level_instance->CheckLevelCompletion())
				return true;
		return false;
	}

	bool Game::CanCompleteLevel()
	{
		// shurefactor




		// game instance knows about that
		death::GameInstance const* game_instance = GetGameInstance();
		if (game_instance != nullptr)
			if (!game_instance->CanCompleteLevel())
				return false;
		// level knows about that
		death::LevelInstance const * level_instance = GetLevelInstance();
		if (level_instance != nullptr)
			if (!level_instance->CanCompleteLevel())
				return false;
		return true;
	}

	bool Game::TickGameLoop(float delta_time)
	{

		// shurefactor


		// level finished
		if (CheckLevelCompleted())
		{
			bool can_complete = 
#if _DEBUG
				GetCheatSkipLevelRequired() ||
#endif
				CanCompleteLevel();

			if (can_complete) // maybe there is a small delay for an animation or a sound
			{
				if (!SetNextLevel(looping_levels))
					RequireExitGame();
				return false; // do not call remaining code in TickGameLoop(...) specialization
			}
		}

		// shu46 : can have game over even if CheckLevelCompleted() !!!



		// game over ? (not if level is finished and wait for completion)
		if (CheckGameOverCondition())
		{
			RequireGameOver();
			return false;
		}
		// tick the level
		if (level_instance != nullptr)
			level_instance->Tick(delta_time);
		return true;
	}

	chaos::TagType Game::GetCurrentStateTag(bool strict_state, bool use_destination) const
	{
		if (game_sm_instance == nullptr)
			return -1;
		chaos::SM::StateBase const * current_state = (strict_state)?
			game_sm_instance->GetCurrentStrictState(use_destination) :
			game_sm_instance->GetCurrentState();
		if (current_state == nullptr)
			return -1;
		return current_state->GetTag();
	}

	bool Game::IsPlaying(bool strict_state, bool use_destination) const
	{
		if (GetCurrentStateTag(strict_state, use_destination) == GameStateMachineKeys::STATE_PLAYING)
			return true;
		return false;
	}

	bool Game::IsPaused(bool strict_state, bool use_destination) const
	{
		if (GetCurrentStateTag(strict_state, use_destination) == GameStateMachineKeys::STATE_PAUSE)
			return true;
		return false;
	}

	bool Game::RequireGameOver()
	{
		if (game_sm_instance->SendEvent(GameStateMachineKeys::EVENT_GAME_OVER, nullptr))
			return true;
		return false;
	}

	bool Game::RequirePauseGame()
	{
		if (!IsPaused())
			return RequireTogglePause();
		return false;
	}

	bool Game::RequireTogglePause()
	{
		if (game_sm_instance->SendEvent(GameStateMachineKeys::EVENT_TOGGLE_PAUSE, nullptr))
			return true;
		return false;
	}

	bool Game::RequireExitGame()
	{
		if (game_sm_instance->SendEvent(GameStateMachineKeys::EVENT_EXIT_GAME, nullptr))
			return true;
		return false;
	}

	bool Game::RequireStartGame(chaos::MyGLFW::PhysicalGamepad * physical_gamepad)
	{
		PhysicalGamepadWrapper game_pad_wrapper = PhysicalGamepadWrapper(physical_gamepad);

		if (game_sm_instance->SendEvent(GameStateMachineKeys::EVENT_START_GAME, &game_pad_wrapper))
			return true;
		return false;
	}

#define DEATH_IMPLEMENTHUD_FUNC(classname)\
	bool Game::Create##classname()\
	{\
		hud = DoCreate##classname();\
		if (hud == nullptr)\
			return false;\
		if (!hud->InitializeHUD())\
		{\
			hud = nullptr;\
			return false;\
		}\
		return true;\
	}\
	GameHUD * Game::DoCreate##classname()\
	{\
		return new classname(this);\
	}
	DEATH_IMPLEMENTHUD_FUNC(PauseMenuHUD);
	DEATH_IMPLEMENTHUD_FUNC(MainMenuHUD);
	DEATH_IMPLEMENTHUD_FUNC(PlayingHUD);
	DEATH_IMPLEMENTHUD_FUNC(GameOverHUD);

#undef DEATH_IMPLEMENTHUD_FUNC

	void Game::DestroyHUD()
	{
		hud = nullptr;
	}

	chaos::box2 Game::GetCanvasBox() const
	{
		glm::vec2 canvas_size;
		canvas_size.x = 1600.0f;
		canvas_size.y = canvas_size.x / viewport_wanted_aspect;

		chaos::box2 result;
		result.position = glm::vec2(0.0f, 0.0f);
		result.half_size = canvas_size * 0.5f;
		return result;
	}

	chaos::box2 Game::GetWorldBox() const
	{
		// look at the level instance
		if (level_instance != nullptr)
		{
			chaos::box2 result = level_instance->GetBoundingBox();
			if (!IsGeometryEmpty(result))
				return result;
		}
		// by default, the world will be the same size than the screen
		return GetCanvasBox();
	}


	bool Game::SetNextLevel(bool looping_levels)
	{
#if _DEBUG
		SetCheatSkipLevelRequired(false);
#endif

		// existing any level
		size_t count = levels.size();
		if (count == 0)
			return false;
		// very first level
		Level * current_level = GetLevel();
		if (current_level == nullptr)
			return SetCurrentLevel(levels[0].get());
		// search the current level
		size_t i = 0;
		for (; i < count; ++i)
			if (levels[i].get() == current_level)
				break;
		// level not found ?
		if (i == count)
			return false;
		// very last level
		if (i == count - 1)
		{
			if (looping_levels)
				return SetCurrentLevel(levels[0].get());
			return false;
		}
		// default
		return SetCurrentLevel(levels[i + 1].get());
	}
	
	chaos::AutoCastable<Level> Game::GetLevel(int level_index)
	{
		size_t count = levels.size();
		for (size_t i = 0; i < count; ++i)
			if (levels[i]->GetLevelIndex() == level_index)
				return levels[i].get();
		return nullptr;
	}

	chaos::AutoConstCastable<Level> Game::GetLevel(int level_index) const
	{
		size_t count = levels.size();
		for (size_t i = 0; i < count; ++i)
			if (levels[i]->GetLevelIndex() == level_index)
				return levels[i].get();
		return nullptr;
	}

	bool Game::SetCurrentLevel(int level_index)
	{
		Level * new_level = GetLevel(level_index); // we required a level_index, level should not be nullptr !
		if (new_level == nullptr)
			return false;
		return SetCurrentLevel(new_level);
	}

	bool Game::SetCurrentLevel(Level * new_level) // new_level can be set to nullptr, just to clear every thing
	{
		chaos::shared_ptr<Level> old_level = GetLevel();

		// destroy current level instance, so that new instance can get all resources it want
		if (level_instance != nullptr)
		{
			level_instance->OnLevelEnded();
			level_instance = nullptr;
		}

		// create the new level instance if required
		if (new_level != nullptr)
		{
			level_instance = new_level->CreateLevelInstance(this);
			if (level_instance == nullptr)
				return false;
			level_instance->OnLevelStarted();
		}

		// change the level
		OnLevelChanged(new_level, old_level.get(), level_instance.get());

		return true;
	}

	void Game::OnLevelChanged(Level * new_level, Level * old_level, LevelInstance * new_level_instance)
	{
		// free camera points an invalid 'level_instance'
		if (free_camera != nullptr)
		{
			if (new_level != nullptr && old_level != nullptr && new_level->GetLevelIndex() == old_level->GetLevelIndex()) // level unchanged : want to keep free cam position
				free_camera = DoCreateFreeCamera(free_camera.get(), new_level_instance);
			else
				free_camera = nullptr; // the camera will be created at next tick
		}
		// update the instance
		if (game_instance != nullptr)
			game_instance->OnLevelChanged(new_level, old_level, new_level_instance);
	}

	bool Game::ReloadGameConfiguration()
	{
		chaos::MyGLFW::SingleWindowApplication * application = chaos::Application::GetInstance();
		if (application == nullptr)
			return false;

		// this call may take a while. Avoid Frame rate jump
		application->FreezeNextFrameTickDuration();

		nlohmann::json config;
		if (!application->ReloadConfigurationFile(config))
			return false;

		nlohmann::json const * game_config = chaos::JSONTools::GetStructure(config, "game");
		if (game_config == nullptr)
			return false;

		bool result = InitializeGameValues(*game_config, application->GetConfigurationPath(), true); // true => hot_reload
		if (result)
			OnGameValuesChanged(true);
		return result;
	}

	bool Game::ReloadCurrentLevel()
	{
		if (level_instance == nullptr)
			return false;

		chaos::shared_ptr<Level> old_level = level_instance->GetLevel(); // keep a reference to prevent the destruction when it will be removed from the levels array
		assert(old_level != nullptr);

		// reload the level
		chaos::shared_ptr<Level> level = DoLoadLevel(old_level->GetPath()); 
		if (level == nullptr)
			return false;
		// initialize it
		level->SetPath(old_level->GetPath());
		level->level_index = old_level->GetLevelIndex();
		// the effective index of a level is not necessary its position in the array. search it
		size_t index = 0;
		while (index < levels.size() && levels[index].get() != old_level)
			++index;
		if (index == levels.size())
			return false;
		levels[index] = level; // replace the level in the array
		// restart
		SetCurrentLevel(level.get());

		return true;
	}

	GameInstance * Game::DoCreateGameInstance()
	{
		return new GameInstance();
	}

	bool Game::IsFreeCameraMode() const
	{
		return free_camera_mode;
	}

	void Game::SetFreeCameraMode(bool in_free_camera_mode)
	{
		free_camera_mode = in_free_camera_mode;
		if (!in_free_camera_mode)
			free_camera = nullptr;
	}

	Camera * Game::GetFreeCamera()
	{		
		if (free_camera == nullptr)
			free_camera = CreateFreeCamera();
		return free_camera.get();
	}
	
	Camera const * Game::GetFreeCamera() const
	{
		if (free_camera == nullptr)
			free_camera = CreateFreeCamera();
		return free_camera.get();
	}

	Camera * Game::CreateFreeCamera() const
	{
		LevelInstance const * level_instance = GetLevelInstance();
		if (level_instance != nullptr)
		{
			if (level_instance->GetCameraCount() > 0)
			{
				Camera const * first_camera = level_instance->DoGetCamera(0, false); // do not accept free camera
				if (first_camera != nullptr)
					return DoCreateFreeCamera(first_camera, (LevelInstance *)level_instance);
			}
		}
		return nullptr;
	}

	Camera * Game::DoCreateFreeCamera(Camera const * camera_to_copy, LevelInstance * level_instance) const
	{
		Camera* result = level_instance->CreateCamera();
		if (result != nullptr)
		{
			float zoom_value = 1.0f;

			FreeCameraComponent const * other_free_camera_component = camera_to_copy->FindComponentByClass<FreeCameraComponent>();
			if (other_free_camera_component != nullptr)
				zoom_value = other_free_camera_component->GetZoomValue();

			result->camera_box = camera_to_copy->camera_box;
			result->initial_camera_obox = camera_to_copy->initial_camera_obox;
			result->safe_zone = camera_to_copy->safe_zone;

			FreeCameraComponent * new_free_camera_component = new FreeCameraComponent(0); // player 0
			if (new_free_camera_component != nullptr)
			{
				new_free_camera_component->SetZoomValue(zoom_value);
				result->AddComponent(new_free_camera_component);
			}			

			result->AddComponent(new SoundListenerCameraComponent());
		}
		return result;
	}

	GameCheckpoint * Game::SaveIntoCheckpoint()
	{
		if (game_instance == nullptr)
			return nullptr;
		return game_instance->SaveIntoCheckpoint();
	}

}; // namespace death

