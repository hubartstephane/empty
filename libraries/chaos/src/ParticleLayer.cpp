#include <chaos/ParticleManager.h>
#include <chaos/GLTools.h>
#include <chaos/DrawPrimitive.h>
#include <chaos/GPUResourceManager.h>
#include <chaos/MyGLFWSingleWindowApplication.h>
#include <chaos/ParticleTools.h>

namespace chaos
{

	// ==============================================================
	// ParticleAllocationBase
	// ==============================================================

	ParticleAllocationBase::ParticleAllocationBase(ParticleLayerBase * in_layer) :
		layer(in_layer)
	{
		assert(in_layer != nullptr);
	}

	ParticleAllocationBase::~ParticleAllocationBase()
	{
		assert(layer == nullptr);
	}

	void ParticleAllocationBase::RemoveFromLayer()
	{
		if (layer == nullptr)
			return;
		layer->RemoveParticleAllocation(this);
	}

	void ParticleAllocationBase::OnRemovedFromLayer()
	{
		ConditionalRequireGPUUpdate(true, true);
		layer = nullptr;
	}

	void ParticleAllocationBase::ConditionalRequireGPUUpdate(bool skip_if_invisible, bool skip_if_empty)
	{
		if (layer == nullptr)
			return;
		if (skip_if_invisible && !IsVisible())
			return;
		if (skip_if_empty && GetParticleCount() == 0)
			return;
		layer->require_GPU_update = true;
	}

	bool ParticleAllocationBase::IsAttachedToLayer() const
	{
		return (layer != nullptr);
	}

	void ParticleAllocationBase::Pause(bool in_paused)
	{
		paused = in_paused;
	}

	bool ParticleAllocationBase::IsPaused() const
	{
		return paused;
	}

	void ParticleAllocationBase::Show(bool in_visible)
	{
		if (visible != in_visible)
		{
			visible = in_visible;
			ConditionalRequireGPUUpdate(false, true); // the GPU buffer is about to be changed
		}
	}

	bool ParticleAllocationBase::IsVisible() const
	{
		return visible;
	}

	bool ParticleAllocationBase::AddParticles(size_t extra_count)
	{
		return Resize(extra_count + GetParticleCount());
	}

	//
	// XXX : SpawnParticle(...) returns a raw pointer on ParticleAllocation
	//       the user has to possibilities
	//         - he ignores the result => the allocation is stored inside the layer and will be destroyed when the layer is destroyed
	//         - he stores the result into an intrusive_ptr<...>
	//              -> that means that he wants to override the allocation lifetime
	//              -> as soon as this intrusive_ptr<...> is destroyed, we want to destroy the ParticleAllocation even if there still is 1 reference
	//                 from the layer
	//
	void ParticleAllocationBase::SubReference(SharedPointerPolicy policy)
	{
		if (layer == nullptr)
		{
			ReferencedObject::SubReference(policy); // the ParticleAllocation is handled as usual
		}
		else
		{
			if (--shared_count == 1) // the last reference is the one from the layer. Destroy it
				RemoveFromLayer();
		}
	}

	// ==============================================================
	// ParticleLayerBase
	// ==============================================================

	ParticleLayerBase::ParticleLayerBase()
	{
	}

	ParticleLayerBase::~ParticleLayerBase()
	{
		DetachAllParticleAllocations();
	}

	void ParticleLayerBase::DetachAllParticleAllocations()
	{
		// faster to do that from end to begin
		while (particles_allocations.size())
			RemoveParticleAllocation(particles_allocations[particles_allocations.size() - 1].get());
	}

	void ParticleLayerBase::RemoveParticleAllocation(ParticleAllocationBase * allocation)
	{
		assert(allocation != nullptr);
		assert(allocation->layer == this);

		for (size_t i = particles_allocations.size(); i > 0; --i)
		{
			size_t index = i - 1;
			if (particles_allocations[index] == allocation)
			{
				allocation->OnRemovedFromLayer();
				particles_allocations.erase(particles_allocations.begin() + index);
				return;
			}
		}
	}

	bool ParticleLayerBase::DoTick(double delta_time)
	{
		// update the particles themselves
		if (AreParticlesDynamic())
			require_GPU_update |= TickAllocations(delta_time);
		return true;
	}

	bool ParticleLayerBase::TickAllocations(double delta_time)
	{
		bool result = false;

		// store allocations that want to be notified of their emptyness here,
		// to be handled after the main loop
		std::vector<ParticleAllocationBase *> to_destroy_allocations;

		// main loop
		size_t count = particles_allocations.size();
		for (size_t i = 0; i < count; ++i)
		{
			ParticleAllocationBase * allocation = particles_allocations[i].get();
			if (allocation == nullptr)
				continue;
			// tick and register as an allocation to be destroyed
			bool destroy_allocation = allocation->TickAllocation((float)delta_time, GetLayerTrait());
			if (destroy_allocation)
				to_destroy_allocations.push_back(allocation);
			// particles have changed ... so must it be for vertices
			result = true;
		}

		// handle allocation that wanted to react whenever they become empty
		// XXX : this is done outside the loop because this could callback can create or destroy some allocations 
		//       causing iteration in 'particles_allocations' dangerous
		size_t empty_count = to_destroy_allocations.size();
		for (size_t i = 0; i < empty_count; ++i)
			to_destroy_allocations[i]->RemoveFromLayer();

		return result;
	}

	ParticleAllocationBase * ParticleLayerBase::SpawnParticles(size_t count)
	{
		// create an allocation
		ParticleAllocationBase * result = DoSpawnParticles(count);
		if (result == nullptr)
			return nullptr;
		// increase the particle count for that allocation
		result->Resize(count);
		// register the allocation
		particles_allocations.push_back(result);
		return result;
	}

	int ParticleLayerBase::DoDisplay(Renderer * renderer, GPUProgramProviderBase const * uniform_provider, RenderParams const & render_params) const
	{
		// early exit
		if (vertices_count == 0)
			return 0;
		// search the material
		GPURenderMaterial const * final_material = render_params.GetMaterial(this, render_material.get());
		// prepare rendering state
		UpdateRenderingStates(renderer, true);
		// update uniform provider with atlas, and do the rendering
		chaos::GPUProgramProviderChain main_uniform_provider(uniform_provider);
		if (atlas != nullptr)
			main_uniform_provider.AddVariableTexture("material", atlas->GetTexture());

		int result = DoDisplayHelper(renderer, vertices_count, final_material, (atlas == nullptr) ? uniform_provider : &main_uniform_provider, render_params);
		// restore rendering states
		UpdateRenderingStates(renderer, false);
		return result;
	}

	bool ParticleLayerBase::DoUpdateGPUResources(Renderer * renderer) const
	{
		// update the vertex declaration
		UpdateVertexDeclaration();
		// return the number of vertices from the previous call
		if (!require_GPU_update && !AreVerticesDynamic())
			return true;

		// create the vertex buffer if necessary
		bool dynamic_buffer = (AreVerticesDynamic() || AreParticlesDynamic());

		if (vertex_buffer == nullptr)
		{
			vertices_count = 0; // no vertices inside for the moment

			vertex_buffer = new GPUVertexBuffer(dynamic_buffer);
			if (vertex_buffer == nullptr || !vertex_buffer->IsValid())
			{
				vertex_buffer = nullptr;
				return false;
			}
		}

		// release memory => maybe this is worth delay this action after a while and being sure the data inside is not necessary anymore
		size_t vertex_buffer_size = GetVertexSize() * GetVerticesPerParticle() * ComputeMaxParticleCount();
		if (vertex_buffer_size == 0)
		{
			vertex_buffer->SetBufferData(nullptr, 0); // empty the buffer : for some reason, we cannot just kill the buffer
			vertices_count = 0;
			return true;
		}

		// reserve memory (for the maximum number of vertices possible)
		if (!vertex_buffer->SetBufferData(nullptr, vertex_buffer_size))
		{
			vertices_count = 0;
			return false;
		}
		// map the vertex buffer
		char * buffer = vertex_buffer->MapBuffer(0, vertex_buffer_size, false, true);
		if (buffer == nullptr)
		{
			vertices_count = 0;
			return false;
		}
		// fill the buffer with data
		vertices_count = DoUpdateGPUBuffers(buffer, vertex_buffer_size);
		// unmap the buffer
		vertex_buffer->UnMapBuffer();

		// no more update required
		require_GPU_update = false;

		return true;
	}

	int ParticleLayerBase::DoDisplayHelper(Renderer * renderer, size_t vertex_count, GPURenderMaterial const * final_material, GPUProgramProviderBase const * uniform_provider, RenderParams const & render_params) const
	{
		// no vertices, no rendering
		if (vertex_count == 0)
			return 0;
		// get the vertex array
		GPUVertexArray const * vertex_array = vertex_array_cache.FindOrCreateVertexArray(final_material->GetEffectiveProgram(render_params), vertex_buffer.get(), nullptr, vertex_declaration, 0);
		if (vertex_array == nullptr)
			return 0;
		// use the material
		final_material->UseMaterial(uniform_provider, render_params);
		// bind the vertex array
		glBindVertexArray(vertex_array->GetResourceID());
		// one draw call for the whole buffer
		DrawPrimitive primitive;
		primitive.primitive_type = GL_TRIANGLES;
		primitive.indexed = false;
		primitive.count = (int)vertex_count;
		primitive.start = 0;
		primitive.base_vertex_index = 0;
		renderer->Draw(primitive, render_params.instancing);
		glBindVertexArray(0);
		return 1; // 1 DrawCall
	}

	void ParticleLayerBase::UpdateVertexDeclaration() const
	{
		// is the vertex declaration already filled
		if (vertex_declaration.entries.size() > 0)
			return;
		// fill the vertex declaration
		vertex_declaration = GetVertexDeclaration();
	}

	size_t ParticleLayerBase::DoUpdateGPUBuffers(char * buffer, size_t vertex_buffer_size) const
	{
		size_t result = 0;

		size_t vertex_size = GetVertexSize();

		size_t count = particles_allocations.size();
		for (size_t i = 0; i < count; ++i)
		{
			// get the allocation, ignore if invisible
			ParticleAllocationBase * allocation = particles_allocations[i].get();
			if (!allocation->IsVisible())
				continue;
			// transform particles into vertices
			size_t new_vertices = allocation->ParticlesToVertices(buffer, GetLayerTrait());
			// shift buffer
			buffer += new_vertices * vertex_size;
			result += new_vertices;
		}

		return result;
	}

	size_t ParticleLayerBase::ComputeMaxParticleCount() const
	{
		size_t result = 0;

		size_t count = particles_allocations.size();
		for (size_t i = 0; i < count; ++i)
		{
			// get the allocation, ignore if invisible
			ParticleAllocationBase * allocation = particles_allocations[i].get();
			if (!allocation->IsVisible())
				continue;
			// ignore empty allocations
			size_t particle_count = allocation->GetParticleCount();
			if (particle_count == 0)
				continue;

			result += allocation->GetParticleCount();
		}
		return result;
	}

	void ParticleLayerBase::UpdateRenderingStates(Renderer * renderer, bool begin) const
	{
		if (begin)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		}
	}

	size_t ParticleLayerBase::GetAllocationCount() const
	{
		return particles_allocations.size();
	}
	ParticleAllocationBase * ParticleLayerBase::GetAllocation(size_t index)
	{
		return particles_allocations[index].get();
	}
	ParticleAllocationBase const * ParticleLayerBase::GetAllocation(size_t index) const
	{
		return particles_allocations[index].get();
	}

}; // namespace chaos

