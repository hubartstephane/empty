#include "SoundTest.h"

// ==============================================================
// PARTICLE RANGE ALLOCATION
// ==============================================================

ParticleRangeAllocation::~ParticleRangeAllocation()
{
	RemoveFromLayer();
}

void ParticleRangeAllocation::RemoveFromLayer()
{
	if (layer == nullptr)
		return;
	layer->RemoveParticleAllocation(this);
}

ParticleRange ParticleRangeAllocation::GetParticleRange() const
{
	if (layer == nullptr)
		return ParticleRange();
	return layer->particles_ranges[range_index];
}

size_t ParticleRangeAllocation::GetParticleCount() const
{
	return GetParticleRange().count;
}

void * ParticleRangeAllocation::GetParticleBuffer()
{
	if (layer == nullptr)
		return nullptr;
	return layer->GetParticleBuffer(layer->particles_ranges[range_index]);
}

void const * ParticleRangeAllocation::GetParticleBuffer() const
{
	if (layer == nullptr)
		return nullptr;
	return layer->GetParticleBuffer(layer->particles_ranges[range_index]);
}

// ==============================================================
// CALLBACKS
// ==============================================================

ParticleLayerBase::~ParticleLayerBase()
{
	DetachAllParticleAllocations();
}

void ParticleLayerBase::DetachAllParticleAllocations()
{
	while (range_allocations.size())
		RemoveParticleAllocation(range_allocations[range_allocations.size() - 1]);
}

size_t ParticleLayerBase::GetParticleCount() const
{
	if (particle_size == 0)
		return 0;
	return particles.size() / particle_size;
}

size_t ParticleLayerBase::GetParticleCount(ParticleRange range) const
{
	if (particle_size == 0)
		return 0;
	return range.count / particle_size;
}

void * ParticleLayerBase::GetParticleBuffer(ParticleRange range)
{
	if (particle_size == 0)
		return nullptr;
	return &particles[range.start * particle_size];
}

void const * ParticleLayerBase::GetParticleBuffer(ParticleRange range) const
{
	if (particle_size == 0)
		return nullptr;
	return &particles[range.start * particle_size];
}

void ParticleLayerBase::Pause(bool in_paused)
{
  paused = in_paused;
}

bool ParticleLayerBase::IsPaused() const
{
  return paused;
}

void ParticleLayerBase::Show(bool in_visible)
{
  visible = in_visible;
}

bool ParticleLayerBase::IsVisible() const
{
  return visible;
}

bool ParticleLayerBase::AreParticlesDynamic() const
{
  return true;
}

bool ParticleLayerBase::AreParticlesMortal() const
{
  return true;
}

void ParticleLayerBase::TickParticles(float delta_time)
{
  // early exit
  if (IsPaused())
    return;

	// update the particles themselves
  if (AreParticlesDynamic())
  {
    UpdateParticles(delta_time);
    require_GPU_update = true;
  }
	// destroy the particles that are to be destroyed
  if (pending_kill_particles > 0 || AreParticlesMortal())
  {
    size_t new_particle_count = DestroyObsoletParticles();
    if (new_particle_count != GetParticleCount())
    {
      UpdateParticleRanges(new_particle_count);
      require_GPU_update = true;
    }
    pending_kill_particles = 0;
  }
}

ParticleRangeAllocation * ParticleLayerBase::SpawnParticlesAndKeepRange(size_t count, bool particles_owner)
{
	ParticleRange range = SpawnParticles(count);
	if (range.count == 0)
		return nullptr;

	ParticleRangeAllocation * result = new ParticleRangeAllocation;

	result->layer = this;
	result->range_index = particles_ranges.size();
	result->particles_owner = particles_owner;

	particles_ranges.push_back(range);
	range_allocations.push_back(result);

	return result;
}

ParticleRange ParticleLayerBase::SpawnParticles(size_t count)
{
	ParticleRange result;
	if (count > 0 && particle_size > 0)
	{
		size_t current_particle_count = GetParticleCount();

		// initialize the result
		result.start = current_particle_count;
		result.count = count;
		// create the particles and the suppression corresponding data
		size_t new_count = current_particle_count + count;
		particles.resize(new_count * particle_size, 0);
		deletion_vector.resize(new_count, 0);	
	}
	return result;
}

void ParticleLayerBase::MarkParticlesToDestroy(size_t start, size_t count) 
{
	// clamp the range
	size_t suppression_count = deletion_vector.size();
	if (start >= suppression_count)
		return;
	size_t end = min(start + count, suppression_count);
	// mark the particles to destroy    
	while (start != end)
    deletion_vector[start++] = DESTROY_PARTICLE_MARK;
  // count the number of particles to destroy
  pending_kill_particles += count;
}

void ParticleLayerBase::RemoveParticleAllocation(ParticleRangeAllocation * allocation)
{
	assert(allocation != nullptr);

	// mark the range to be destroyed
	if (allocation->particles_owner)
	{
		ParticleRange range = allocation->GetParticleRange();
		MarkParticlesToDestroy(range.start, range.count);
	}
	// displace range and allocation
	size_t last_index = range_allocations.size() - 1;
	if (allocation->range_index < last_index)
	{
		range_allocations[allocation->range_index] = range_allocations[last_index]; // replace the allocation
		particles_ranges[allocation->range_index]  = particles_ranges[last_index];	
	}
	range_allocations.pop_back();
	particles_ranges.pop_back();

	// reset the object
	*allocation = ParticleRangeAllocation();
}

void ParticleLayerBase::UpdateParticles(float delta_time)
{

}

size_t ParticleLayerBase::DestroyObsoletParticles()
{
  return deletion_vector.size();
}

void ParticleLayerBase::UpdateParticleRanges(size_t new_particle_count)
{
	// update the ranges (code is useless from one TICK to the next. the only important value is NUMERIC LIMIT)
	size_t range_count = particles_ranges.size();
	for (size_t i = 0; i < range_count; ++i)
	{
    ParticleRange & range = particles_ranges[i];
    if (range.count == 0)
      continue;
		// read the range
		size_t start = range.start;
		size_t end   = start + range.count - 1;
		// apply the suppression count
		start -= deletion_vector[start];
		end   -= deletion_vector[end];
		// update the structure
    range.start = start;
    range.count = end - start + 1;
	}
  // resize some vectors
  particles.resize(new_particle_count * particle_size);
  deletion_vector.resize(new_particle_count);
	// reset the suppression vector
	for (size_t i = 0; i < new_particle_count; ++i)
    deletion_vector[i] = 0;
}




// ==============================================================
// CALLBACKS
// ==============================================================

