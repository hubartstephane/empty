#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/RenderMaterial.h>

// -----------------------------------------------------------------

// all classes in this file
#define CHAOS_PARTICLE_CLASSES (ParticleRange) (ParticleRangeAllocation) (ParticleLayerBase) (ParticleManager)

// forward declaration
#define CHAOS_PARTICLE_FORWARD_DECL(r, data, elem) class elem;
BOOST_PP_SEQ_FOR_EACH(CHAOS_PARTICLE_FORWARD_DECL, _, CHAOS_PARTICLE_CLASSES)

// friendship macro
#define CHAOS_PARTICLE_FRIEND_DECL(r, data, elem) friend class elem;
#define CHAOS_PARTICLE_ALL_FRIENDS BOOST_PP_SEQ_FOR_EACH(CHAOS_PARTICLE_FRIEND_DECL, _, CHAOS_PARTICLE_CLASSES)

// ==============================================================
// PARTICLE RANGE
// ==============================================================

class ParticleRange
{
	CHAOS_PARTICLE_ALL_FRIENDS

public:

	/** the beginning of the range */
	size_t start = 0;
	/** the number of element in the range */
	size_t count = 0;
};

// ==============================================================
// PARTICLE RANGE ALLOCATION
// ==============================================================

class ParticleRangeAllocation : public chaos::ReferencedObject
{
	CHAOS_PARTICLE_ALL_FRIENDS

public:

	/** destructor */
	virtual ~ParticleRangeAllocation();
	/** remove the allocation from its layer */
	void RemoveFromLayer();

	/** get the range of concern */
	ParticleRange GetParticleRange() const;
	/** get the number of particles */
	size_t GetParticleCount() const;
	/** get the particles */
	void * GetParticleBuffer();
	/** get the particles */
	void const * GetParticleBuffer() const;

protected:

	/** the particle layer that contains the range */
	ParticleLayerBase * layer = nullptr;
	/** the index of the range in its array */
	size_t range_index = std::numeric_limits<size_t>::max();
	/** whether the particles existence in linked to the allocation lifetime */
	bool particles_owner = true;
};

// ==============================================================
// PARTICLE LAYER BASE
// ==============================================================

class ParticleLayerBase : public chaos::ReferencedObject
{
	CHAOS_PARTICLE_ALL_FRIENDS

public:

	static size_t const DESTROY_PARTICLE_MARK = std::numeric_limits<size_t>::max();

	/** destructor */
	virtual ~ParticleLayerBase();

	/** get the total number of particles */
	size_t GetParticleCount() const;
	/** get the number of particles */
	size_t GetParticleCount(ParticleRange range) const;
	/** get the particles */
	void * GetParticleBuffer(ParticleRange range);
	/** get the particles */
	void const * GetParticleBuffer(ParticleRange range) const;

  /** pause/resume the layer */
  void Pause(bool in_paused = true);
  /** returns whether the layer is paused */
  bool IsPaused() const;

  /** show/hide the layer */
  void Show(bool in_visible = true);
  /** returns whether the layer is visible */
  bool IsVisible() const;

	/** ticking the particle system */
	virtual void TickParticles(float delta_time);
	/** spawn a given number of particles */
	ParticleRange SpawnParticles(size_t count);
	/** spawn a given number of particles and keep a range */
	ParticleRangeAllocation *SpawnParticlesAndKeepRange(size_t count, bool particles_owner = true);

	/** mark any particle as to be destroyed next tick */
	void MarkParticlesToDestroy(size_t start, size_t count);

protected:

  /** returns whether it is necessary to tick particles (not necessary for static elements) */
  virtual bool AreParticlesDynamic() const;
  /** returns whether particles may have a limited lifetime */
  virtual bool AreParticlesMortal() const;

	/** unlink all particles allocations */
	void DetachAllParticleAllocations();
	/** internal method to remove a range from the layer */
	void RemoveParticleAllocation(ParticleRangeAllocation * allocation);
	/** internal method to update the particles */
	virtual void UpdateParticles(float delta_time);
	/** internal method to test whether particles should be destroyed (returns the number of particles still in the layer) */
	virtual size_t DestroyObsoletParticles();
	/** internal method fix the ranges after particles destruction */
	virtual void UpdateParticleRanges(size_t new_particle_count);

	template<typename T>
	void DoUpdateParticles(float delta_time, T & obj)
	{
		size_t particle_count = GetParticleCount();
		if (particle_count > 0)
		{
			T::particle_type * p = (T::particle_type *)(&particles[0]);
			for (size_t i = 0; i < particle_count; ++i)
				obj.UpdateParticle(delta_time, &p[i]);
		}
	}

	template<typename T>
	size_t DoDestroyObsoletParticles(T & obj)
	{
		size_t particle_count = GetParticleCount();
		if (particle_count > 0)
		{
			T::particle_type * p = (T::particle_type *)(&particles[0]);

			size_t i = 0;
			size_t j = 0;
			while (i < particle_count)
			{
				if (deletion_vector[i] != DESTROY_PARTICLE_MARK && !obj.MustDestroyParticle(&p[i])) // particle is still alive ?
				{
					if (i != j)
						p[j] = p[i]; // keep the particle by copying it 
          deletion_vector[i] = (i - j);
					++j;
				}
				else
          deletion_vector[i] = (i - j);
				++i;
			}
      return j;
		}
    return deletion_vector.size();
	}

protected:

	/** the size of one particle */
	size_t particle_size = 0;
	/** the size of one vertex */
	size_t vertex_size = 0;
	/** the order of the layer in the manager */
	int render_order = 0;
  /** whether the layer is paused */
  bool paused = false;
  /** whether the layer is visible */
  bool visible = true;
  /** number of particles waiting for a destruction */
  size_t pending_kill_particles = 0;
  /** whether there was changes in particles, and a vertex array need to be recomputed */
  bool require_GPU_update = false;
	/** the material used to render the layer */
	boost::intrusive_ptr<chaos::RenderMaterial> render_material;
	/** the array containing the particles */
	std::vector<char> particles;
	/** a utility vector that is used to mark particles to destroy, then as an internal utility vector */
	std::vector<size_t> deletion_vector;
	/** particles ranges */
	std::vector<ParticleRange> particles_ranges;
	/** ranges reservations */
	std::vector<ParticleRangeAllocation*> range_allocations;
};

// ==============================================================
// CALLBACKS
// ==============================================================

template<typename PARTICLE_DESC_TYPE>
class ParticleLayer : public ParticleLayerBase
{
	CHAOS_PARTICLE_ALL_FRIENDS

public:

	using particle_desc_type = PARTICLE_DESC_TYPE;
	using particle_type = typename PARTICLE_DESC_TYPE::particle_type;
	using vertex_type = typename PARTICLE_DESC_TYPE::vertex_type;

	/** constructor */
	ParticleLayer(particle_desc_type in_desc = particle_desc_type()) :
		particle_desc(in_desc) 
	{
    particle_size = sizeof(particle_type);
	}

protected:

	virtual void UpdateParticles(float delta_time) override
	{
		DoUpdateParticles(delta_time, particle_desc);
	}

	virtual size_t DestroyObsoletParticles() override
	{
		return DoDestroyObsoletParticles(particle_desc);
	}

  virtual bool AreParticlesDynamic() const override
  {
    return particle_desc.AreParticlesDynamic();
  }

  virtual bool AreParticlesMortal() const override
  {
    return particle_desc.AreParticlesMortal();
  }

protected:

	/** the particle behavior description */
	particle_desc_type particle_desc;
};

// ==============================================================
// CALLBACKS
// ==============================================================

template<typename PARTICLE_TYPE, typename VERTEX_TYPE>
class ParticleLayerDescBase
{
	CHAOS_PARTICLE_ALL_FRIENDS

public:

	using particle_type = typename PARTICLE_TYPE;
	using vertex_type = typename VERTEX_TYPE;

	inline bool MustDestroyParticle(particle_type const * p){ return false;}

  inline void UpdateParticle(float delta_time, particle_type * p) {}

  inline bool AreParticlesDynamic() const { return false; }

  inline bool AreParticlesMortal() const { return false; }
};


// ==============================================================
// CALLBACKS
// ==============================================================

class ParticleManager : public chaos::ReferencedObject
{
	CHAOS_PARTICLE_ALL_FRIENDS


};






// ==============================================================
// CALLBACKS
// ==============================================================

class ParticleExample
{
  glm::vec3 position;

};

class VertexExample
{

};

class ParticleLayerDescExample : public ParticleLayerDescBase<ParticleExample, VertexExample>
{


};


// undefine macros
#undef CHAOS_PARTICLE_CLASSES
#undef CHAOS_PARTICLE_FORWARD_DECL
#undef CHAOS_PARTICLE_FRIEND_DECL
#undef CHAOS_PARTICLE_ALL_FRIENDS