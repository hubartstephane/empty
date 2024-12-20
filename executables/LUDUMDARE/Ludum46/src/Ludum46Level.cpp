#include "Ludum46PCH.h"
#include "Ludum46Level.h"
#include "Ludum46LevelInstance.h"
#include "Ludum46Game.h"
#include "Ludum46Player.h"
#include "Ludum46GameInstance.h"

// =============================================================
// EffectorObject implementation
// =============================================================

bool EffectorObject::Initialize(chaos::TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const * in_geometric_object, chaos::TMObjectReferenceSolver& in_reference_solver)
{


	return true;
}

// =============================================================
// SpikeBar implementation
// =============================================================

bool SpikeBar::Initialize(chaos::TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const * in_geometric_object, chaos::TMObjectReferenceSolver& in_reference_solver)
{
	if (!EffectorObject::Initialize(in_layer_instance, in_geometric_object, in_reference_solver))
		return false;
	if (!chaos::TMObject::Initialize(in_layer_instance, in_geometric_object, in_reference_solver))
		return false;

	in_reference_solver.DeclareReference(my_path, "MY_PATH", in_geometric_object);


	//SetParticleMaster(false);

	return true;
}

bool SpikeBar::DoTick(float delta_time)
{
	chaos::TMObject::DoTick(delta_time);

	// data from entity to particle
	SynchronizeData(false);

	path_value += delta_time;

	chaos::TMPath * p = auto_cast(my_path.get());
	if (p != nullptr)
	{
		auto y = GetPosition();

		auto x = p->GetPosition();

		//SetPosition(p->GetPosition());



		p = p;
	}

	return true;
}


void SpikeBar::OnEffectorChangeState()
{


}

// =============================================================
// SoulTrigger implementation
// =============================================================

bool SoulTrigger::Initialize(chaos::TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const * in_geometric_object, chaos::TMObjectReferenceSolver& in_reference_solver)
{
	if (!chaos::TMObject::Initialize(in_layer_instance, in_geometric_object, in_reference_solver))
		return false;

	// number of element that must be triggered
	trigger_limit = in_geometric_object->GetPropertyValueInt("TRIGGER_LIMIT", trigger_limit);

	// read all the effectors
	std::string effectors = in_geometric_object->GetPropertyValueString("EFFECTORS", "");

	char const separator = ',';

	std::vector<std::string> name_array = chaos::StringTools::Split(effectors.c_str(), separator);
	for (std::string & name : name_array)
		effector_names.push_back(std::move(name));

	in_reference_solver.DeclareReference(myreference, "OBJECT_REFERENCE", in_geometric_object);

	// update internals (used whenever checkpoint is reloaded)
	trigger_count = 0;

	return true;
}

bool SoulTrigger::DoTick(float delta_time)
{
	chaos::TMObject::DoTick(delta_time);

	for (std::string const& effector_name : effector_names)
	{
		EffectorObject* effector = auto_cast(layer_instance->FindObject(effector_name));
		if (effector == nullptr)
			continue;
		effector->SetEffectorState(trigger_count >= trigger_limit);
	}
	trigger_count = 0;


	if (myreference != nullptr)
	{
		trigger_count = 0;

	}

	return true;
}

bool SoulTrigger::AddTriggerCount()
{
	++trigger_count; // will be triggered next tick

	return false; // do not destroy the particle
}

// =============================================================
// SoulBurnTrigger implementation
// =============================================================

bool SoulBurnTrigger::AddTriggerCount()
{
	SoulTrigger::AddTriggerCount();

	LudumLevelInstance* ludum_level_instance = GetLayerInstance()->GetLevelInstance();
	if (ludum_level_instance != nullptr)
	{
		LudumPlayer* player = ludum_level_instance->GetPlayer(0);
		if (player != nullptr)
			player->AddBurnedSouls(1);

	}
	return true;
}








// =============================================================
// Spawner implementation
// =============================================================

int Spawner::GetRemainingParticleCount() const
{
	if (max_spawned_particles < 0) // infinite case
		return -1;
	return (max_spawned_particles - spawned_count);
}

bool Spawner::Initialize(chaos::TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const * in_geometric_object, chaos::TMObjectReferenceSolver& in_reference_solver)
{
	if (!chaos::TMObject::Initialize(in_layer_instance, in_geometric_object, in_reference_solver))
		return false;

	max_spawned_particles = in_geometric_object->GetPropertyValueInt("MAX_SPAWNED_PARTICLES", max_spawned_particles);
	particle_start_velocity.x = in_geometric_object->GetPropertyValueFloat("PARTICLE_START_VELOCITY_X", particle_start_velocity.x);
	particle_start_velocity.y = in_geometric_object->GetPropertyValueFloat("PARTICLE_START_VELOCITY_Y", particle_start_velocity.y);
	spawn_per_second = in_geometric_object->GetPropertyValueFloat("SPAWN_PER_SECOND", spawn_per_second);
	target_layer = in_geometric_object->GetPropertyValueString("TARGET_LAYER", "");
	spawned_particle = in_geometric_object->GetPropertyValueString("SPAWNED_PARTICLE", "");
	particle_duration = in_geometric_object->GetPropertyValueFloat("PARTICLE_DURATION", particle_duration);
	emission_started = in_geometric_object->GetPropertyValueBool("EMISSION_STARTED", emission_started);
	// update internals (used whenever checkpoint is reloaded)
	spawned_count = 0;
	nospawn_time_cumulated = 0.0f;

	return true;
}


bool Spawner::SerializeIntoJSON(nlohmann::json * json) const
{
	if (!chaos::TMObject::SerializeIntoJSON(json))
		return false;

	chaos::JSONTools::SetAttribute(json, "MAX_SPAWNED_PARTICLES", max_spawned_particles);
	chaos::JSONTools::SetAttribute(json, "PARTICLE_START_VELOCITY_X", particle_start_velocity.x);
	chaos::JSONTools::SetAttribute(json, "PARTICLE_START_VELOCITY_Y", particle_start_velocity.y);
	chaos::JSONTools::SetAttribute(json, "SPAWN_PER_SECOND", spawn_per_second);
	chaos::JSONTools::SetAttribute(json, "TARGET_LAYER", target_layer);
	chaos::JSONTools::SetAttribute(json, "SPAWNED_PARTICLE", spawned_particle);
	chaos::JSONTools::SetAttribute(json, "PARTICLE_DURATION", particle_duration);
	chaos::JSONTools::SetAttribute(json, "EMISSION_STARTED", emission_started);

	return true;
}

bool Spawner::SerializeFromJSON(chaos::JSONReadConfiguration config)
{
	if (!chaos::TMObject::SerializeFromJSON(config))
		return false;

	chaos::JSONTools::GetAttribute(config, "MAX_SPAWNED_PARTICLES", max_spawned_particles);
	chaos::JSONTools::GetAttribute(config, "PARTICLE_START_VELOCITY_X", particle_start_velocity.x);
	chaos::JSONTools::GetAttribute(config, "PARTICLE_START_VELOCITY_Y", particle_start_velocity.y);
	chaos::JSONTools::GetAttribute(config, "SPAWN_PER_SECOND", spawn_per_second);
	chaos::JSONTools::GetAttribute(config, "TARGET_LAYER", target_layer);
	chaos::JSONTools::GetAttribute(config, "SPAWNED_PARTICLE", spawned_particle);
	chaos::JSONTools::GetAttribute(config, "PARTICLE_DURATION", particle_duration);
	chaos::JSONTools::GetAttribute(config, "EMISSION_STARTED", emission_started);

	return true;
}



bool Spawner::DoTick(float delta_time)
{
	chaos::TMObject::DoTick(delta_time);

	// whether the emission is enabled
	if (!emission_started)
		return true;
	// nothing to spawn
	if (spawn_per_second <= 0.0f)
		return true;
	// already all particles have been spawned
	if (max_spawned_particles > 0 && spawned_count >= max_spawned_particles)
		return true;

	// compute the number of particles to spawn
	int count = (int)(spawn_per_second * (delta_time + nospawn_time_cumulated));

	if (count == 0)
	{
		nospawn_time_cumulated += delta_time;
	}
	else
	{
		LudumLevelInstance * ludum_level_instance = GetLayerInstance()->GetLevelInstance();
		if (ludum_level_instance == nullptr)
			return true;

		chaos::ParticleSpawner spawner = ludum_level_instance->GetParticleSpawner(target_layer, spawned_particle);
		if (!spawner.IsValid())
			return false;

		SpawnParticles(spawner, count);
		spawned_count += count;
		nospawn_time_cumulated = 0.0f;
	}

	return true;
}

void Spawner::SpawnParticles(chaos::ParticleSpawner & spawner, int count)
{
	spawner.SpawnParticles(count, false).Process([this](chaos::ParticleAccessor<ParticleBase> accessor)
	{
		chaos::box2 bx = GetBoundingBox(false);

		for (ParticleBase& p : accessor)
		{
			p.bounding_box = bx;
			p.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			p.velocity = particle_start_velocity;
			p.duration = particle_duration;
		}
	});
}


// =============================================================
// Spawner implementation
// =============================================================


bool FireSpawner::Initialize(chaos::TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const * in_geometric_object, chaos::TMObjectReferenceSolver& in_reference_solver)
{
	if (!EffectorObject::Initialize(in_layer_instance, in_geometric_object, in_reference_solver))
		return false;
	if (!Spawner::Initialize(in_layer_instance, in_geometric_object, in_reference_solver))
		return false;

	return true;
}

void FireSpawner::OnEffectorChangeState()
{
	if (active)
		emission_started = true;
}

// =============================================================
// LudumLevel implementation
// =============================================================

LudumLevel::LudumLevel()
{
	level_instance_class = LudumLevelInstance::GetStaticClass();
}

chaos::ParticleLayerBase * LudumLevel::DoCreateParticleLayer(chaos::TMLayerInstance * layer_instance)
{
	LudumGame * ludum_game = auto_cast(layer_instance->GetGame());

	std::string const & layer_name = layer_instance->GetTiledLayer()->name;

	if (chaos::StringTools::Stricmp(layer_name, "PlayerAndCamera") == 0)
	{
		ParticlePlayerLayerTrait layer_trait;
		layer_trait.game = ludum_game;
		return new chaos::ParticleLayer<ParticlePlayerLayerTrait>(layer_trait);
	}

	if (chaos::StringTools::Stricmp(layer_name, "Souls") == 0)
	{
		ParticleSoulLayerTrait layer_trait;
		layer_trait.game = ludum_game;
		return new chaos::ParticleLayer<ParticleSoulLayerTrait>(layer_trait);
	}

	if (chaos::StringTools::Stricmp(layer_name, "Fire") == 0)
	{
		ParticleFireLayerTrait layer_trait;
		layer_trait.game = ludum_game;
		return new chaos::ParticleLayer<ParticleFireLayerTrait>(layer_trait);
	}

	if (chaos::StringTools::Stricmp(layer_name, "Blood") == 0)
	{
		ParticleBloodLayerTrait layer_trait;
		layer_trait.game = ludum_game;
		return new chaos::ParticleLayer<ParticleBloodLayerTrait>(layer_trait);
	}

	if (chaos::StringTools::Stricmp(layer_name, "BurnedSouls") == 0)
	{
		ParticleBurnedSoulLayerTrait layer_trait;
		layer_trait.game = ludum_game;
		return new chaos::ParticleLayer<ParticleBurnedSoulLayerTrait>(layer_trait);
	}

	if ((chaos::StringTools::Stricmp(layer_name, "BurningFlames") == 0) || (chaos::StringTools::Stricmp(layer_name, "Animated") == 0))
	{
		return new chaos::ParticleLayer<ParticleAnimatedLayerTrait>();
	}




	return chaos::TMLevel::DoCreateParticleLayer(layer_instance);
}


chaos::TMObjectFactory LudumLevel::DoGetObjectFactory(chaos::TMLayerInstance * in_layer_instance, chaos::TiledMap::TypedObject const * in_typed_object)
{
	if (in_typed_object->IsObjectOfType("Spawner"))
	{
		std::string const* spawner_type = in_typed_object->FindPropertyString("SPAWNER_TYPE");
		if (spawner_type != nullptr)
		{
			if (chaos::StringTools::Strcmp(*spawner_type, "SoulSpawner") == 0)
				return CHAOS_TM_MAKE_OBJECT_FACTORY(return new SoulSpawner(););
			if (chaos::StringTools::Strcmp(*spawner_type, "FireSpawner") == 0)
				return CHAOS_TM_MAKE_OBJECT_FACTORY(return new FireSpawner(););
		}
	}

	if (in_typed_object->IsObjectOfType("SoulTrigger"))
	{
		std::string const* trigger_type = in_typed_object->FindPropertyString("TRIGGER_TYPE");
		if (trigger_type != nullptr && chaos::StringTools::Strcmp(*trigger_type, "BurnTrigger") == 0)
			return CHAOS_TM_MAKE_OBJECT_FACTORY(return new SoulBurnTrigger(););
		else
			return CHAOS_TM_MAKE_OBJECT_FACTORY(return new SoulTrigger(););
	}

	if (in_typed_object->IsObjectOfType("SpikeBar"))
		return CHAOS_TM_MAKE_OBJECT_FACTORY(return new SpikeBar(););

	return chaos::TMLevel::DoGetObjectFactory(in_layer_instance, in_typed_object);
}


bool LudumLevel::Initialize(chaos::TiledMap::Map* in_tiled_map)
{
	if (!chaos::TMLevel::Initialize(in_tiled_map))
		return false;

	required_souls = in_tiled_map->GetPropertyValueInt("REQUIRED_SOULS", required_souls);
	if (required_souls <= 0)
		return false;

	flame_initial_health = in_tiled_map->GetPropertyValueFloat("FLAME_HEALTH", flame_initial_health);
	if (flame_initial_health <= 0.0f)
		return false;

	flame_health_lost_per_second = in_tiled_map->GetPropertyValueFloat("FLAME_HEALTH_LOST_PER_SECOND", flame_health_lost_per_second);
	if (flame_health_lost_per_second <= 0.0f)
		return false;

	return true;
}
