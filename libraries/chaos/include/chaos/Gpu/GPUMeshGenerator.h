namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class GPUMeshGenerationRequirement;
	class GPUMeshGenerator;

	template<typename T>
	class GPUPrimitiveMeshGenerator;

	class GPUQuadMeshGenerator;
	class GPUTriangleMeshGenerator;
	class GPUBoxMeshGenerator;
	class GPUWireframeBoxMeshGenerator;
	class GPUCircleMeshGenerator;
	class GPUSphereMeshGenerator;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* A class to describe requirement for a mesh
	*/

	class CHAOS_API GPUMeshGenerationRequirement
	{
	public:

		/** test whether the requirement is valid */
		bool IsValid() const;

	public:

		/** size of a vertex */
		int vertex_size = 0;
		/** number of vertices required */
		int vertices_count = 0;
		/** number of indices required */
		int indices_count = 0;
	};

	/**
	* GPUMeshGenerator : an object that is responsible for generating the mesh data
	*/

	class CHAOS_API GPUMeshGenerator : public Object
	{
	public:

		/** the destructor */
		virtual ~GPUMeshGenerator() = default;

		/** get requirement */
		virtual GPUMeshGenerationRequirement GetRequirement() const = 0;
		/** get the vertex declaration */
		virtual GPUVertexDeclaration* GenerateVertexDeclaration() const = 0;
		/** get the mesh data */
		virtual void GenerateMeshData(std::vector<GPUDrawPrimitive>& primitives, MemoryBufferWriter& vertices_writer, MemoryBufferWriter& indices_writer) const = 0;

		/** generation function */
		shared_ptr<GPUMesh> GenerateMesh() const;
		/** population function */
		bool FillMeshData(GPUMesh* mesh) const;
	};

	/**
	* GPUPrimitiveMeshGenerator : a templated base class for primitives
	*/

	template<typename T>
	class GPUPrimitiveMeshGenerator : public GPUMeshGenerator
	{
	public:

		using primitive_type = T;

		/** constructor */
		GPUPrimitiveMeshGenerator(primitive_type const& in_primitive, glm::mat4x4 const& in_transform = glm::mat4x4(1.0f)) :
			primitive(in_primitive),
			transform(in_transform) {}

	protected:

		/** the primitive that is been generated */
		primitive_type primitive;
		/** the transformation to apply to vertices */
		glm::mat4x4 transform;
	};

	/**
	* GPUQuadMeshGenerator : help defines mesh as simple quad
	*/

	class CHAOS_API GPUQuadMeshGenerator : public GPUPrimitiveMeshGenerator<box2>
	{

	public:

		using GPUPrimitiveMeshGenerator::GPUPrimitiveMeshGenerator;

		/** get requirement */
		virtual GPUMeshGenerationRequirement GetRequirement() const override;
		/** get the vertex declaration */
		virtual GPUVertexDeclaration* GenerateVertexDeclaration() const override;
		/** get the mesh data */
		virtual void GenerateMeshData(std::vector<GPUDrawPrimitive>& primitives, MemoryBufferWriter& vertices_writer, MemoryBufferWriter& indices_writer) const override;

	protected:

		/** the vertices defining a face facing planes inside [-1, +1] */
		static glm::vec2 const vertices[4];
		/** the triangles indices defining a face facing planes */
		static GLuint const triangles[6];
	};

	/**
	* GPUTriangleMeshGenerator : help defines mesh as simple traingle
	*/

	class CHAOS_API GPUTriangleMeshGenerator : public GPUPrimitiveMeshGenerator<triangle3>
	{

	public:

		using GPUPrimitiveMeshGenerator::GPUPrimitiveMeshGenerator;

		/** get requirement */
		virtual GPUMeshGenerationRequirement GetRequirement() const override;
		/** get the vertex declaration */
		virtual GPUVertexDeclaration* GenerateVertexDeclaration() const override;
		/** get the mesh data */
		virtual void GenerateMeshData(std::vector<GPUDrawPrimitive>& primitives, MemoryBufferWriter& vertices_writer, MemoryBufferWriter& indices_writer) const override;
	};

	/**
	* GPUBoxMeshGenerator : help defines cube mesh
	*/

	class CHAOS_API GPUBoxMeshGenerator : public GPUPrimitiveMeshGenerator<box3>
	{

	public:

		using GPUPrimitiveMeshGenerator::GPUPrimitiveMeshGenerator;

		/** get requirement */
		virtual GPUMeshGenerationRequirement GetRequirement() const override;
		/** get the vertex declaration */
		virtual GPUVertexDeclaration* GenerateVertexDeclaration() const override;
		/** get the mesh data */
		virtual void GenerateMeshData(std::vector<GPUDrawPrimitive>& primitives, MemoryBufferWriter& vertices_writer, MemoryBufferWriter& indices_writer) const override;

	protected:

		/** the vertices defining a cube */
		static glm::vec3 const vertices[24 * 2];
		/** the triangles defining a cube */
		static GLuint const triangles[36];
	};

	/**
	* GPUWireframeBoxMeshGenerator : help defines cube mesh
	*/

	class CHAOS_API GPUWireframeBoxMeshGenerator : public GPUPrimitiveMeshGenerator<box3>
	{

	public:

		using GPUPrimitiveMeshGenerator::GPUPrimitiveMeshGenerator;

		/** get requirement */
		virtual GPUMeshGenerationRequirement GetRequirement() const override;
		/** get the vertex declaration */
		virtual GPUVertexDeclaration* GenerateVertexDeclaration() const override;
		/** get the mesh data */
		virtual void GenerateMeshData(std::vector<GPUDrawPrimitive>& primitives, MemoryBufferWriter& vertices_writer, MemoryBufferWriter& indices_writer) const override;

	protected:

		/** the vertices defining a cube */
		static glm::vec3 const vertices[8];
		/** the indices defining a cube */
		static GLuint const indices[24];
	};

	/**
	* GPUCircleMeshGenerator : help defines mesh as simple 2D circle
	*/

	class CHAOS_API GPUCircleMeshGenerator : public GPUPrimitiveMeshGenerator<sphere2>
	{

	public:

		/** constructor */
		GPUCircleMeshGenerator(sphere2 const& in_primitive, glm::mat4x4 const& in_transform = glm::mat4x4(1.0f), int in_subdivisions = 10) :
			GPUPrimitiveMeshGenerator<sphere2>(in_primitive, in_transform),
			subdivisions(in_subdivisions) {}

		/** get requirement */
		virtual GPUMeshGenerationRequirement GetRequirement() const override;
		/** get the vertex declaration */
		virtual GPUVertexDeclaration* GenerateVertexDeclaration() const override;
		/** get the mesh data */
		virtual void GenerateMeshData(std::vector<GPUDrawPrimitive>& primitives, MemoryBufferWriter& vertices_writer, MemoryBufferWriter& indices_writer) const override;

	protected:

		/** number of subdivisions */
		int subdivisions = 10;
	};


	/**
	* GPUSphereMeshGenerator : help defines mesh as simple sphere
	*/

	class CHAOS_API GPUSphereMeshGenerator : public GPUPrimitiveMeshGenerator<sphere3>
	{

	public:

		/** constructor */
		GPUSphereMeshGenerator(sphere3 const& in_primitive, glm::mat4x4 const& in_transform = glm::mat4x4(1.0f), int in_subdivisions = 10) :
			GPUPrimitiveMeshGenerator<sphere3>(in_primitive, in_transform),
			subdivisions(in_subdivisions) {}

		/** get requirement */
		virtual GPUMeshGenerationRequirement GetRequirement() const override;
		/** get the vertex declaration */
		virtual GPUVertexDeclaration* GenerateVertexDeclaration() const override;
		/** get the mesh data */
		virtual void GenerateMeshData(std::vector<GPUDrawPrimitive>& primitives, MemoryBufferWriter& vertices_writer, MemoryBufferWriter& indices_writer) const override;

	protected:

		/** get a vertex on the sphere from polar angle */
		void InsertVertex(MemoryBufferWriter& vertices_writer, float alpha, float beta) const;

	protected:

		/** number of subdivisions */
		int subdivisions = 10;
	};

#endif

}; // namespace chaos