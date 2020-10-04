#pragma once


#include <chaos/StandardHeaders.h> 
#include <death/PlayerDisplacementComponent.h>

class LudumPlayerDisplacementComponent : public death::PlayerDisplacementComponent
{
	CHAOS_OBJECT_DECLARE_CLASS2(LudumPlayerDisplacementComponent, death::PlayerDisplacementComponent);


public:

	static void ComputeBorderCollision(class ParticleBase& particle, class LudumLevelInstance* li, class CarData const& car_data);


protected:

	/** override */
	virtual bool DoTick(float delta_time) override;





};
