#pragma once

#include <chaos/StandardHeaders.h>

#include <chaos/GPUClasses.h>
#include <chaos/GPUDrawPrimitive.h>
#include <chaos/GPUBufferCache.h>

#define CHAOS_INDEXED_QUAD_RENDERING 1

namespace chaos
{
    // QUAD
    // ----
    //     3      2
    //     +------+
    //     |      |
    //     |      |
    //     +------+
    //     0      1
    // 
    // TRIANGLE PAIR
    // -------------
    //
    //     5     2,4
    //     +------+
    //     |    / |
    //     | /    |
    //     +------+
    //    0,3     1
    // 

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

    /** returns the number of element per primitive (user accessible) */
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

    /** returns the real number of element per primitive (the count in GPU buffer) */
    constexpr size_t GetRealVerticesPerParticle(PrimitiveType primitive_type)
    {
#if !CHAOS_INDEXED_QUAD_RENDERING
        if (primitive_type == PrimitiveType::quad)
            return 6;
#endif
        return GetVerticesPerParticle(primitive_type);
    }

    /** returns the OpenGL primitive type corresponding to the primitive */
    constexpr GLenum GetGLPrimitiveType(PrimitiveType primitive_type)
    {
        if (primitive_type == PrimitiveType::triangle)
            return GL_TRIANGLES;
        if (primitive_type == PrimitiveType::triangle_pair)
            return GL_TRIANGLES;
        if (primitive_type == PrimitiveType::quad)
            return GL_TRIANGLES;
        if (primitive_type == PrimitiveType::triangle_strip)
            return GL_TRIANGLE_STRIP;
        if (primitive_type == PrimitiveType::triangle_fan)
            return GL_TRIANGLE_FAN;
        return GL_NONE;
    }

    /**
     * PrimitiveBase : base object for writing GPU primitives into memory (GPU mapped memory for the usage) 
     */

    template<typename VERTEX_TYPE, PrimitiveType PRIMITIVE_TYPE>
    class PrimitiveBase
    {
    public:

        using vertex_type = VERTEX_TYPE;

        /** base constructor */
        inline PrimitiveBase() = default;
        /** copy constructor */
        inline PrimitiveBase(PrimitiveBase const & src) = default;
        /** constructor */
        inline PrimitiveBase(char* in_buffer, size_t in_vertex_size) :
            buffer(in_buffer), 
            vertex_size(in_vertex_size) 
        {
            assert(in_buffer != nullptr);
            assert(in_vertex_size > 0);
        }
        /** destructor */
        inline ~PrimitiveBase()
        {
            // Quad is internally considered as a triangle pair. Duplicate vertices
#if !CHAOS_INDEXED_QUAD_RENDERING
            if constexpr (PRIMITIVE_TYPE == PrimitiveType::quad)
            {
                size_t offset2 = vertex_size * 2;
                size_t offset4 = vertex_size * 4;
                size_t offset5 = vertex_size * 5;

                std::memcpy(buffer + offset4, buffer, vertex_size);
                std::memcpy(buffer + offset5, buffer + offset2, vertex_size);
            }
#endif
        }

        /** cast operator to child vertex type */
        template<typename OTHER_VERTEX_TYPE> 
        operator PrimitiveBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE>& () 
        {
            static_assert(std::is_base_of_v<OTHER_VERTEX_TYPE, VERTEX_TYPE>);
            return *(PrimitiveBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE>*)this;
        }
        /** cast operator to child vertex type */
        template<typename OTHER_VERTEX_TYPE>
        operator PrimitiveBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE> const & () const
        {
            static_assert(std::is_base_of_v<OTHER_VERTEX_TYPE, VERTEX_TYPE>);
            return *(PrimitiveBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE>*)this;
        }

        /** accessor */
        inline vertex_type & operator [](size_t index)
        {
            assert(index < chaos::GetVerticesPerParticle(PRIMITIVE_TYPE));
            return *((vertex_type*)(buffer + vertex_size * index));
        }
        /** const accessor */
        inline vertex_type const & operator [](size_t index) const
        {
            assert(index < chaos::GetVerticesPerParticle(PRIMITIVE_TYPE));
            return *((vertex_type const*)(buffer + vertex_size * index));
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
    };

    /**
      * The usual existing primitives
      */

    // fixed length primitives
    template<typename VERTEX_TYPE> using TrianglePrimitive = PrimitiveBase<VERTEX_TYPE, PrimitiveType::triangle>;
    template<typename VERTEX_TYPE> using TrianglePairPrimitive = PrimitiveBase<VERTEX_TYPE, PrimitiveType::triangle_pair>;
    template<typename VERTEX_TYPE> using QuadPrimitive = PrimitiveBase<VERTEX_TYPE, PrimitiveType::quad>;
    // non-fixed length vertices count
    template<typename VERTEX_TYPE> using TriangleStripPrimitive = PrimitiveBase<VERTEX_TYPE, PrimitiveType::triangle_strip>;
    template<typename VERTEX_TYPE> using TriangleFanPrimitive = PrimitiveBase<VERTEX_TYPE, PrimitiveType::triangle_fan>;

    /**
     * PrimitiveOutputBase : a primitive generator (the base class)
     */

    class PrimitiveOutputBase
    {
    public:

        /** constructor */
        PrimitiveOutputBase(GPUDynamicMesh * in_dynamic_mesh, GPUBufferCache * in_buffer_cache, GPUVertexDeclaration * in_vertex_declaration, GPURenderer* in_renderer, size_t in_vertex_requirement_evaluation) :
            dynamic_mesh(in_dynamic_mesh),
            buffer_cache(in_buffer_cache),
            vertex_declaration(in_vertex_declaration),
            renderer(in_renderer),
            vertex_requirement_evaluation(in_vertex_requirement_evaluation)
        {
            assert(in_buffer_cache != nullptr);
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
        GPUVertexDeclaration * vertex_declaration = nullptr;
        /** the renderer used fence requests */
        GPURenderer* renderer = nullptr;
        /** the buffer where we are writting vertices */
        shared_ptr<GPUBuffer> vertex_buffer = nullptr;

        /** an evaluation of how many vertices could be used */
        size_t vertex_requirement_evaluation = 0;
       
        /** the current position for writing */
        char* buffer_position = nullptr;
        /** start of currently allocated buffer */
        char* buffer_start = nullptr;
        /** end of currently allocated buffer */
        char* buffer_end = nullptr;

        /** size of a vertex */
        size_t vertex_size = 0;
        /** the number of vertices per primitive (accessible for user from Primitive with [] operator) */
        size_t vertices_per_primitive = 0;
        /** the real number of vertices per primitives (in GPU memory quads are transformed into a triangle pair) */
        size_t real_vertices_per_primitive = 0;
        /** the primitive type */
        PrimitiveType type;
        /** the GL primitive type */
        GLenum primitive_gl_type = GL_NONE;
    };

    /**
     * TypedPrimitiveOutputBase : generic primitive generator
     */

    template<typename VERTEX_TYPE, PrimitiveType PRIMITIVE_TYPE> // PRIMITIVE_VERTICES_COUNT : should be 0 for STRIPS & FANS
    class TypedPrimitiveOutputBase : public PrimitiveOutputBase
    {
    public:

        using vertex_type = VERTEX_TYPE;

        using primitive_type = PrimitiveBase<vertex_type, PRIMITIVE_TYPE>;

        /** constructor */
        TypedPrimitiveOutputBase(GPUDynamicMesh* in_dynamic_mesh, GPUBufferCache* in_buffer_cache, GPUVertexDeclaration * in_vertex_declaration, GPURenderer* in_renderer, size_t in_vertex_requirement_evaluation) :
            PrimitiveOutputBase(in_dynamic_mesh, in_buffer_cache, in_vertex_declaration, in_renderer, in_vertex_requirement_evaluation)
        {
            vertex_size = sizeof(vertex_type);
            vertices_per_primitive = chaos::GetVerticesPerParticle(PRIMITIVE_TYPE);
            real_vertices_per_primitive = chaos::GetRealVerticesPerParticle(PRIMITIVE_TYPE);
            type = PRIMITIVE_TYPE;
            primitive_gl_type = chaos::GetGLPrimitiveType(PRIMITIVE_TYPE);
        }

        /** cast operator to child vertex type */
        template<typename OTHER_VERTEX_TYPE>
        operator TypedPrimitiveOutputBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE>& ()
        {
            static_assert(std::is_base_of_v<OTHER_VERTEX_TYPE, VERTEX_TYPE>);
            return *(TypedPrimitiveOutputBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE>*)this;
        }
        /** cast operator to child vertex type */
        template<typename OTHER_VERTEX_TYPE>
        operator TypedPrimitiveOutputBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE> const& () const
        {
            static_assert(std::is_base_of_v<OTHER_VERTEX_TYPE, VERTEX_TYPE>);
            return *(TypedPrimitiveOutputBase<OTHER_VERTEX_TYPE, PRIMITIVE_TYPE>*)this;
        }

        /** add a primitive */
        inline primitive_type AddPrimitive(size_t custom_vertices_count = 0)
        {
            assert((vertices_per_primitive == 0) ^ (custom_vertices_count == 0)); // STRIPS & FANS require a CUSTOM number of vertices, other requires a NON CUSTOM number of vertices
            // implementation for STRIPS or FANS
            if constexpr (chaos::GetVerticesPerParticle(PRIMITIVE_TYPE) == 0)
            {
                // TODO : implement fans and strips 
                primitive_type result;                
                assert(0);
                return result;
            }
            // implementation for fixed length primitives
            else
            {
                return primitive_type(GeneratePrimitive(vertex_size * real_vertices_per_primitive), vertex_size);
            }
        }
    };

    /**
      * The usual existing primitives output's
      */

    // fixed length primitive
    template<typename VERTEX_TYPE> using TriangleOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, PrimitiveType::triangle>;
    template<typename VERTEX_TYPE> using TrianglePairOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, PrimitiveType::triangle_pair>;
    template<typename VERTEX_TYPE> using QuadOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, PrimitiveType::quad>;
    // non-fixed length vertices count
    template<typename VERTEX_TYPE> using TriangleStripOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, PrimitiveType::triangle_strip>;
    template<typename VERTEX_TYPE> using TriangleFanOutput = TypedPrimitiveOutputBase<VERTEX_TYPE, PrimitiveType::triangle_fan>;
    
}; // namespace chaos
