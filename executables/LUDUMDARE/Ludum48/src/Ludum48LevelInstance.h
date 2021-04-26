#pragma once

#include "Ludum48.h"
#include "Ludum48Game.h"

class GridCellInfo
{
public:

	bool CanLock(GameObjectParticle* p) const;

	void Lock(GameObjectParticle* p);
	void UnLock(GameObjectParticle* p);




	GameObjectParticle* particle = nullptr;

	bool locked = false;

	bool create_diamond = false;

#if _DEBUG
	box2 locked_by_box;
#endif
};

class GridInfo
{
public:

	~GridInfo() 
	{
		delete[] cells;
	}

	bool IsInside(glm::ivec2 const& p) const;

	GridCellInfo& operator ()(glm::ivec2 const& index);
	GridCellInfo const & operator ()(glm::ivec2 const& index) const;
	GridCellInfo& operator ()(glm::vec2 const & p);
	GridCellInfo const& operator ()(glm::vec2 const& p) const;

	glm::ivec2 GetIndexForPosition(glm::vec2 const& p) const;

	box2 GetBoundingBox(GridCellInfo const & cell) const;

	glm::ivec2 GetCellCoord(GridCellInfo const& cell) const;


	glm::ivec2 size = {0, 0};
	glm::vec2 tile_size = { 0, 0 };

	GridCellInfo* cells = nullptr;

	glm::vec2 min_position = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	glm::vec2 max_position = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
};





// =================================================
// LudumLevelInstance
// =================================================

class LudumLevelInstance : public TMLevelInstance
{
public:

	CHAOS_DECLARE_GAMEPLAY_FRIENDSHIPS(Ludum);

	CHAOS_DECLARE_OBJECT_CLASS2(LudumLevelInstance, TMLevelInstance);

	LudumLevelInstance();


	void TakeDiamond();

	int GetDiamondCount() const { return diamond_count; }
	int GetRequiredDiamondCount() const { return required_diamond_count; }

protected:

	/** override */
	virtual bool DoTick(float delta_time) override;
	/** override */
	virtual bool Initialize(Game * in_game, Level * in_level) override;
	/** override */
	virtual void CreateCameraComponents(Camera* camera, TMCameraTemplate* camera_template) override;
	/** override */
	virtual bool InitializeLevelInstance(TMObjectReferenceSolver& reference_solver, TiledMap::PropertyOwner const* property_owner) override;


	virtual int DoDisplay(GPURenderer* renderer, GPUProgramProviderBase const* uniform_provider, GPURenderParams const& render_params) override;

	/** override */
	virtual bool IsPlayerDead(Player* player) override;

	virtual bool CheckLevelCompletion() const override;

	virtual bool CanCompleteLevel() const override;

	virtual uint64_t GetCollisionFlagByName(char const* name) const override;

	void DiamondsCreationRequest(glm::ivec2 const& p);


	GridInfo const & GetGridInfo() const { return grid_info; }
	GridInfo& GetGridInfo(){ return grid_info; }

	

protected:

	void HandlePlayerObject(float delta_time);

	void HandleFallingObjects(float delta_time);

	void HandleMonsterObjects(float delta_time);

	void HandleBlobObjects(float delta_time);

	void HandleDisplacements(float delta_time);

	void CollectObjects();

	void KillPlayer(GameObjectParticle* player);

	void KillMonster(GameObjectParticle* monster);

	void CreateDiamonds();


	public:

	/** pointer on game */
	class LudumGame * game = nullptr;


	mutable float completion_timer = -1.0f; // shu48 : due to Checkcomplete constness !!
	float completion_delay = 1.0f;


	GridInfo grid_info;

	int diamond_count = 0;
	int required_diamond_count = 0;

	float frame_duration = 0.5f;
	float frame_timer = 0.0f;
	float object_speed = 3.0f;

	bool door_opened = false;
	bool level_complete = false;

};
