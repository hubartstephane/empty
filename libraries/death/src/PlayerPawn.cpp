#include <death/PlayerPawn.h>
#include <death/GameCheckpoint.h>

#include <death/GameGettersImpl.h>

namespace death
{
	DEATH_GAMEGETTERS_IMPLEMENT(PlayerPawn);

	// =============================================
	// PlayerPawn
	// =============================================

	bool PlayerPawn::Initialize(Player* in_player)
	{
		// ensure valid arguments and not already initialized
		assert(in_player != nullptr);
		assert(player == nullptr);
		player = in_player;
		return true;
	}

	chaos::ParticleDefault* PlayerPawn::GetParticle()
	{
		return chaos::ParticleTools::GetParticle(GetAllocation(), 0);
	}

	chaos::ParticleDefault const* PlayerPawn::GetParticle() const
	{
		return chaos::ParticleTools::GetParticle(GetAllocation(), 0);
	}

	glm::vec2 PlayerPawn::GetPosition() const
	{
		return chaos::ParticleTools::GetParticlePosition(GetAllocation(), 0);
	}

	chaos::box2 PlayerPawn::GetBox() const
	{
		return chaos::ParticleTools::GetParticleBox(GetAllocation(), 0);
	}

	bool PlayerPawn::SetPosition(glm::vec2 const& position)
	{
		return chaos::ParticleTools::SetParticlePosition(GetAllocation(), 0, position);
	}
	bool PlayerPawn::SetBox(chaos::box2 const& box)
	{
		return chaos::ParticleTools::SetParticleBox(GetAllocation(), 0, box);
	}

	void PlayerPawn::SetAllocation(chaos::ParticleAllocationBase* in_allocation)
	{
		allocations = in_allocation;
	}
	
	bool PlayerPawn::DoSaveIntoCheckpoint(PlayerPawnCheckpoint* checkpoint) const
	{
		checkpoint->pawn_box = GetBox();
		return true;
	}
	
	bool PlayerPawn::DoLoadFromCheckpoint(PlayerPawnCheckpoint const* checkpoint)
	{
		SetBox(checkpoint->pawn_box);
		return true;
	}
	
}; // namespace death

