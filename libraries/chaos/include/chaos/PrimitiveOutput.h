#pragma once

#include <chaos/StandardHeaders.h>

#include <chaos/GPUClasses.h>
#include <chaos/GPUDrawPrimitive.h>
#include <chaos/GPUBufferCache.h>

namespace chaos
{
    /**
     * PrimitiveType : the type of primitives that can be rendered
     */

    enum class PrimitiveType
    {
        triangle = 0,
        triangle_pair = 1,
        quad = 2,
        triangle_strip = 3,
        triangle_fan = 4,
    };

    /** returns the number of element per primitive */
    constexpr size_t GetVerticesPerParticle(PrimitiveType primitive_type)
    {
        if (primitive_type == PrimitiveType::triangle)
            return 3;
        if (primitive_type == PrimitiveType::triangle_pair)
            return 6;
        if (primitive_type == PrimitiveType::quad)
            return 4;
        return 0; // strip and fans have no defined values for this
    }

    constexpr GLenum GetGLPrimitiveType(PrimitiveType primitive_type)
    {
        if (primitive_type == PrimitiveType::triangle)
            return GL_TRIANGLES;
        if (primitive_type == PrimitiveType::triangle_pair)
            return GL_TRIANGLES;
        if (primitive_type == PrimitiveType::quad)
            return GL_QUADS;
        if (primitive_type == PrimitiveType::triangle_strip)
            return GL_TRIANGLE_STRIP;
        if (primitive_type == PrimitiveType::triangle_fan)
            return GL_TRIANGLE_FAN;
        return GL_NONE;
    }

    /**
     * PrimitiveBase : base object for writing GPU primitives into memory (GPU mapped memory for the usage) 
     */

    template<size_t VERTICES_COUNT = 0>
    class PrimitiveBase
    {
    public:

        /** base constructor */
        PrimitiveBase() = default;
        /** copy constructor */
        PrimitiveBase(PrimitiveBase const& src) = default;
        /** initialization constructor */
        PrimitiveBase(char* in_buffer, size_t in_vertex_size, size_t in_vertices_count) :
            buffer(in_buffer),
            vertex_size(in_vertex_size),
            vertices_count(in_vertices_count)
        {
            assert(in_buffer != nullptr);
            assert(in_vertex_size > 0);
            // xxx : do not test for vertices_count. At this point 0, is a valid value for Fans and Strips
        }

        /** gets the buffer for this primitive */
        char* GetBuffer() const { return buffer; }
        /** gets the size of one vertice of this primitive */
        size_t GetVertexSize() const { return vertex_size; }
        /** gets the number of vertices for this primitive */
        size_t GetVerticesCount() const { return vertices_count; }

    protected:

        /** the buffer where we write buffer */
        char* buffer = nullptr;
        /** the size of a vertex for this primitive */
        size_t vertex_size = 0;
        /** the number of vertices for this primitive */
        size_t vertices_count = VERTICES_COUNT;
    };

    /**
      * TypedPrimitiveBase : base object for writing GPU primitives into memory (vertex type is known)
      */

    template<typename VERTEX_TYPE, size_t VERTICES_COUNT = 0>
    class TypedPrimitiveBase : public PrimitiveBase<VERTICES_COUNT>
    {
    public:

        using vertex_type = VERTEX_TYPE;

        /** base constructor */
        TypedPrimitiveBase() = default;
        /** copy constructor */
        TypedPrimitiveBase(TypedPrimitiveBase const & src) = default;
        /** downcast constructor */
        template<typename OTHER_VERTEX_TYPE>
        TypedPrimitiveBase(TypedPrimitiveBase<OTHER_VERTEX_TYPE> const& src, std::enable_if_t<std::is_base_of_v<VERTEX_TYPE, OTHER_VERTEX_TYPE>, int> = 0) :
            TypedPrimitiveBase(src.GetBuffer(), src.GetVertexSize(), src.GetVerticesCount()) {}
        /** constructor */
        TypedPrimitiveBase(char* in_buffer, size_t in_vertex_size, size_t in_vertices_count) :
            PrimitiveBase(in_buffer, in_vertex_size, in_vertices_count) {}

        /** accessor */
        vertex_type & operator [](size_t index)
        {
            assert(index < vertices_count);
            return *((vertex_type*)(buffer + vertex_size * index));
        }

        /** const accessor */
        vertex_type const & operator [](size_t index) const
        {
            assert(index < vertices_count);
            return *((vertex_type const*)(buffer + vertex_size * index));
        }
    };

    /**
      * The usual existing primitives
      */

    // fixed length primitives
    template<typename VERTEX_TYPE> using TrianglePrimitive = TypedPrimitiveBase<VERTEX_TYPE, 3>;
    template<typename VERTEX_TYPE> using TrianglePairPrimitive = TypedPrimitiveBase<VERTEX_TYPE, 6>;
    template<typename VERTEX_TYPE> using QuadPrimitive = TypedPrimitiveBase<VERTEX_TYPE, 4>;
    // 0 for non-fixed vertices count
    template<typename VERTEX_TYPE> using TriangleStripPrimitive = TypedPrimitiveBase<VERTEX_TYPE, 0>;
    template<typename VERTEX_TYPE> using TriangleFanPrimitive = TypedPrimitiveBase<VERTEX_TYPE, 0>;   

    /**
     * PrimitiveOutputBase : a primitive generator (the base class)
     */

    class PrimitiveOutputBase
    {

    public:

        /** constructor */
        PrimitiveOutputBase(GPUDynamicMesh * in_dynamic_mesh, GPUBufferCache * in_buffer_cache, GPUVertexDeclaration const * in_vertex_declaration, GPURenderer* in_renderer, size_t in_vertex_requirement_evaluation) :
            dynamic_mesh(in_dynamic_mesh),
            buffer_cache(in_buffer_cache),
            vertex_declaration(in_vertex_declaration),
            renderer(in_renderer),
            vertex_requirement_evaluation(in_vertex_requirement_evaluation)
        {
            assert(in_dynamic_mesh != nullptr);
            assert(in_renderer != nullptr);
        }

        /** gets the size of one vertice of the generated primitive */
        size_t GetVertexSize() const { return vertex_size; }
        /** flush all pending Draws into the GPUDynamicMesh */
        void Flush();

    protected:

        /** register a new primitive */
        char* GeneratePrimitive(size_t required_size);
        /** allocate a buffer for the primitive and register a new primitive */
        char* ReserveBuffer(size_t required_size);

    protected:

        /** the dynamic mesh we are working on (to store primitives to render) */
        GPUDynamicMesh * dynamic_mesh = nullptr;
        /** a buffer cache */
        GPUBufferCache* buffer_cache = nullptr;
        /** the vertex declaration for all buffers */
        GPUVertexDeclaration const* vertex_declaration = nullptr;
        /** the renderer used fence requests */
        GPURenderer* renderer = nullptr;
        /** the buffer where we are writting vertices */
        GPUBufferCacheEntry cached_buffer;

        /** an evaluation of how many vertices could be used */
        size_t vertex_requirement_evaluation = 0;
       
        /** start of currently allocated buffer */
        char* buffer_start = nullptr;
        /** end of currently allocated buffer */
        char* buffer_end = nullptr;
        /** number of vertices inserted into the GPU buffer and waiting for a flush */
        size_t pending_vertices_count = 0;

        /** size of a vertex */
        size_t vertex_size = 0;
        /** the number of vertices per primitive */
        size_t vertices_per_primitive = 0;
        /** the primitive type */
        GLenum primitive_gl_type = GL_NONE;
    };

    /**
     * TypedPrimitiveOutputBase : generic primitive generator
     */

    template<typename VERTEX_TYPE, size_t VERTICES_PER_PRIMITIVE, GLenum GL_PRIMITIVE_TYPE> // PRIMITIVE_VERTICES_COUNT : should be 0 for STRIPS & FANS
    class TypedPrimitiveOutputBase : public PrimitiveOutputBase
    {
    public:

        using vertex_type = VERTEX_TYPE;

        using primitive_type = TypedPrimitiveBase<vertex_type, VERTICES_PER_PRIMITIVE>;

        /** constructor */
        TypedPrimitiveOutputBase(GPUDynamicMesh* in_dynamic_mesh, GPUBufferCache* in_buffer_cache, GPUVertexDeclaration const* in_vertex_declaration, GPURenderer* in_renderer, size_t in_vertex_requirement_evaluation) :
            PrimitiveOutputBase(in_dynamic_mesh, in_buffer_cache, in_vertex_declaration, in_renderer, in_vertex_requirement_evaluation)
        {
            vertex_size = sizeof(vertex_type);
            vertices_per_primitive = VERTICES_PER_PRIMITIVE;
            primitive_gl_type = GL_PRIMITIVE_TYPE;
        }

        /** add a primitive */
        primitive_type AddPrimitive(size_t custom_vertices_count = 0)
        {
            assert((vertices_per_primitive == 0) ^ (custom_vertices_count == 0)); // STRIPS & FANS require a CUSTOM number of vertices, other requires a NON CUSTOM number of vertices

            // implementation for STRIPS or FANS
            if constexpr (VERTICES_PER_PRIMITIVE == 0)
            {
                // TODO : implement fans and strips 
                primitive_type result;                
                assert(0);
                return result;
            }
            // implementation for fixed length primitives
            else
            {
                return primitive_type(
                    GeneratePrimitive(vertex_size * vertices_per_primitive),
                    vertex_size,
                    vertices_per_primitive
                );
            }
        }
    };

    /**
      * The usual existing primitives output's
      */

    // fixed length primitive
    template<typename VERTEX_TYPE> using TriangleOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, 3, GL_TRIANGLES>;
    template<typename VERTEX_TYPE> using TrianglePairOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, 6, GL_TRIANGLES>;
    template<typename VERTEX_TYPE> using QuadOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, 4, GL_QUADS>;
    // 0 for non-fixed vertices count
    template<typename VERTEX_TYPE> using TriangleFanOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, 0, GL_TRIANGLE_FAN>;
    template<typename VERTEX_TYPE> using TriangleStripOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, 0, GL_TRIANGLE_STRIP>;


}; // namespace chaos

