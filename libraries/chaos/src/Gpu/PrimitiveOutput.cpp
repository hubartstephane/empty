#include <chaos/Chaos.h>

namespace chaos
{
    PrimitiveOutputBase::PrimitiveOutputBase(GPUDynamicMesh* in_dynamic_mesh, GPUBufferCache* in_buffer_cache, GPUVertexDeclaration* in_vertex_declaration, GPURenderMaterial* in_render_material, size_t in_vertex_requirement_evaluation) :
        dynamic_mesh(in_dynamic_mesh),
        buffer_cache(in_buffer_cache),
        vertex_declaration(in_vertex_declaration),
        render_material(in_render_material),
        vertex_requirement_evaluation(in_vertex_requirement_evaluation)
    {
        assert(in_dynamic_mesh != nullptr);
    }

    void PrimitiveOutputBase::SetRenderMaterial(GPURenderMaterial* in_render_material)
    {
        assert(0);

        // TODO


    }

    void PrimitiveOutputBase::Flush()
    {
        // skip empty primitive
        if (buffer_start == nullptr || buffer_position == buffer_start)
            return;

        // get the manager for resources
        GPUResourceManager* gpu_resource_manager = nullptr;
        
        if (type == PrimitiveType::QUAD)
        {
            gpu_resource_manager = WindowApplication::GetGPUResourceManagerInstance();
            if (gpu_resource_manager == nullptr)
                return;
        }

        // one new GPU buffer each time so primitives need to be flushed => one GPUDynamicMeshElement each time
        GPUDynamicMeshElement& new_element = dynamic_mesh->AddMeshElement();

        new_element.vertex_buffer = vertex_buffer;
        new_element.vertex_declaration = vertex_declaration;        
        new_element.render_material = render_material;

        if (type == PrimitiveType::QUAD)
        {
            size_t quad_rendering_count = 0;
            new_element.index_buffer = (type == PrimitiveType::QUAD) ? gpu_resource_manager->GetQuadIndexBuffer(&quad_rendering_count) : nullptr;

            size_t quad_count = ((buffer_position - buffer_start) / vertex_size) / 4;

            size_t base_vertex_index = 0;
            while (quad_count > 0)
            {
                size_t count = std::min(quad_count, quad_rendering_count);

                // insert the primitive into the element
                GPUDrawPrimitive primitive;
                primitive.primitive_type = primitive_gl_type;
                primitive.indexed = true;
                primitive.count = (int)(6 * count); // 6 vertices per quad
                primitive.start = 0;
                primitive.base_vertex_index = (int)base_vertex_index;
                new_element.primitives.push_back(primitive);

                // end of loop
                if (quad_count <= quad_rendering_count)
                    break;
                base_vertex_index += 4 * count;
                quad_count -= quad_rendering_count;
            }
        }
        else
        {
            // insert the primitive into the element
            GPUDrawPrimitive primitive;
            primitive.primitive_type = primitive_gl_type;
            primitive.indexed = false;
            primitive.count = (int)((buffer_position - buffer_start) / vertex_size);
            primitive.start = 0;
            primitive.base_vertex_index = 0;
            new_element.primitives.push_back(primitive);
        }

        // clear internals
        vertex_buffer->UnMapBuffer();
        vertex_buffer->SetBufferData(nullptr, new_element.vertex_buffer->GetBufferSize()); // orphan the buffer

        buffer_start = nullptr;
        buffer_position = nullptr;
        buffer_end = nullptr;
        vertex_buffer = nullptr;
    }

    char* PrimitiveOutputBase::GeneratePrimitive(size_t required_size)
    {
        // flush primitive and allocate memory if necessary
        if (buffer_start == nullptr || buffer_position + required_size >= buffer_end)
        {
            if (buffer_start != nullptr)
                Flush();
            if (ReserveBuffer(required_size) == nullptr)
                return nullptr;
        }
        // make buffer progress and update pending_vertices_count
        char* result = buffer_position;
        buffer_position += required_size;
        return result;
    }

    char* PrimitiveOutputBase::ReserveBuffer(size_t required_size)
    {
        // compute/evaluate number of vertices to use for the buffer
        size_t count = 0;        
        if (vertex_requirement_evaluation != 0)
            count = std::max(vertex_requirement_evaluation, required_size / vertex_size);
        else
            count = std::max(30 * real_vertices_per_primitive, required_size / vertex_size); // a default buffer of 30 primitives
        // compute buffer size and allocate a buffer
        size_t bufsize = count * vertex_size;         

        if (buffer_cache == nullptr)
        {
            if (!GPUBufferCache::CreateBuffer(bufsize, vertex_buffer))
                return nullptr;
        } 
        else
        {
            if (!buffer_cache->GetBuffer(bufsize, vertex_buffer))
                return nullptr;
        }
        
        // map the buffer

#if 0 // only map required memory
        buffer_start = vertex_buffer->MapBuffer(0, bufsize, false, true);
        buffer_end = buffer_start + bufsize;
#else // map whole buffer
        buffer_start = vertex_buffer->MapBuffer(0, 0, false, true);
        buffer_end = buffer_start + vertex_buffer->GetBufferSize();
#endif
        buffer_position = buffer_start;

        return buffer_position;
    }

}; // namespace chaos
