#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ParticleManager.h>
#include <chaos/ParticleDefault.h>
#include <chaos/EmptyClass.h>
#include <chaos/PrimitiveOutput.h>

namespace death
{

	// =====================================
	// TiledMapParticle
	// =====================================

	class TiledMapParticle : public chaos::ParticleDefault::Particle
	{
	public:

		int gid = 0;
		chaos::BitmapAtlas::BitmapInfo const* bitmap_info = nullptr;
	};

	// =====================================
	// TiledMapParticleTrait
	// =====================================

	class TiledMapParticleTrait : public chaos::ParticleAllocationTrait<TiledMapParticle, chaos::ParticleDefault::Vertex, false, false> // shuxxx set to false = optimization 1
	{
	public:

		static void ParticleToPrimitives(TiledMapParticle const& particle, chaos::QuadOutput<chaos::ParticleDefault::Vertex>& output)
		{
			chaos::ParticleDefault::ParticleTrait::ParticleToPrimitives(particle, output);
		}

		static void ParticleToPrimitives(TiledMapParticle const& particle, chaos::TrianglePairOutput<chaos::ParticleDefault::Vertex>& output)
		{
			chaos::ParticleDefault::ParticleTrait::ParticleToPrimitives(particle, output);
		}
	};

}; // namespace death
