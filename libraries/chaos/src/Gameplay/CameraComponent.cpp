#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{

	// =================================================
	// CameraComponent
	// =================================================

	CHAOS_IMPLEMENT_GAMEPLAY_GETTERS(CameraComponent);

	box2 CameraComponent::ApplyModifier(box2 const & src) const
	{
		return src;
	}

	void CameraComponent::OnInsertedInto(Camera * in_camera)
	{
		assert(in_camera != nullptr);
	}

	void CameraComponent::OnRemovedFrom(Camera * in_camera)
	{
		assert(in_camera != nullptr);
	}

}; // namespace chaos
