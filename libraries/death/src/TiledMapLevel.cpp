#include <death/TiledMapLevel.h>
#include <death/Game.h>
#include <death/LayerInstanceParticlePopulator.h>
#include <death/Player.h>
#include <death/SoundContext.h>

#include <chaos/CollisionFramework.h>
#include <chaos/TiledMapTools.h>
#include <chaos/ParticleDefault.h>
#include <chaos/GPUProgramGenerator.h>
#include <chaos/StringTools.h>

namespace death
{
	// =====================================
	// TiledMapObject implementation
	// =====================================

	TiledMapObject::TiledMapObject(class TiledMapLayerInstance* in_layer_instance) :
		layer_instance(in_layer_instance)
	{
		assert(in_layer_instance != nullptr);
	}

	// =====================================
	// TiledMapGeometricObject implementation
	// =====================================

	TiledMapGeometricObject::TiledMapGeometricObject(class TiledMapLayerInstance* in_layer_instance) :
		TiledMapObject(in_layer_instance)
	{
		assert(in_layer_instance != nullptr);
	}

	chaos::box2 TiledMapGeometricObject::GetBoundingBox(bool world_system) const
	{
		chaos::box2 result = bounding_box;
		if (world_system)
			result.position += layer_instance->GetLayerOffset();
		return result;
	}

	bool TiledMapGeometricObject::Initialize(chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		assert(in_geometric_object != nullptr);
		// get some data from the geometric object
		name = in_geometric_object->name;
		id = in_geometric_object->GetObjectID();
		geometric_object = in_geometric_object;
		// extract the bounding box
		chaos::TiledMap::GeometricObjectSurface* surface = in_geometric_object->GetObjectSurface();
		if (surface != nullptr)
			bounding_box = surface->GetBoundingBox(false);  // make our own correction for world system because the LayerInstance can change its offset

		return true;
	}

	bool TiledMapGeometricObject::IsParticleCreationEnabled() const
	{
		return true;
	}

	// =====================================
	// TiledMapTriggerObject implementation
	// =====================================

	TiledMapTriggerObject::TiledMapTriggerObject(TiledMapLayerInstance* in_layer_instance) :
		TiledMapGeometricObject(in_layer_instance)
	{
	}

	bool TiledMapTriggerObject::Initialize(chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		if (!TiledMapGeometricObject::Initialize(in_geometric_object))
			return false;
		// default values are set to the one defined by default in constructor
		enabled = in_geometric_object->FindPropertyBool("ENABLED", enabled);
		trigger_once = in_geometric_object->FindPropertyBool("TRIGGER_ONCE", trigger_once);
		trigger_id = in_geometric_object->FindPropertyInt("TRIGGER_ID", trigger_id);
		outside_box_factor = in_geometric_object->FindPropertyFloat("OUTSIDE_BOX_FACTOR", outside_box_factor);

		// enable the possibility to trigger once again the object
		enter_event_triggered = false;
		return true;
	}

	bool TiledMapTriggerObject::IsCollisionWith(chaos::box2 const& other_box, std::vector<chaos::weak_ptr<TiledMapTriggerObject>> const* triggers) const
	{
		chaos::box2 box = GetBoundingBox(true);

		if (outside_box_factor > 1.0f) // the box is bigger when we want to go outside !
		{
			// search whether we were already colliding with
			if (triggers != nullptr)
				if (std::find(triggers->begin(), triggers->end(), this) != triggers->end()) // we where already colliding the object
					box.half_size *= outside_box_factor;
		}
		return chaos::Collide(other_box, box);
	}

	bool TiledMapTriggerObject::OnPlayerCollisionEvent(float delta_time, class Player* player, chaos::CollisionType event_type)
	{
		return false; // do not do anything with collision
	}

	bool TiledMapTriggerObject::OnCameraCollisionEvent(float delta_time, chaos::box2 const& camera_box, chaos::CollisionType event_type)
	{
		return false; // do not do anything with collision
	}

	void TiledMapTriggerObject::SetEnabled(bool in_enabled)
	{
		enabled = in_enabled;
		SetModified();
	}

	void TiledMapTriggerObject::SetTriggerOnce(bool in_trigger_once)
	{
		trigger_once = in_trigger_once;
		SetModified();
	}

	TiledMapObjectCheckpoint* TiledMapTriggerObject::DoCreateCheckpoint() const
	{
		return new TiledMapTriggerObjectCheckpoint();
	}

	bool TiledMapTriggerObject::DoSaveIntoCheckpoint(TiledMapObjectCheckpoint* checkpoint) const
	{
		TiledMapTriggerObjectCheckpoint* trigger_checkpoint = auto_cast(checkpoint);
		if (trigger_checkpoint == nullptr)
			return false;

		if (!TiledMapGeometricObject::DoSaveIntoCheckpoint(checkpoint))
			return false;

		trigger_checkpoint->enabled = enabled;
		trigger_checkpoint->trigger_once = trigger_once;
		trigger_checkpoint->enter_event_triggered = enter_event_triggered;

		return true;
	}

	bool TiledMapTriggerObject::DoLoadFromCheckpoint(TiledMapObjectCheckpoint const* checkpoint)
	{
		TiledMapTriggerObjectCheckpoint const* trigger_checkpoint = auto_cast(checkpoint);
		if (trigger_checkpoint == nullptr)
			return false;

		if (!TiledMapGeometricObject::DoLoadFromCheckpoint(checkpoint))
			return false;

		enabled = trigger_checkpoint->enabled;
		trigger_once = trigger_checkpoint->trigger_once;
		enter_event_triggered = trigger_checkpoint->enter_event_triggered;

		return true;
	}

	// =============================================================
	// TiledMapCheckPointTriggerObject implementation
	// =============================================================

	bool TiledMapCheckpointTriggerObject::Initialize(chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		if (!TiledMapTriggerObject::Initialize(in_geometric_object))
			return false;
		trigger_once = true; // force a trigger once for checkpoint
		return true;
	}

	bool TiledMapCheckpointTriggerObject::OnCameraCollisionEvent(float delta_time, chaos::box2 const& camera_box, chaos::CollisionType event_type)
	{
		if (event_type != chaos::CollisionType::STARTED)
			return false;

		GameInstance* game_instance = GetLayerInstance()->GetGame()->GetGameInstance();
		if (game_instance != nullptr && game_instance->GetPlayerCount() > 0)
			game_instance->CreateRespawnCheckpoint();

		return true; // collisions handled successfully
	}

	bool TiledMapCheckpointTriggerObject::IsParticleCreationEnabled() const
	{
		return false;
	}

	// =====================================
	// TiledMapPlayerStartObject implementation
	// =====================================

	bool TiledMapPlayerStartObject::Initialize(chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		if (!TiledMapGeometricObject::Initialize(in_geometric_object))
			return false;
		// search the bitmap name for the player
		std::string const * in_bitmap_name = in_geometric_object->FindPropertyString("BITMAP_NAME");
		if (in_bitmap_name == nullptr)
			return false;
		bitmap_name = *in_bitmap_name;
		return true;
	}

	// =================================================
	// TiledMapNotificationTriggerObject
	// =================================================

	bool TiledMapNotificationTriggerObject::IsParticleCreationEnabled() const
	{
		return false;
	}

	bool TiledMapNotificationTriggerObject::Initialize(chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		if (!TiledMapTriggerObject::Initialize(in_geometric_object))
			return false;

		notification_string = in_geometric_object->FindPropertyString("NOTIFICATION", "");
		notification_lifetime = in_geometric_object->FindPropertyFloat("LIFETIME", notification_lifetime);
		stop_when_collision_over = in_geometric_object->FindPropertyBool("STOP_WHEN_COLLISION_OVER", stop_when_collision_over);
		player_collision = in_geometric_object->FindPropertyBool("PLAYER_COLLISION", player_collision);

		return true;
	}

	bool TiledMapNotificationTriggerObject::OnCameraCollisionEvent(float delta_time, chaos::box2 const& camera_box, chaos::CollisionType event_type)
	{
		if (player_collision)
			return false;
		return OnTriggerCollision(delta_time, event_type);
	}

	bool TiledMapNotificationTriggerObject::OnPlayerCollisionEvent(float delta_time, class Player* player, chaos::CollisionType event_type)
	{
		if (!player_collision)
			return false;
		return OnTriggerCollision(delta_time, event_type);
	}

	bool TiledMapNotificationTriggerObject::OnTriggerCollision(float delta_time, chaos::CollisionType event_type)
	{
		// early exit
		if (event_type != chaos::CollisionType::STARTED && event_type != chaos::CollisionType::FINISHED) // ignore AGAIN event
			return false;
		if (event_type == chaos::CollisionType::FINISHED && !stop_when_collision_over) // ignore FINISHED if you do not want to kill the notification
			return false;
		// get some variables 
		Game* game = layer_instance->GetGame();
		if (game == nullptr)
			return false;
		death::GameHUD* hud = game->GetCurrentHUD();
		if (hud == nullptr)
			return false;
		GameHUDNotificationComponent* notification_component = hud->FindComponentByClass<GameHUDNotificationComponent>();
		if (notification_component == nullptr)
			return false;
		// show notification
		if (event_type == chaos::CollisionType::STARTED)
			notification_component->ShowNotification(notification_string.c_str(), notification_lifetime);
		// hide notification
		else if (event_type == chaos::CollisionType::FINISHED) // XXX : 'stop_when_collision_over' has already be checked
			notification_component->HideNotification();
		return true;
	}

	// =====================================
	// TiledMapSoundTriggerObject implementation
	// =====================================

	bool TiledMapSoundTriggerObject::Initialize(chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		if (!TiledMapTriggerObject::Initialize(in_geometric_object))
			return false;

		sound_name = in_geometric_object->FindPropertyString("SOUND_NAME", "");
		min_distance_ratio = in_geometric_object->FindPropertyFloat("MIN_DISTANCE_RATIO", min_distance_ratio);
		min_distance_ratio = std::clamp(min_distance_ratio, 0.0f, 1.0f);

		pause_timer_when_too_far = in_geometric_object->FindPropertyFloat("PAUSE_TIMER_WHEN_TOO_FAR", pause_timer_when_too_far);
		is_3D_sound = in_geometric_object->FindPropertyBool("3D_SOUND", is_3D_sound);
		looping = in_geometric_object->FindPropertyBool("LOOPING", looping);
		stop_when_collision_over = in_geometric_object->FindPropertyBool("STOP_WHEN_COLLISION_OVER", stop_when_collision_over);

		return true;
	}

	chaos::Sound* TiledMapSoundTriggerObject::CreateSound() const
	{
		// early exit
		if (sound_name.length() == 0)
			return nullptr;

		// create the sound
		chaos::Sound* result = nullptr;

		Game* game = layer_instance->GetGame();
		if (game != nullptr)
		{
			chaos::box2 box = GetBoundingBox(true);
			glm::vec2   position = box.position;

			chaos::PlaySoundDesc play_desc;
			play_desc.paused = false;
			play_desc.looping = looping;
			play_desc.blend_in_time = 0.0f;
			play_desc.pause_timer_when_too_far = pause_timer_when_too_far;

			play_desc.SetPosition(glm::vec3(position, 0.0f), is_3D_sound);
			play_desc.max_distance = glm::length(box.half_size);
			play_desc.min_distance = play_desc.max_distance * min_distance_ratio;

			result = game->PlaySound(sound_name.c_str(), play_desc, SoundContext::LEVEL);
		}

		return result;
	}

	bool TiledMapSoundTriggerObject::OnCameraCollisionEvent(float delta_time, chaos::box2 const& camera_box, chaos::CollisionType event_type)
	{
		if (event_type == chaos::CollisionType::STARTED)
		{
			chaos::Sound* new_sound = CreateSound();
			if (stop_when_collision_over)
				sound = new_sound;
			return true;
		}
		if (event_type == chaos::CollisionType::FINISHED)
		{
			if (stop_when_collision_over && sound != nullptr)
			{
				sound->Stop();
				sound = nullptr;
			}
			return true;
		}
		return false;
	}

	bool TiledMapSoundTriggerObject::IsParticleCreationEnabled() const
	{
		return false;
	}

	// =============================================================
	// TiledMapFinishingTriggerObject implementation
	// =============================================================

	bool TiledMapFinishingTriggerObject::IsParticleCreationEnabled() const
	{
		return false;
	}

	bool TiledMapFinishingTriggerObject::OnPlayerCollisionEvent(float delta_time, death::Player* player, chaos::CollisionType event_type)
	{
		if (event_type != chaos::CollisionType::STARTED)
			return false;

		Game* game = layer_instance->GetGame();
		if (game != nullptr)
		{
			death::LevelInstance* level_instance = game->GetLevelInstance();
			if (level_instance != nullptr)
				level_instance->SetLevelCompletionFlag();
		}
		return true;
	}

	// =====================================
	// TiledMapCameraObject implementation
	// =====================================

	bool TiledMapCameraObject::Initialize(chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		if (!TiledMapGeometricObject::Initialize(in_geometric_object))
			return false;
		return true;
	}







	// =====================================
	// BoxScissoringWithRepetitionResult : an utility object to compute instances in 2D of a box that collide a given scissor
	// =====================================

	class BoxScissoringWithRepetitionResult
	{
	public:

		/** constructor */
		BoxScissoringWithRepetitionResult(chaos::box2 const& in_target_box, chaos::box2 const& in_scissor_box, bool in_wrap_x, bool in_wrap_y);

		/** offset of a given instance */
		glm::vec2 GetInstanceOffset(glm::ivec2 const& index) const;

	public:

		/** the 'index' of the first instance to render/collide ... (included) */
		glm::ivec2 start_instance = glm::ivec2(0, 0);
		/** the 'index' of the last instance to render/collide ... (excluded) */
		glm::ivec2 last_instance = glm::ivec2(0, 0);
		/** copy of the construction parameters */
		chaos::box2 target_box;;
		/** copy of the construction parameters */
		chaos::box2 scissor_box;
		/** copy of the construction parameters */
		bool wrap_x = false;
		/** copy of the construction parameters */
		bool wrap_y = false;
	};

	// =====================================
	// BoxScissoringWithRepetitionResult implementation
	// =====================================

	BoxScissoringWithRepetitionResult::BoxScissoringWithRepetitionResult(chaos::box2 const& in_target_box, chaos::box2 const& in_scissor_box, bool in_wrap_x, bool in_wrap_y)
	{
		// copy the parameters
		target_box = in_target_box;
		scissor_box = in_scissor_box;
		wrap_x = in_wrap_x;
		wrap_y = in_wrap_y;

		// First,
		//   Considere that there is always wrap_x = wrap_y = true
		//   Search along X and Y, the instance index that must collides with the scissor box
		// Then,
		//   whenever there are no wrap, we can skip the whole rendering if the instancee (0, 0) is not in the visible range

		glm::vec2 target_bottomleft = GetBoxExtremums(in_target_box).first;
		glm::vec2 scissor_bottomleft = GetBoxExtremums(in_scissor_box).first;

		glm::vec2 target_size = 2.0f * in_target_box.half_size;
		glm::vec2 scissor_size = 2.0f * in_scissor_box.half_size;

		// clamp out all non intersecting box when no WRAP
		if (!wrap_x)
		{
			if ((target_bottomleft.x + target_size.x < scissor_bottomleft.x) || (target_bottomleft.x > scissor_bottomleft.x + scissor_size.x))
			{
				start_instance = last_instance = glm::ivec2(0, 0); // nothing to render at all
				return;
			}
		}

		if (!wrap_y)
		{
			if ((target_bottomleft.y + target_size.y < scissor_bottomleft.y) || (target_bottomleft.y > scissor_bottomleft.y + scissor_size.y))
			{
				start_instance = last_instance = glm::ivec2(0, 0); // nothing to render at all
				return;
			}
		}

		// number of time to decal the layer box, to be directly left of the scissor box
		glm::ivec2 offset_count = glm::ivec2(
			(int)std::ceil((scissor_bottomleft.x - target_bottomleft.x - target_size.x) / target_size.x),
			(int)std::ceil((scissor_bottomleft.y - target_bottomleft.y - target_size.y) / target_size.y)
		);

		// the bottomleft corner of the decaled box
		glm::vec2  virtual_target_bottomleft = target_bottomleft + chaos::RecastVector<glm::vec2>(offset_count) * target_size;
		// competition of the number of repetition
		glm::vec2  tmp = ((scissor_bottomleft - virtual_target_bottomleft + scissor_size) / target_size);

		glm::ivec2 repetition_count = glm::ivec2(
			(int)std::ceil(tmp.x),
			(int)std::ceil(tmp.y)
		);

		// unwrap case, then only visible instance is 0 (we previsously tested for visibility)
		if (!wrap_x)
		{
			offset_count.x = 0;
			repetition_count.x = 1;
		}
		// unwrap case, then only visible instance is 0 (we previsously tested for visibility)
		if (!wrap_y)
		{
			offset_count.y = 0;
			repetition_count.y = 1;
		}

		// finalize the initialization
		start_instance = offset_count;
		last_instance = offset_count + repetition_count;
	}

	glm::vec2 BoxScissoringWithRepetitionResult::GetInstanceOffset(glm::ivec2 const& index) const
	{
		return 2.0f * target_box.half_size * chaos::RecastVector<glm::vec2>(index);
	}

	// =====================================
	// TiledMapLevel implementation
	// =====================================

	TiledMapLevel::TiledMapLevel()
	{

	}

	bool TiledMapLevel::Initialize(chaos::TiledMap::Map* in_tiled_map)
	{
		assert(in_tiled_map != nullptr);
		assert(tiled_map == nullptr);
		// keep a copy of the tiled object
		tiled_map = in_tiled_map;
		// get level title (if specified)
		std::string const* in_level_title = in_tiled_map->FindPropertyString("LEVEL_TITLE");
		if (in_level_title != nullptr)
			level_title = *in_level_title;
		// get the level time	
		level_timeout = in_tiled_map->FindPropertyFloat("LEVEL_TIMEOUT", -1);
		return true;
	}

	LevelInstance* TiledMapLevel::DoCreateLevelInstance(Game* in_game)
	{
		return new TiledMapLevelInstance;
	}

	chaos::BitmapAtlas::TextureArrayAtlas const* TiledMapLevel::GetTextureAtlas(TiledMapLayerInstance* layer_instance) const
	{
		Game const* game = layer_instance->GetGame();
		if (game == nullptr)
			return nullptr;
		return game->GetTextureAtlas();
	}

	chaos::BitmapAtlas::FolderInfo const* TiledMapLevel::GetFolderInfo(TiledMapLayerInstance* layer_instance) const
	{
		// get the atlas
		chaos::BitmapAtlas::TextureArrayAtlas const* texture_atlas = GetTextureAtlas(layer_instance);
		if (texture_atlas == nullptr)
			return nullptr;
		// get the folder containing the sprites
		return texture_atlas->GetFolderInfo("sprites");
	}

	chaos::ParticleLayerBase* TiledMapLevel::DoCreateParticleLayer(TiledMapLayerInstance* layer_instance)
	{
		return new chaos::ParticleLayer<TiledMapParticleTrait>();
	}
	
	GeometricObjectFactory TiledMapLevel::DoGetGeometricObjectFactory(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::TypedObject * in_typed_object)
	{
		// player start 
		if (chaos::TiledMapTools::IsPlayerStartObject(in_typed_object))
			return DEATH_MAKE_GEOMETRICOBJECT_FACTORY(return DoCreatePlayerStartObject(in_layer_instance, in_geometric_object););
		// camera 
		if (chaos::TiledMapTools::IsCameraObject(in_typed_object))
			return DEATH_MAKE_GEOMETRICOBJECT_FACTORY(return DoCreateCameraObject(in_layer_instance, in_geometric_object););
		// other kind of objects
		if (chaos::TiledMapTools::IsFinishTrigger(in_typed_object))
			return DEATH_MAKE_GEOMETRICOBJECT_FACTORY(return DoCreateFinishingTriggerObject(in_layer_instance, in_geometric_object););
		if (chaos::TiledMapTools::IsCheckpointTrigger(in_typed_object))
			return DEATH_MAKE_GEOMETRICOBJECT_FACTORY(return DoCreateCheckpointTriggerObject(in_layer_instance, in_geometric_object););
		if (chaos::TiledMapTools::IsNotificationTrigger(in_typed_object))
			return DEATH_MAKE_GEOMETRICOBJECT_FACTORY(return DoCreateNotificationTriggerObject(in_layer_instance, in_geometric_object););
		if (chaos::TiledMapTools::IsSoundTrigger(in_typed_object))
			return DEATH_MAKE_GEOMETRICOBJECT_FACTORY(return DoCreateSoundTriggerObject(in_layer_instance, in_geometric_object););
		return nullptr;
	}

	GeometricObjectFactory TiledMapLevel::GetGeometricObjectFactory(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::TypedObject * in_typed_object)
	{
		// get a very first factory that just
		GeometricObjectFactory factory = DoGetGeometricObjectFactory(in_layer_instance, in_typed_object);
		if (!factory)
			return nullptr;
		// create another factory that wraps the previous (and add Initialize(...) call)
		GeometricObjectFactory result = [factory](chaos::TiledMap::GeometricObject* in_geometric_object)
		{
			TiledMapGeometricObject * result = factory(in_geometric_object);
			if (result != nullptr && !result->Initialize(in_geometric_object))
			{
				delete result;
				result = nullptr;
			}
			return result;
		};
		return result;
	}
	
	TiledMapCameraObject* TiledMapLevel::DoCreateCameraObject(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		return new TiledMapCameraObject(in_layer_instance);
	}



	TiledMapFinishingTriggerObject* TiledMapLevel::DoCreateFinishingTriggerObject(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		return new TiledMapFinishingTriggerObject(in_layer_instance);
	}
	TiledMapCheckpointTriggerObject* TiledMapLevel::DoCreateCheckpointTriggerObject(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		return new TiledMapCheckpointTriggerObject(in_layer_instance);
	}
	TiledMapNotificationTriggerObject* TiledMapLevel::DoCreateNotificationTriggerObject(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		return new TiledMapNotificationTriggerObject(in_layer_instance);
	}
	TiledMapSoundTriggerObject* TiledMapLevel::DoCreateSoundTriggerObject(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		return new TiledMapSoundTriggerObject(in_layer_instance);
	}

	TiledMapPlayerStartObject* TiledMapLevel::DoCreatePlayerStartObject(TiledMapLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject* in_geometric_object)
	{
		return new TiledMapPlayerStartObject(in_layer_instance);
	}

	TiledMapLayerInstance* TiledMapLevel::DoCreateLayerInstance(TiledMapLevelInstance* in_level_instance, chaos::TiledMap::LayerBase* in_layer)
	{
		return new TiledMapLayerInstance(in_level_instance, in_layer);
	}

	TiledMapLayerInstance* TiledMapLevel::CreateLayerInstance(TiledMapLevelInstance* in_level_instance, chaos::TiledMap::LayerBase* in_layer)
	{
		TiledMapLayerInstance* result = DoCreateLayerInstance(in_level_instance, in_layer);
		if (result == nullptr)
			return nullptr;
		if (!result->Initialize())
		{
			delete result;
			return nullptr;
		}
		return result;
	}

	chaos::GPUProgram* TiledMapLevel::GenDefaultRenderProgram()
	{
		char const* vertex_shader_source = R"VERTEXSHADERCODE(
			in vec2 position;
			in vec3 texcoord;
			in vec4 color;

			out vec2 vs_position;
			out vec3 vs_texcoord;
			out vec4 vs_color;

			uniform vec2 offset;
			uniform mat4 camera_transform;
			uniform vec4 camera_box;

			void main()
			{
				vec2 pos = position + offset;

				vs_position = pos;
				vs_texcoord = texcoord;
				vs_color = color;

				vec4 transformed_pos = camera_transform * vec4(pos.x, pos.y, 0.0, 1.0);

				gl_Position.xy = transformed_pos.xy / camera_box.zw;
				gl_Position.z = 0.0;
				gl_Position.w = 1.0;
			}									
		)VERTEXSHADERCODE";

		char const* pixel_shader_source = R"PIXELSHADERCODE(
			out vec4 output_color; // "output_color" replaces "gl_FragColor" because glBindFragDataLocation(...) has been called

			in vec2 vs_position;
			in vec3 vs_texcoord;
			in vec4 vs_color;

			uniform sampler2DArray material;

			void main()
			{
				vec4 color = (vs_texcoord.x < 0.0 || vs_texcoord.y < 0.0)? 
					vec4(1.0, 1.0, 1.0, 1.0) : 
					texture(material, vs_texcoord);
				output_color.xyz = color.xyz * vs_color.xyz;
				output_color.a   = vs_color.a * color.a;
			};
		)PIXELSHADERCODE";

		chaos::GPUProgramGenerator program_generator;
		program_generator.AddShaderSource(GL_VERTEX_SHADER, vertex_shader_source);
		program_generator.AddShaderSource(GL_FRAGMENT_SHADER, pixel_shader_source);
		return program_generator.GenProgramObject();
	}

	chaos::GPURenderMaterial* TiledMapLevel::GetDefaultRenderMaterial()
	{
		chaos::shared_ptr<chaos::GPUProgram> program = GenDefaultRenderProgram(); // store a temporary object for lifetime management
		return chaos::GPURenderMaterial::GenRenderMaterialObject(program.get());
	}

	// =====================================
	// TiledMapLayerInstance implementation
	// =====================================

	TiledMapLayerInstance::TiledMapLayerInstance(TiledMapLevelInstance* in_level_instance, chaos::TiledMap::LayerBase* in_layer) :
		level_instance(in_level_instance),
		layer(in_layer)
	{
		assert(in_level_instance != nullptr);
		assert(in_layer != nullptr);
	}

	chaos::AutoCastable<Game> TiledMapLayerInstance::GetGame()
	{
		return level_instance->GetGame();
	}

	chaos::AutoConstCastable<Game> TiledMapLayerInstance::GetGame() const
	{
		return level_instance->GetGame();
	}

	chaos::AutoCastable<Level> TiledMapLayerInstance::GetLevel()
	{
		if (level_instance == nullptr)
			return nullptr;
		return level_instance->GetLevel();
	}

	chaos::AutoConstCastable<death::Level> TiledMapLayerInstance::GetLevel() const
	{
		if (level_instance == nullptr)
			return (Level*)nullptr;
		return level_instance->GetLevel();
	}

	chaos::AutoCastable<LevelInstance> TiledMapLayerInstance::GetLevelInstance()
	{
		return level_instance;
	}

	chaos::AutoConstCastable<LevelInstance> TiledMapLayerInstance::GetLevelInstance() const
	{
		return level_instance;
	}

	chaos::box2 TiledMapLayerInstance::GetBoundingBox(bool world_system) const
	{
		chaos::box2 result;
		if (!infinite_bounding_box)
		{
			result = bounding_box; // apply our own offset that can have changed during game lifetime
			if (world_system)
				result.position += offset;
		}
		else
			result = result;
		return result;
	}

	chaos::GPURenderMaterial* TiledMapLayerInstance::FindOrCreateRenderMaterial(char const* material_name)
	{
		if (material_name != nullptr && material_name[0] != 0) // unamed material ?
		{
			// get the resource manager
			chaos::GPUResourceManager* resource_manager = chaos::MyGLFW::SingleWindowApplication::GetGPUResourceManagerInstance();
			if (resource_manager != nullptr)
			{
				// search declared material
				chaos::GPURenderMaterial* result = resource_manager->FindRenderMaterial(material_name);
				if (result != nullptr)
					return result;
			}
		}
		// default material else where
		return level_instance->GetDefaultRenderMaterial();
	}

	bool TiledMapLayerInstance::Initialize()
	{
		// get the properties of interrest
		id = layer->id;

		float const* ratio = layer->FindPropertyFloat("DISPLACEMENT_RATIO");
		if (ratio != nullptr)
		{
			displacement_ratio = glm::vec2(*ratio, *ratio);
		}
		else
		{
			displacement_ratio.x = layer->FindPropertyFloat("DISPLACEMENT_RATIO_X", 1.0f);
			displacement_ratio.y = layer->FindPropertyFloat("DISPLACEMENT_RATIO_Y", 1.0f);
		}
		wrap_x = layer->FindPropertyBool("WRAP_X", false);
		wrap_y = layer->FindPropertyBool("WRAP_Y", false);
		material_name = layer->FindPropertyString("MATERIAL", "");

		triggers_enabled = layer->FindPropertyBool("TRIGGERS_ENABLED", false);
		player_collision_enabled = layer->FindPropertyBool("PLAYER_COLLISIONS_ENABLED", false);
		camera_collision_enabled = layer->FindPropertyBool("CAMERA_COLLISIONS_ENABLED", false);
		tile_collisions_enabled = layer->FindPropertyBool("TILE_COLLISIONS_ENABLED", false);

		infinite_bounding_box = layer->FindPropertyBool("INFINITE_BOUNDING_BOX", false);

		// copy the offset / name
		offset = layer->offset;
		name = layer->name;

		// reset the bounding box
		bounding_box = chaos::box2();
		// special initialization
		chaos::TiledMap::ImageLayer* image_layer = layer->GetImageLayer();
		if (image_layer != nullptr)
		{
			if (!InitializeImageLayer(image_layer))
				return false;
			return FinalizeParticles(nullptr);
		}

		chaos::TiledMap::ObjectLayer* object_layer = layer->GetObjectLayer();
		if (object_layer != nullptr)
		{
			if (!InitializeObjectLayer(object_layer))
				return false;
			return FinalizeParticles(nullptr);
		}

		chaos::TiledMap::TileLayer* tile_layer = layer->GetTileLayer();
		if (tile_layer != nullptr)
		{
			if (!InitializeTileLayer(tile_layer))
				return false;
			return FinalizeParticles(nullptr);
		}

		return false;
	}

	bool TiledMapLayerInstance::InitializeImageLayer(chaos::TiledMap::ImageLayer* image_layer)
	{
		return true;
	}

	void TiledMapLayerInstance::CreateGeometricObjectParticles(chaos::TiledMap::GeometricObject* geometric_object, TiledMapGeometricObject* object, TiledMapLayerInstanceParticlePopulator* particle_populator)
	{

		// shuzzz



		chaos::TiledMap::Map* tiled_map = level_instance->GetTiledMap();

		// create additionnal particles (TEXT)
		chaos::TiledMap::GeometricObjectText* text = geometric_object->GetObjectText();
		if (text != nullptr)
		{
			// create particle layer if necessary
			if (particle_layer == nullptr)
				if (CreateParticleLayer() == nullptr) // the generate layer is of  TiledMapParticleTrait => this works fine for Text (except a useless GID per particle)
					return;

			Game* game = GetGame();

			chaos::ParticleTextGenerator::GeneratorResult result;
			chaos::ParticleTextGenerator::GeneratorParams params;

			params.line_height = (float)text->pixelsize;
			params.hotpoint = chaos::Hotpoint::TOP;
			params.position = text->position;
			params.default_color = text->color;

			std::string const* font_name = text->FindPropertyString("FONT_NAME");
			if (font_name != nullptr)
				params.font_info_name = *font_name;

			// create particles
			game->GetTextGenerator()->Generate(text->text.c_str(), result, params);
			chaos::ParticleTextGenerator::CreateTextAllocation(particle_layer.get(), result);
		}

		// create additionnal particles (TILES)
		chaos::TiledMap::GeometricObjectTile* tile = geometric_object->GetObjectTile();
		if (tile != nullptr)
		{
			int gid = tile->gid;



			// shuludum ?

			// shu FindProperty



			// search the tile information 
			chaos::TiledMap::TileInfo tile_info = tiled_map->FindTileInfo(gid);
			if (tile_info.tiledata == nullptr)
				return;
			// aspect ratio is to be maintained ??

			bool keep_aspect_ratio = tile->FindPropertyBool("KEEP_ASPECT_RATIO", true);
#if 0

			bool keep_aspect_ratio = true;
			bool* prop = tile->FindPropertyBool("KEEP_ASPECT_RATIO");
			if (prop != nullptr)
				keep_aspect_ratio = *prop;
			else
			{
				prop = tile_info.tiledata->FindPropertyBool("KEEP_ASPECT_RATIO");
				if (prop != nullptr)
					keep_aspect_ratio = *prop;
			}
#endif











			// create a simple particle
			chaos::box2 particle_box = tile->GetBoundingBox(true);
			if (object != nullptr)
			{



				chaos::TiledMap::GeometricObjectSurface const* surface_object = geometric_object->GetObjectSurface();
				if (surface_object != nullptr)
				{
					keep_aspect_ratio = false; // ??? shuludum




					particle_box = surface_object->GetBoundingBox(false); // shuxxx : the TILE is generated on the same layer then the surface. does it get the layer_offset ????
				}
			}
			particle_populator->AddParticle(tile_info.tiledata->atlas_key.c_str(), particle_box, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), gid, tile->horizontal_flip, tile->vertical_flip, keep_aspect_ratio);
		}
	}

	bool TiledMapLayerInstance::ShouldCreateParticleForObject(chaos::TiledMap::PropertyOwner * property_owner, TiledMapGeometricObject* object) const
	{
#if _DEBUG
		if (chaos::Application::HasApplicationCommandLineFlag("-TiledGeometricObject::ForceParticleCreation")) // CMDLINE
			return true;

#endif			
		return property_owner->FindPropertyBool("PARTICLE_CREATION", (object != nullptr) ? object->IsParticleCreationEnabled() : true);
	}

	GeometricObjectFactory TiledMapLayerInstance::GetGeometricObjectFactory(chaos::TiledMap::TypedObject * in_typed_object)
	{
		TiledMapLevel* level = GetLevel();

		// get a factory for the object (new + Initialize(...) ...)
		GeometricObjectFactory factory = level->GetGeometricObjectFactory(this, in_typed_object);
		if (!factory)
			return nullptr;
		// create a 'final' factory that use previous one + insert the result object in correct list
		GeometricObjectFactory result = [this, factory](chaos::TiledMap::GeometricObject* geometric_object)
		{
			TiledMapGeometricObject* result = factory(geometric_object);
			if (result != nullptr)
			{
				if (TiledMapTriggerObject* trigger = auto_cast(result))
					trigger_objects.push_back(trigger);
				else if (TiledMapPlayerStartObject* player_start = auto_cast(result))
					player_start_objects.push_back(player_start);
				else if (TiledMapCameraObject* camera = auto_cast(result))
					camera_objects.push_back(camera);
				else
					geometric_objects.push_back(result);
			}
			return result;
		};
		return result;
	}

	TiledMapLayerInstanceParticlePopulator* TiledMapLayerInstance::CreateParticlePopulator()
	{
		return new TiledMapLayerInstanceParticlePopulator();
	}

	bool TiledMapLayerInstance::InitializeObjectLayer(chaos::TiledMap::ObjectLayer* object_layer)
	{
		// search the bounding box (explicit or not)
		chaos::box2 box;
		chaos::box2 explicit_bounding_box;

		// the particle generator
		chaos::shared_ptr<TiledMapLayerInstanceParticlePopulator> particle_populator = CreateParticlePopulator();
		if (!particle_populator->Initialize(this))
			return false;

		// iterate over all objects
		size_t count = object_layer->geometric_objects.size();
		for (size_t i = 0; i < count; ++i)
		{
			chaos::TiledMap::GeometricObject* geometric_object = object_layer->geometric_objects[i].get();
			if (geometric_object == nullptr)
				continue;

			// explicit world bounding box
			if (!level_instance->has_explicit_bounding_box && chaos::TiledMapTools::IsWorldBoundingBox(geometric_object))
			{
				chaos::TiledMap::GeometricObjectSurface const* object_surface = geometric_object->GetObjectSurface();
				if (object_surface != nullptr)
				{
					level_instance->explicit_bounding_box = object_surface->GetBoundingBox(true); // in world coordinate	
					level_instance->has_explicit_bounding_box = true;
				}
			}
			// explicit layer bounding box
			if (IsGeometryEmpty(explicit_bounding_box) && chaos::TiledMapTools::IsLayerBoundingBox(geometric_object))
			{
				chaos::TiledMap::GeometricObjectSurface const* object_surface = geometric_object->GetObjectSurface();
				if (object_surface != nullptr)
					explicit_bounding_box = object_surface->GetBoundingBox(false); // in layer coordinates	
			}

			// get factory + create the object
			TiledMapGeometricObject* object = nullptr;

			GeometricObjectFactory factory = GetGeometricObjectFactory(geometric_object);
			if (factory)
				object = factory(geometric_object);

			// create tile if no object created 
			// (XXX : no way yet to know whether this a normal situation because user does not want to create object or whether an error happened)
			if (object == nullptr || ShouldCreateParticleForObject(geometric_object, object))
				CreateGeometricObjectParticles(geometric_object, object, particle_populator.get());
		}

		// final flush
		particle_populator->FlushParticles();
		// update the bounding box
		if (!IsGeometryEmpty(explicit_bounding_box))
			bounding_box = explicit_bounding_box;
		else
			bounding_box = box | particle_populator->GetBoundingBox();

		return true;
	}

	bool TiledMapLayerInstance::FinalizeParticles(chaos::ParticleAllocationBase* allocation)
	{
		// no layer, nothing to do !
		if (particle_layer == nullptr)
			return true;
		// no level ?
		TiledMapLevel* level = GetLevel();
		if (level == nullptr)
			return true;
		// initialize each allocations
		if (allocation == nullptr)
		{
			size_t allocation_count = particle_layer->GetAllocationCount();
			for (size_t i = 0; i < allocation_count; ++i)
				if (!level->FinalizeLayerParticles(this, particle_layer->GetAllocation(i)))
					return false;
		}
		// or just specified one
		else
		{
			if (!level->FinalizeLayerParticles(this, allocation))
				return false;
		}
		return true;
	}
	bool TiledMapLayerInstance::InitializeParticleLayer(chaos::ParticleLayerBase* in_particle_layer)
	{
		// the name
		std::string const* renderable_name = layer->FindPropertyString("RENDERABLE_NAME");
		if (renderable_name != nullptr)
			in_particle_layer->SetName(renderable_name->c_str());
		else
			in_particle_layer->SetName(layer->name.c_str());
		// the tag
		std::string const* renderable_tag = layer->FindPropertyString("RENDERABLE_TAG");
		if (renderable_tag != nullptr)
			in_particle_layer->SetTag(chaos::MakeStaticTagType(renderable_tag->c_str()));
		else
		{
			int const* layer_tag = layer->FindPropertyInt("RENDERABLE_TAG");
			if (layer_tag != nullptr)
				in_particle_layer->SetTag((chaos::TagType) * layer_tag);
		}
		// enabled renderpasses
		std::string const* enabled_renderpasses = layer->FindPropertyString("ENABLED_RENDERPASSES");
		if (enabled_renderpasses != nullptr)
			in_particle_layer->AddEnabledRenderPasses(enabled_renderpasses->c_str());
		// disabled renderpasses
		std::string const* disabled_renderpasses = layer->FindPropertyString("DISABLED_RENDERPASSES");
		if (disabled_renderpasses != nullptr)
			in_particle_layer->AddDisabledRenderPasses(disabled_renderpasses->c_str());
		return true;
	}

	chaos::ParticleAllocationBase* TiledMapLayerInstance::SpawnParticles(size_t count)
	{
		// create particle layer if necessary
		if (CreateParticleLayer() == nullptr)
			return nullptr;
		// create the allocation
		return particle_layer->SpawnParticles(count);
	}

	chaos::ParticleLayerBase* TiledMapLayerInstance::CreateParticleLayer()
	{
		if (particle_layer == nullptr)
		{
			// find render material
			chaos::GPURenderMaterial* render_material = FindOrCreateRenderMaterial(material_name.c_str());
			if (render_material == nullptr)
				return nullptr;
			// create a particle layer
			TiledMapLevel* level = GetLevel();
			particle_layer = level->DoCreateParticleLayer(this);
			if (particle_layer == nullptr)
				return false;
			// add name and tag to the particle_layer
			InitializeParticleLayer(particle_layer.get());
			// set the material
			particle_layer->SetRenderMaterial(render_material);
			// set the atlas			
			particle_layer->SetTextureAtlas(GetGame()->GetTextureAtlas());
		}
		return particle_layer.get();
	}





	bool TiledMapLayerInstance::InitializeTileLayer(chaos::TiledMap::TileLayer* tile_layer)
	{
		

		// shuzzz

		TiledMapLevel* level = GetLevel();

		// early exit for empty tile_layer
		size_t count = tile_layer->tile_indices.size();
		if (count == 0)
			return false;

		chaos::shared_ptr<TiledMapLayerInstanceParticlePopulator> particle_populator = CreateParticlePopulator();
		if (!particle_populator->Initialize(this))
			return false;

		// populate the layer
		chaos::TiledMap::Map* tiled_map = level_instance->GetTiledMap();

		for (size_t i = 0; i < count; ++i)
		{
			int gid = tile_layer->tile_indices[i];
			if (gid == 0)
				continue;
			// search the tile information 
			chaos::TiledMap::TileInfo tile_info = tiled_map->FindTileInfo(gid);
			if (tile_info.tiledata == nullptr)
				continue;

			// prepare data for the tile/object
			glm::ivec2  tile_coord = tile_layer->GetTileCoordinate(i);
			chaos::box2 particle_box = tile_layer->GetTileBoundingBox(tile_coord, tile_info.tiledata->image_size, false);

			bool horizontal_flip = false;
			bool vertical_flip = false;

			// try to create a geometric object from the tile
			GeometricObjectFactory factory = GetGeometricObjectFactory(tile_info.tiledata);
			if (factory)
			{
				chaos::shared_ptr<chaos::TiledMap::GeometricObjectTile> tile_object = new chaos::TiledMap::PropertyOwnerOverride<chaos::TiledMap::GeometricObjectTile>(nullptr, tile_info.tiledata);
				if (tile_object != nullptr)
				{
					// compute an ID base on 'tile_coord' 
					// TiledMap gives positive ID
					// we want a negative ID to avoid conflicts
					// for a 32 bits integer
					// 15 bits for X
					// 15 bits for Y
					// 1  unused
					// 1  bit for sign

					int int_bit_count = 8 * sizeof(int);
					int per_component_bit_count = (int_bit_count - 1) / 2;
					int mask = ~(-1 << per_component_bit_count);
					int idx = tile_coord.x & mask;
					int idy = tile_coord.y & mask;
					int object_id = -1 * (idx | (idy << per_component_bit_count));

					tile_object->id  = object_id;
					tile_object->gid = gid;
					tile_object->type = tile_info.tiledata->type;
					tile_object->size = particle_box.half_size * 2.0f;
					tile_object->position.x = particle_box.position.x - particle_box.half_size.x;
					tile_object->position.y = particle_box.position.y - particle_box.half_size.y;

					// XXX : for player start : but this is not a great idea to process by exception
					//       We are writing int the fake object properties, not in the 'tile_info.tiledata'
					//       That means that if 'tile_info.tiledata' already has a BITMAP_NAME property, this does not
					//       interfere with that (a just in case value)
					tile_object->InsertProperty("BITMAP_NAME", tile_info.tiledata->atlas_key.c_str()); 

					TiledMapGeometricObject* object = factory(tile_object.get());
					if (object != nullptr)
					{
						if (!ShouldCreateParticleForObject(tile_object.get(), object))
							continue;
					}
				}
			}

			// create a simple particle
			particle_populator->AddParticle(tile_info.tiledata->atlas_key.c_str(), particle_box, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), gid, horizontal_flip, vertical_flip);
		}

		// final flush
		particle_populator->FlushParticles();
		// update the bounding box
		bounding_box = particle_populator->GetBoundingBox();

		return true;
	}

	void TiledMapLayerInstance::FindPlayerTileCollisions(Player* player, std::vector<TileParticleCollisionInfo>& result)
	{
		// layer accept collision with player
		if (!ArePlayerCollisionEnabled() || !AreTileCollisionsEnabled())
			return;
		// early exit
		PlayerPawn* player_pawn = player->GetPawn();
		if (player_pawn == nullptr)
			return;
		chaos::box2 pawn_box = player_pawn->GetBox();
		if (IsGeometryEmpty(pawn_box))
			return;

		// search all collisions with particles that does not belongs to the player
		FindTileCollisions(result, pawn_box, [player_pawn](chaos::ParticleAllocationBase const* allocation)
		{
			return (player_pawn->GetAllocation() != allocation); // player does not collide itself
		});
	}

	void TiledMapLayerInstance::HandlePlayerAndCameraCollision(float delta_time)
	{
		// get the game
		Game* game = GetGame();
		if (game == nullptr)
			return;

		// early exit
		if (!AreTriggersEnabled())
			return;

		// check player collisions
		if (ArePlayerCollisionEnabled())
		{
			// compute the collisions for all players
			size_t player_count = game->GetPlayerCount();
			for (size_t i = 0; i < player_count; ++i)
			{
				Player* player = game->GetPlayer(i);
				if (player == nullptr)
					continue;
				// collision with surface triggers
				if (!HandlePlayerCollisionWithSurfaceTriggers(delta_time, player))
					continue;
			}
		}

		// compute collision with camera
		if (AreCameraCollisionEnabled())
		{
			Camera const* camera = game->GetCamera(0);
			if (camera != nullptr)
			{
				chaos::box2 camera_box = camera->GetCameraBox(true);
				if (!IsGeometryEmpty(camera_box))
					HandleCameraCollisionWithSurfaceTriggers(delta_time, camera_box);
			}
		}
	}

	TiledMapPlayerAndTriggerCollisionRecord* TiledMapLayerInstance::FindPlayerCollisionRecord(Player* player)
	{
		size_t count = collision_records.size();
		for (size_t i = count; i > 0; --i)
		{
			size_t index = i - 1;
			if (collision_records[index].player == nullptr)
				collision_records.erase(collision_records.begin() + index);
			else if (collision_records[index].player == player)
				return &collision_records[index];
		}
		return nullptr;
	}


	bool TiledMapLayerInstance::HandleCameraCollisionWithSurfaceTriggers(float delta_time, chaos::box2 const& camera_box)
	{
		// the new colliding triggers
		std::vector<chaos::weak_ptr<TiledMapTriggerObject>> new_triggers;

		// search all colliding triggers
		size_t triggers_count = trigger_objects.size();
		for (size_t i = 0; i < triggers_count; ++i)
		{
			TiledMapTriggerObject* trigger = trigger_objects[i].get();
			if (trigger == nullptr || !trigger->IsEnabled())
				continue;
			// detect collision
			if (trigger->IsCollisionWith(camera_box, &camera_collision_records))
				new_triggers.push_back(trigger);
		}

		// triggers collisions 
		size_t new_triggers_count = new_triggers.size();
		for (size_t i = 0; i < new_triggers_count; ++i)
		{
			TiledMapTriggerObject* trigger = new_triggers[i].get();

			// search in previous frame data	
			chaos::CollisionType collision_type = chaos::CollisionType::STARTED;
			if (std::find(camera_collision_records.begin(), camera_collision_records.end(), trigger) != camera_collision_records.end())
				collision_type = chaos::CollisionType::AGAIN;

			// trigger once : do not trigger anymore entering events
			if (collision_type == chaos::CollisionType::STARTED && trigger->IsTriggerOnce() && trigger->enter_event_triggered)
				continue;
			// trigger event
			if (trigger->OnCameraCollisionEvent(delta_time, camera_box, collision_type))
			{
				if (collision_type == chaos::CollisionType::STARTED && trigger->IsTriggerOnce())
				{
					trigger->enter_event_triggered = true;
					trigger->SetModified();
				}
			}
		}

		// triggers end of collisions
		size_t previous_count = camera_collision_records.size();
		for (size_t i = 0; i < previous_count; ++i)
		{
			if (std::find(new_triggers.begin(), new_triggers.end(), camera_collision_records[i]) == new_triggers.end()) // no more colliding
				camera_collision_records[i]->OnCameraCollisionEvent(delta_time, camera_box, chaos::CollisionType::FINISHED);
		}

		// store the new triggers
		camera_collision_records = std::move(new_triggers);

		return true;

	}

	bool TiledMapLayerInstance::HandlePlayerCollisionWithSurfaceTriggers(float delta_time, class Player* player)
	{
		// the new colliding triggers
		std::vector<chaos::weak_ptr<TiledMapTriggerObject>> new_triggers;
		// the previous colliding triggers
		TiledMapPlayerAndTriggerCollisionRecord* previous_collisions = FindPlayerCollisionRecord(player);

		// search all colliding triggers
		PlayerPawn* player_pawn = player->GetPawn();
		if (player_pawn != nullptr)
		{
			chaos::box2 pawn_box = player_pawn->GetBox();
			
			size_t triggers_count = trigger_objects.size();
			for (size_t i = 0; i < triggers_count; ++i)
			{
				TiledMapTriggerObject* trigger = trigger_objects[i].get();
				if (trigger == nullptr || !trigger->IsEnabled())
					continue;
				// detect collision
				if (trigger->IsCollisionWith(pawn_box, (previous_collisions != nullptr) ? &previous_collisions->triggers : nullptr))
					new_triggers.push_back(trigger);
			}
		}

		// triggers collisions 
		size_t new_triggers_count = new_triggers.size();
		for (size_t i = 0; i < new_triggers_count; ++i)
		{
			TiledMapTriggerObject* trigger = new_triggers[i].get();

			// search in previous frame data
			chaos::CollisionType collision_type = chaos::CollisionType::STARTED;
			if (previous_collisions != nullptr)
				if (std::find(previous_collisions->triggers.begin(), previous_collisions->triggers.end(), trigger) != previous_collisions->triggers.end())
					collision_type = chaos::CollisionType::AGAIN;
			// trigger once : do not trigger anymore entering events
			if (collision_type == chaos::CollisionType::STARTED && trigger->IsTriggerOnce() && trigger->enter_event_triggered)
				continue;
			// trigger event
			if (trigger->OnPlayerCollisionEvent(delta_time, player, collision_type))
			{
				if (collision_type == chaos::CollisionType::STARTED && trigger->IsTriggerOnce())
				{
					trigger->enter_event_triggered = true;
					trigger->SetModified();
				}
			}
		}

		// triggers end of collisions
		if (previous_collisions != nullptr)
		{
			size_t previous_count = previous_collisions->triggers.size();
			for (size_t i = 0; i < previous_count; ++i)
			{
				if (std::find(new_triggers.begin(), new_triggers.end(), previous_collisions->triggers[i]) == new_triggers.end()) // no more colliding
					previous_collisions->triggers[i]->OnPlayerCollisionEvent(delta_time, player, chaos::CollisionType::FINISHED);
			}
		}

		// store the record
		if (previous_collisions != nullptr)
			previous_collisions->triggers = std::move(new_triggers);
		else
		{
			TiledMapPlayerAndTriggerCollisionRecord new_record;
			new_record.player = player;
			new_record.triggers = std::move(new_triggers);
			collision_records.push_back(std::move(new_record));
		}
		return true; // continue other collisions 
	}

	void TiledMapLayerInstance::FindTileCollisions(std::vector<TileParticleCollisionInfo> & result, chaos::box2 const& bounding_box, std::function<bool(chaos::ParticleAllocationBase const*)> filter_allocation_func)
	{
		// no particle layer, no collisions
		if (layer == nullptr || particle_layer == nullptr)
			return;
		// search map
		chaos::TiledMap::Map const* map = layer->GetMap();
		if (map == nullptr)
			return;

		// a cache for tiledata : hope this is less costly than a per particle search
		std::map<int, chaos::TiledMap::TileInfo> gid_to_tileinfo;

		// iterate over all allocations
		size_t allocation_count = particle_layer->GetAllocationCount();
		for (size_t i = 0; i < allocation_count; ++i)
		{
			// check allocation
			chaos::ParticleAllocationBase* particle_allocation = particle_layer->GetAllocation(i);
			if (particle_allocation == nullptr)
				continue;
			// filter out collision
			if (filter_allocation_func && !filter_allocation_func(particle_allocation))
				continue;
			// search collisions
			chaos::ParticleAccessor<TiledMapParticle> accessor = particle_allocation->GetParticleAccessor();
			for (TiledMapParticle& particle : accessor)
			{
				if (chaos::Collide(bounding_box, particle.bounding_box))
				{
					chaos::TiledMap::TileInfo ti;
					
					// search whether the tile_info is already in cache
					auto it = gid_to_tileinfo.find(particle.gid);
					if (it != gid_to_tileinfo.end())
					{
						ti = it->second;
					}
					// search the tile_info in the map
					else
					{
						ti = map->FindTileInfo(particle.gid);
						if (ti.tiledata == nullptr || ti.tileset == nullptr) // unknown info : ignore this collision
							continue;
						gid_to_tileinfo[particle.gid] = ti;
					}
					result.emplace_back(particle, ti);
				}
			}
		}
	}

	bool TiledMapLayerInstance::DoTick(float delta_time)
	{
		// player starts
		size_t player_start_count = player_start_objects.size();
		for (size_t i = 0; i < player_start_count; ++i)
			player_start_objects[i]->Tick(delta_time);
		// camera
		size_t camera_count = camera_objects.size();
		for (size_t i = 0; i < camera_count; ++i)
			camera_objects[i]->Tick(delta_time);
		// trigger
		size_t trigger_count = trigger_objects.size();
		for (size_t i = 0; i < trigger_count; ++i)
			trigger_objects[i]->Tick(delta_time);
		// geometric objects
		size_t geometric_count = geometric_objects.size();
		for (size_t i = 0; i < geometric_count; ++i)
			geometric_objects[i]->Tick(delta_time);
		// tick the particles
		if (particle_layer != nullptr)
			particle_layer->Tick(delta_time);
		return true;
	}

	int TiledMapLayerInstance::DoDisplay(chaos::GPURenderer* renderer, chaos::GPUProgramProviderBase const* uniform_provider, chaos::GPURenderParams const& render_params)
	{
		// early exit
		int result = 0;
		if (particle_layer == nullptr)
			return result;

		// camera is expressed in world, so is for layer
		chaos::obox2 camera_obox = GetLevelInstance()->GetCameraOBox(0);
		chaos::obox2 initial_camera_obox = GetLevelInstance()->GetInitialCameraOBox(0);


		// XXX : we want some layers to appear further or more near the camera
		//       the displacement_ratio represent how fast this layer is moving relatively to other layers.
		//       The reference layer is the layer where the 'effective' camera (and so the PlayerStart is)
		//         => when player goes outside the screen, the camera is updated so that it is still watching the player
		//         => that why we consider the PlayerStart's layer as the reference
		//       to simulate other layer's speed, we just create at rendering time 'virtual cameras' (here this is 'final_camera_box')
		//       We only multiply 'true camera' distance from its initial position by a ratio value

		// apply the displacement to the camera

		glm::vec2 final_ratio = glm::vec2(1.0f, 1.0f);

		TiledMapLayerInstance const* reference_layer = level_instance->reference_layer.get();
		if (reference_layer != nullptr)
		{
			if (reference_layer->displacement_ratio.x != 0.0f)
				final_ratio.x = displacement_ratio.x / reference_layer->displacement_ratio.x;
			if (reference_layer->displacement_ratio.y != 0.0f)
				final_ratio.y = displacement_ratio.y / reference_layer->displacement_ratio.y;

		}

		// shulayer
		chaos::box2 layer_box = GetBoundingBox(true);

#if 0
		chaos::box2 reference_box = reference_layer->GetBoundingBox(true);
		if (!IsGeometryEmpty(layer_box) && !IsGeometryEmpty(reference_box))
			final_ratio = layer_box.half_size / reference_box.half_size;
#endif

		chaos::obox2 final_camera_obox;
		final_camera_obox.position = initial_camera_obox.position + (camera_obox.position - initial_camera_obox.position) * final_ratio;
		final_camera_obox.half_size = initial_camera_obox.half_size + (camera_obox.half_size - initial_camera_obox.half_size) * final_ratio;

		// compute repetitions
		BoxScissoringWithRepetitionResult scissor_result = BoxScissoringWithRepetitionResult(layer_box, chaos::GetBoundingBox(final_camera_obox), wrap_x, wrap_y);

		// new provider for camera override (will be fullfill only if necessary)
		chaos::GPUProgramProviderChain main_uniform_provider(uniform_provider);
		main_uniform_provider.AddVariableValue("camera_transform", CameraTransform::GetCameraTransform(final_camera_obox));

		chaos::box2 final_camera_box;
		final_camera_box.position = final_camera_obox.position;
		final_camera_box.half_size = final_camera_obox.half_size;
		main_uniform_provider.AddVariableValue("camera_box", chaos::EncodeBoxToVector(final_camera_box));


		// HACK : due to bad LAYER_BOUNDING_BOX computation, the layer containing PLAYER_START may be clamped and layer hidden
		glm::ivec2 start_instance = scissor_result.start_instance;
		glm::ivec2 last_instance = scissor_result.last_instance;
		if (this == reference_layer || IsGeometryEmpty(layer_box))
		{
			start_instance = glm::ivec2(0, 0);
			last_instance = glm::ivec2(1, 1); // always see fully the layer without clamp => repetition not working
		}

		// draw instances 
		int draw_instance_count = 0;
		for (int x = start_instance.x; x < last_instance.x; ++x)
		{
			for (int y = start_instance.y; y < last_instance.y; ++y)
			{
				// new Provider to apply the offset for this 'instance'
				chaos::GPUProgramProviderChain instance_uniform_provider(&main_uniform_provider);
				glm::vec2 instance_offset = scissor_result.GetInstanceOffset(glm::ivec2(x, y));
				instance_uniform_provider.AddVariableValue("offset", instance_offset + offset);
				// draw call
				result += particle_layer->Display(renderer, &instance_uniform_provider, render_params);
			}
		}
		return result;
	}

#define DEATH_FIND_OBJECT(result_type, func_name, member_vector, constness)\
		result_type constness * TiledMapLayerInstance::func_name(chaos::NamedObjectRequest request) constness\
		{\
			return NamedObject::FindNamedObject(member_vector, request);\
		}
	DEATH_FIND_OBJECT(TiledMapGeometricObject, FindGeometricObject, geometric_objects, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapGeometricObject, FindGeometricObject, geometric_objects, const);
	DEATH_FIND_OBJECT(TiledMapTriggerObject, FindTriggerObject, trigger_objects, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapTriggerObject, FindTriggerObject, trigger_objects, const);
	DEATH_FIND_OBJECT(TiledMapPlayerStartObject, FindPlayerStartObject, player_start_objects, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapPlayerStartObject, FindPlayerStartObject, player_start_objects, const);
	DEATH_FIND_OBJECT(TiledMapCameraObject, FindCameraObject, camera_objects, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapCameraObject, FindCameraObject, camera_objects, const);

#undef DEATH_FIND_OBJECT


	size_t TiledMapLayerInstance::GetTriggerCount() const
	{
		return trigger_objects.size();
	}

	TiledMapTriggerObject* TiledMapLayerInstance::GetTrigger(size_t index)
	{
		if (index >= trigger_objects.size())
			return nullptr;
		return trigger_objects[index].get();
	}

	TiledMapTriggerObject const* TiledMapLayerInstance::GetTrigger(size_t index) const
	{
		if (index >= trigger_objects.size())
			return nullptr;
		return trigger_objects[index].get();
	}


	TiledMapLayerCheckpoint* TiledMapLayerInstance::DoCreateCheckpoint() const
	{
		return new TiledMapLayerCheckpoint();
	}

	template<typename ELEMENT_VECTOR, typename CHECKPOINT_VECTOR>
	bool TiledMapLayerInstance::DoSaveIntoCheckpointHelper(ELEMENT_VECTOR const& elements, CHECKPOINT_VECTOR& checkpoints) const
	{
		size_t count = elements.size();
		for (size_t i = 0; i < count; ++i)
		{
			// only modified object
			auto const* obj = elements[i].get();
			if (obj == nullptr || !obj->IsModified())
				continue;
			// save the checkpoint
			TiledMapObjectCheckpoint* checkpoint = obj->SaveIntoCheckpoint();
			if (checkpoint == nullptr)
				continue;
			checkpoints[obj->GetObjectID()] = checkpoint;
		}
		return true;
	}

	bool TiledMapLayerInstance::DoSaveIntoCheckpoint(TiledMapLayerCheckpoint* checkpoint) const
	{
		DoSaveIntoCheckpointHelper(trigger_objects, checkpoint->trigger_checkpoints);
		DoSaveIntoCheckpointHelper(geometric_objects, checkpoint->object_checkpoints);
		return true;
	}

	template<typename ELEMENT_VECTOR, typename CHECKPOINT_VECTOR>
	bool TiledMapLayerInstance::DoLoadFromCheckpointHelper(ELEMENT_VECTOR& elements, CHECKPOINT_VECTOR const& checkpoints)
	{
		// shuzzz
		size_t count = elements.size();
		for (size_t i = 0; i < count; ++i)
		{
			// only modified object
			auto* obj = elements[i].get();
			if (obj == nullptr)
				continue;	
			// get checkpoint
			TiledMapObjectCheckpoint* obj_checkpoint = nullptr;
			auto it = checkpoints.find(obj->GetObjectID());
			if (it != checkpoints.end())
				obj_checkpoint = it->second.get();

			// -checkpoint found    => use it
			// -no checkpoint point =>
			//    -> object is currently modified, restore initial settings
			if (obj_checkpoint != nullptr)
				obj->LoadFromCheckpoint(obj_checkpoint);
			else if (obj->IsModified())
				obj->Initialize(obj->GetGeometricObject());
		}
		return true;
	}

	bool TiledMapLayerInstance::DoLoadFromCheckpoint(TiledMapLayerCheckpoint const* checkpoint)
	{
		DoLoadFromCheckpointHelper(trigger_objects, checkpoint->trigger_checkpoints);
		DoLoadFromCheckpointHelper(geometric_objects, checkpoint->object_checkpoints);
		return true;
	}

	void TiledMapLayerInstance::OnLevelEnded()
	{
		// players
		size_t player_start_count = player_start_objects.size();
		for (size_t i = 0; i < player_start_count; ++i)
			player_start_objects[i]->OnLevelEnded();
		// camera
		size_t camera_count = camera_objects.size();
		for (size_t i = 0; i < camera_count; ++i)
			camera_objects[i]->OnLevelEnded();
		// triggers
		size_t trigger_count = trigger_objects.size();
		for (size_t i = 0; i < trigger_count; ++i)
			trigger_objects[i]->OnLevelEnded();
		//  geometric object
		size_t object_count = geometric_objects.size();
		for (size_t i = 0; i < object_count; ++i)
			geometric_objects[i]->OnLevelEnded();
	}

	void TiledMapLayerInstance::OnLevelStarted()
	{
		// players
		size_t player_start_count = player_start_objects.size();
		for (size_t i = 0; i < player_start_count; ++i)
			player_start_objects[i]->OnLevelStarted();
		// camera
		size_t camera_count = camera_objects.size();
		for (size_t i = 0; i < camera_count; ++i)
			camera_objects[i]->OnLevelStarted();
		// triggers
		size_t trigger_count = trigger_objects.size();
		for (size_t i = 0; i < trigger_count; ++i)
			trigger_objects[i]->OnLevelStarted();
		//  geometric object
		size_t object_count = geometric_objects.size();
		for (size_t i = 0; i < object_count; ++i)
			geometric_objects[i]->OnLevelStarted();
	}

	// =====================================
	// TiledMapLevelInstance implementation
	// =====================================

	chaos::TiledMap::Map* TiledMapLevelInstance::GetTiledMap()
	{
		TiledMapLevel* level = GetLevel();
		if (level == nullptr)
			return nullptr;
		return level->GetTiledMap();
	}

	chaos::TiledMap::Map const* TiledMapLevelInstance::GetTiledMap() const
	{
		TiledMapLevel const* level = GetLevel();
		if (level == nullptr)
			return nullptr;
		return level->GetTiledMap();
	}

	void TiledMapLevelInstance::HandlePlayerAndCameraCollision(float delta_time)
	{
		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
			layer_instances[i]->HandlePlayerAndCameraCollision(delta_time);
	}

	void TiledMapLevelInstance::FindPlayerTileCollisions(Player* player, std::vector<TileParticleCollisionInfo>& result)
	{
		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
			layer_instances[i]->FindPlayerTileCollisions(player, result);
	}

	bool TiledMapLevelInstance::DoTick(float delta_time)
	{
		death::LevelInstance::DoTick(delta_time);

		// tick the particle manager
		if (particle_manager != nullptr)
			particle_manager->Tick(delta_time);
		// tick all layer instances
		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
			layer_instances[i]->Tick(delta_time);
		// compute the collisions with the player
		HandlePlayerAndCameraCollision(delta_time);

		return true;
	}

	int TiledMapLevelInstance::DoDisplay(chaos::GPURenderer* renderer, chaos::GPUProgramProviderBase const* uniform_provider, chaos::GPURenderParams const& render_params)
	{
		int result = 0;

		// display particle manager
		if (particle_manager != nullptr)
			result += particle_manager->Display(renderer, uniform_provider, render_params);
		// draw the layer instances0
		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
			result += layer_instances[i]->Display(renderer, uniform_provider, render_params);

		return result;
	}

	bool TiledMapLevelInstance::Initialize(Game* in_game, death::Level* in_level)
	{
		if (!death::LevelInstance::Initialize(in_game, in_level))
			return false;
		// create a the layers instances
		if (!CreateLayerInstances(in_game))
			return false;
		// create a particle manager
		if (!CreateParticleManager(in_game))
			return false;

		return true;
	}

	bool TiledMapLevelInstance::CreateParticleManager(Game* in_game)
	{
		particle_manager = new chaos::ParticleManager;
		if (particle_manager == nullptr)
			return false;
		particle_manager->SetTextureAtlas(in_game->GetTextureAtlas()); // take the atlas
		return true;
	}

	bool TiledMapLevelInstance::CreateLayerInstances(Game* in_game)
	{
		TiledMapLevel* level = GetLevel();

		chaos::TiledMap::Map* tiled_map = GetTiledMap();

		// handle layers ordered by Z-Order
		size_t count = tiled_map->GetLayerCount();
		for (size_t i = 0; i < count; ++i)
		{
			// get the chaos::LayerBase object per Z-order
			chaos::TiledMap::LayerBase* layer = tiled_map->FindLayerByZOrder(i);
			if (layer == nullptr)
				continue;
			// create and store the layer_instance
			TiledMapLayerInstance* layer_instance = level->CreateLayerInstance(this, layer);
			if (layer_instance != nullptr)
				layer_instances.push_back(layer_instance);
		}
		return true;
	}

	chaos::box2 TiledMapLevelInstance::GetBoundingBox() const
	{
		// explicit bounding box
		if (has_explicit_bounding_box)
			return explicit_bounding_box;
		// depend on layers
		chaos::box2 result;
		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
			result = result | layer_instances[i]->GetBoundingBox(true); // expressed in world system the bounding boxes
		return result;
	}

	chaos::GPURenderMaterial* TiledMapLevelInstance::GetDefaultRenderMaterial()
	{
		if (default_material == nullptr)
		{
			TiledMapLevel* level = GetLevel();
			default_material = level->GetDefaultRenderMaterial(); // create material and cache
		}
		return default_material.get();
	}

#define DEATH_FIND_OBJECT(result_type, func_name, constness)\
		result_type constness * TiledMapLevelInstance::func_name(chaos::NamedObjectRequest request) constness\
		{\
			size_t count = layer_instances.size();\
			for (size_t i = 0; i < count; ++i)\
			{\
				result_type constness * result = layer_instances[i]->func_name(request);\
				if (result != nullptr)\
					return result;\
			}\
			return nullptr;\
		}
	DEATH_FIND_OBJECT(TiledMapGeometricObject, FindGeometricObject, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapGeometricObject, FindGeometricObject, const);
	DEATH_FIND_OBJECT(TiledMapTriggerObject, FindTriggerObject, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapTriggerObject, FindTriggerObject, const);
	DEATH_FIND_OBJECT(TiledMapPlayerStartObject, FindPlayerStartObject, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapPlayerStartObject, FindPlayerStartObject, const);
	DEATH_FIND_OBJECT(TiledMapCameraObject, FindCameraObject, BOOST_PP_EMPTY());
	DEATH_FIND_OBJECT(TiledMapCameraObject, FindCameraObject, const);

#undef DEATH_FIND_OBJECT

	TiledMapLayerInstance* TiledMapLevelInstance::FindLayerInstance(chaos::NamedObjectRequest request)
	{
		return NamedObject::FindNamedObject(layer_instances, request);
	}
	TiledMapLayerInstance const* TiledMapLevelInstance::FindLayerInstance(chaos::NamedObjectRequest request) const
	{
		return NamedObject::FindNamedObject(layer_instances, request);
	}

	void TiledMapLevelInstance::CreateCameras()
	{
		TiledMapLevel* level = GetLevel();

		// search CAMERA NAME
		std::string const* camera_name = level->GetTiledMap()->FindPropertyString("CAMERA_NAME");

		// search the CAMERA
		TiledMapCameraObject* camera_object = nullptr;
		if (camera_name != nullptr)
			camera_object = FindCameraObject(*camera_name); // first, if a name is given, use it
		if (camera_object == nullptr)
		{
			camera_object = FindCameraObject(chaos::NamedObjectRequest()); // try to find the very first one otherwise
			if (camera_object == nullptr)
				return;
		}

		// get the aspect ratio
		float aspect_ratio = 16.0f / 9.0f;

		Game const* game = GetGame();
		if (game != nullptr)
			aspect_ratio = game->GetViewportWantedAspect();

		// compute the surface
		chaos::box2 camera_box = chaos::AlterBoxToAspect(camera_object->GetBoundingBox(true), aspect_ratio, true);

		// create the real camera
		Camera* camera = new Camera(this);
		if (camera == nullptr)
			return;
		cameras.push_back(camera);

		// initialize the camera
		camera->SetCameraBox(camera_box);
	}


	TiledMapPlayerStartObject* TiledMapLevelInstance::GetPlayerStartForPawn(Player* player)
	{
		TiledMapLevel* level = GetLevel();
		if (level == nullptr)
			return nullptr;

		// search PLAYER START NAME
		std::string const* player_start_name = level->GetTiledMap()->FindPropertyString("PLAYER_START_NAME");

		// search the PLAYER START
		TiledMapPlayerStartObject* result = nullptr;
		if (player_start_name != nullptr)
			result = FindPlayerStartObject(player_start_name->c_str()); // first, if a name is given, use it
		if (result == nullptr)
			result = FindPlayerStartObject(chaos::NamedObjectRequest()); // try to find the very first one otherwise
		return result;
	}

	PlayerPawn* TiledMapLevelInstance::CreatePlayerPawn(Player* player, TiledMapPlayerStartObject* player_start, TiledMapLayerInstance* layer_instance)	
	{
		// create a particle populator
		chaos::shared_ptr<TiledMapLayerInstanceParticlePopulator> particle_populator = layer_instance->CreateParticlePopulator();
		if (!particle_populator->Initialize(layer_instance))
			return nullptr;

		// create the particle
		int player_gid = 0;
		particle_populator->AddParticle(player_start->bitmap_name.c_str(), player_start->GetBoundingBox(true), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), player_gid);
		particle_populator->FlushParticles();

		// get the allocation and finalize the layer
		chaos::ParticleAllocationBase* player_allocation = particle_populator->GetParticleAllocation();
		if (player_allocation == nullptr)
			return nullptr;
		// shuxxx : first time FinalizeParticles(...) was called, there was no effect because the PlayerStartLayer has no particle. 
		//          call it twice as a fast fix
		layer_instance->FinalizeParticles(player_allocation);

		// create a pawn 
		PlayerPawn* result = DoCreatePlayerPawn(player);
		if (result == nullptr)
			return result;

		result->SetAllocation(player_allocation);
		return result;
	}

	PlayerPawn * TiledMapLevelInstance::CreatePlayerPawn(Player* player)
	{
		// find the player start to use for the player
		TiledMapPlayerStartObject* player_start = GetPlayerStartForPawn(player);
		if (player_start == nullptr)
			return nullptr;

		// initialize some data
		TiledMapLayerInstance* layer_instance = player_start->GetLayerInstance();
		if (layer_instance == nullptr)
			return nullptr;

		// XXX : while camera, is restricted so we can see player, we considere that the displacement_ratio of the layer containing the player start is the reference one
		reference_layer = layer_instance;

		return CreatePlayerPawn(player, player_start, layer_instance);
	}

	void TiledMapLevelInstance::CreateBackgroundImage()
	{
		std::string const* background_material = nullptr;
		std::string const* background_texture = nullptr;

		TiledMapLevel const* level = GetLevel();
		if (level != nullptr)
		{
			background_material = level->GetTiledMap()->FindPropertyString("BACKGROUND_MATERIAL");
			background_texture = level->GetTiledMap()->FindPropertyString("BACKGROUND_TEXTURE");
		}

		game->CreateBackgroundImage(
			(background_material == nullptr) ? nullptr : background_material->c_str(),
			(background_texture == nullptr) ? nullptr : background_texture->c_str());
	}

	void TiledMapLevelInstance::SetInGameMusic()
	{
		std::string const* level_music = nullptr;

		TiledMapLevel const* level = GetLevel();
		if (level != nullptr)
			level_music = level->GetTiledMap()->FindPropertyString("MUSIC");

		if (level_music == nullptr)
			death::LevelInstance::SetInGameMusic();
		else
			game->SetInGameMusic(level_music->c_str());
	}

	LevelCheckpoint* TiledMapLevelInstance::DoCreateCheckpoint() const
	{
		return new TiledMapLevelCheckpoint();
	}

	bool TiledMapLevelInstance::DoSaveIntoCheckpoint(LevelCheckpoint* checkpoint) const
	{
		TiledMapLevelCheckpoint* tiled_level_checkpoint = auto_cast(checkpoint);
		if (tiled_level_checkpoint == nullptr)
			return false;

		if (!death::LevelInstance::DoSaveIntoCheckpoint(checkpoint))
			return false;

		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
		{
			// the layer (death::TiledMap point of view)
			TiledMapLayerInstance const* layer = layer_instances[i].get();
			if (layer == nullptr)
				continue;
			// create the checkpoint 
			TiledMapLayerCheckpoint* layer_checkpoint = layer_instances[i]->SaveIntoCheckpoint();
			if (layer_checkpoint == nullptr)
				continue;
			// insert the layer_checkpoint in level_checkpoint
			tiled_level_checkpoint->layer_checkpoints[layer->GetLayerID()] = layer_checkpoint;
		}

		return true;
	}

	bool TiledMapLevelInstance::DoLoadFromCheckpoint(LevelCheckpoint const* checkpoint)
	{
		TiledMapLevelCheckpoint const* tiled_level_checkpoint = auto_cast(checkpoint);
		if (tiled_level_checkpoint == nullptr)
			return false;

		// super method
		if (!death::LevelInstance::DoLoadFromCheckpoint(checkpoint))
			return false;

		// iterate over layers that have serialized a checkpoint
		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
		{
			// the layer
			TiledMapLayerInstance* layer = layer_instances[i].get();
			if (layer == nullptr)
				continue;
			// find the corresponding checkpoint
			auto it = tiled_level_checkpoint->layer_checkpoints.find(layer->GetLayerID());
			if (it == tiled_level_checkpoint->layer_checkpoints.end())
				continue;
			// load layer_chackpoint
			layer->LoadFromCheckpoint(it->second.get());
		}
		return true;
	}

	void TiledMapLevelInstance::OnLevelEnded()
	{
		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
			layer_instances[i]->OnLevelEnded();

		death::LevelInstance::OnLevelEnded();
	}

	void TiledMapLevelInstance::OnLevelStarted()
	{
		death::LevelInstance::OnLevelStarted();

		size_t count = layer_instances.size();
		for (size_t i = 0; i < count; ++i)
			layer_instances[i]->OnLevelStarted();
	}


}; // namespace death
