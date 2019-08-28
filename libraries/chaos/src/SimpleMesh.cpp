#include <chaos/SimpleMesh.h>

#include <chaos/GLTools.h>
#include <chaos/GPURenderMaterial.h>

namespace chaos
{
	SimpleMesh::~SimpleMesh()
	{
		DoRelease();
	}

	void SimpleMesh::ShiftPrimitivesIndexAndVertexPosition(int vb_offset, int ib_offset)
	{
		if (vb_offset != 0 || ib_offset != 0)
			for (auto & primitive : primitives)
				primitive.ShiftIndexAndVertexPosition(vb_offset, ib_offset);
	}

	bool SimpleMesh::DoRelease()
	{
		vertex_buffer = nullptr;
		index_buffer = nullptr;

		vertex_declaration.Clear();
		primitives.clear();

		vertex_array_cache.Clear();
		return true;
	}

	void SimpleMesh::Render(GPURenderer * renderer, GPUProgram const * program, GPUProgramProviderBase const * uniform_provider, RenderParams const & render_params) const
	{
		// early exit
		if (program == nullptr)
			return;
		// use program
		program->UseProgram(uniform_provider);
		// do the rendering
		DoRender(renderer, program, render_params);
	}

	void SimpleMesh::Render(GPURenderer * renderer, GPURenderMaterial const * material, GPUProgramProviderBase const * uniform_provider, RenderParams const & render_params) const
	{
		// early exit
		if (material == nullptr)
			return;
		// get the program for the material, use it and bind its uniforms
		GPUProgram const * program = material->UseMaterial(uniform_provider, render_params);
		if (program == nullptr)
			return;
		// do the rendering
		DoRender(renderer, program, render_params);
	}

	void SimpleMesh::DoRender(GPURenderer * renderer, GPUProgram const * program, RenderParams const & render_params) const
	{
		assert(program != nullptr);
		// find the vertex array to use
		GPUVertexArray const * vertex_array = vertex_array_cache.FindOrCreateVertexArray(program, vertex_buffer.get(), index_buffer.get(), vertex_declaration, vertex_buffer_offset);
		if (vertex_array == nullptr)
			return;
		// bind the vertex array
		glBindVertexArray(vertex_array->GetResourceID());
		// render the primitives
		for (auto const & primitive : primitives)
			renderer->Draw(primitive, render_params.instancing);
		// unbind the vertex array
		glBindVertexArray(0);
	}

	void SimpleMesh::SetVertexBufferOffset(GLintptr in_vertex_buffer_offset)
	{
		vertex_buffer_offset = in_vertex_buffer_offset;
	}

}; // namespace chaos
