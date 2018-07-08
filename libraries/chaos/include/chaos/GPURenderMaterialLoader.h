#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/GPUResourceManager.h>
#include <chaos/GPURenderMaterial.h>
#include <chaos/FilePath.h>
#include <chaos/GPUResourceManagerLoader.h>
#include <chaos/EmptyClass.h>


namespace chaos
{

	class GPURenderMaterialLoader : public GPUResourceManagerLoader<GPUFileResourceFriend, GPUResourceManager>
	{
	public:

		/** constructor */
		GPURenderMaterialLoader(GPUResourceManager * in_resource_manager);
		/** destructor */
		virtual ~GPURenderMaterialLoader() = default;

		/** Generate a render material from a json content */
		virtual GPURenderMaterial * GenRenderMaterialObject(nlohmann::json const & json, boost::filesystem::path const & config_path, std::string & parent_name) const;
		/** Generate a render material from an file */
		virtual GPURenderMaterial * GenRenderMaterialObject(FilePathParam const & path, std::string & parent_name) const;

	protected:

		/** initialize a texture from its name */
		bool InitializeTextureFromName(GPURenderMaterial * render_material, char const * uniform_name, char const * texture_name) const;
		/** initialize a texture from its path */
		bool InitializeTextureFromPath(GPURenderMaterial * render_material, char const * uniform_name, FilePathParam const & path) const;


		/** initialize the program from its name */
		bool InitializeProgramFromName(GPURenderMaterial * render_material, char const * program_name) const;
		/** initialize the program from its path */
		bool InitializeProgramFromPath(GPURenderMaterial * render_material, FilePathParam const & path) const;

		/** add a uniform in the render material */
		bool AddUniformToRenderMaterial(GPURenderMaterial * render_material, char const * uniform_name, nlohmann::json const & json) const;

		/** get the program from JSON */
		bool InitializeProgramFromJSON(GPURenderMaterial * render_material, nlohmann::json const & json, boost::filesystem::path const & config_path) const;
		/** get the textures from JSON */
		bool InitializeTexturesFromJSON(GPURenderMaterial * render_material, nlohmann::json const & json, boost::filesystem::path const & config_path) const;
		/** get the uniforms from JSON */
		bool InitializeUniformsFromJSON(GPURenderMaterial * render_material, nlohmann::json const & json, boost::filesystem::path const & config_path) const;

		/** search whether the path is already in used in the manager */
		virtual bool IsPathAlreadyUsedInManager(FilePathParam const & path) const override;
		/** search whether the name is already in used in the manager */
		virtual bool IsNameAlreadyUsedInManager(std::string const & in_name) const override;
	};

}; // namespace chaos
