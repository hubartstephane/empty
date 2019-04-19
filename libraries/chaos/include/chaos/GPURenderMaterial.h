#pragma once

#include <chaos/StandardHeaders.h>

#include <chaos/ReferencedObject.h>
#include <chaos/GPUProgram.h>
#include <chaos/GPUProgramProvider.h>
#include <chaos/GPUFileResource.h>
#include <chaos/Renderable.h>

namespace chaos
{

	/**
	* GPUProgramRenderMaterialProvider : this is a variable provider dedicated for RenderMaterials
	*/

	class GPUProgramRenderMaterialProvider : public GPUProgramProvider
	{
	public:

		/** constructor */
		GPUProgramRenderMaterialProvider(class GPURenderMaterial const * in_render_material, GPUProgramProviderBase const * in_other_provider, RenderParams const * in_render_params) :
			render_material(in_render_material),
			other_provider(in_other_provider),
			render_params(in_render_params)
		{}

	protected:

		/** apply the actions */
		virtual bool DoProcessAction(char const * name, GPUProgramAction & action, GPUProgramProviderBase const * top_provider) const override;

	protected:

		/** the render material as base for the chain */
		GPURenderMaterial const * render_material = nullptr;
		/** another provider (use a non intrusive reference !!!) */
		GPUProgramProviderBase const * other_provider = nullptr;
		/** the render params used */
		RenderParams const * render_params = nullptr;
	};

	/**
	* GPURenderMaterial : this is the combinaison of some uniforms and a program
	*/

	class GPURenderMaterial : public GPUFileResource
	{
		friend class GPUProgramData;
		friend class GPUProgramRenderMaterialProvider;
		friend class GPUResourceManager;
		friend class GPURenderMaterialLoader;
		friend class GPURenderMaterialLoaderReferenceSolver;

	public:

		/** constructor */
		GPURenderMaterial();

		/** destructor */
		virtual ~GPURenderMaterial();

		/** prepare the rendering (find the program, use it, fills its uniforms and returns the program) */
		GPUProgram const * UseMaterial(GPUProgramProviderBase const * in_uniform_provider, RenderParams const & render_params) const;

		/** set the program */
		bool SetProgram(GPUProgram * in_program);
		/** set the parent material */
		bool SetParentMaterial(GPURenderMaterial * in_parent);
		/** set a sub material */
		bool SetSubMaterial(char const * submaterial_name, GPURenderMaterial * submaterial);

		/** go throw the hierary and search for the program */
		GPUProgram const * GetEffectiveProgram(RenderParams const & render_params) const;

		/** get the uniform provider */
		GPUProgramProvider & GetUniformProvider() { return uniform_provider; }
		/** get the uniform provider */
		GPUProgramProvider const & GetUniformProvider() const { return uniform_provider; }


		/** search the submaterial by its submaterial_name */
		GPURenderMaterial * FindSubMaterial(char const * submaterial_name);
		/** search the submaterial by its submaterial_name */
		GPURenderMaterial const * FindSubMaterial(char const * submaterial_name) const;

		/** create a RenderMaterial from a simple program */
		static GPURenderMaterial * GenRenderMaterialObject(GPUProgram * program);

	protected:

		/** cleaning the resource */
		virtual bool DoRelease() override;
		/** search some cycles throught parent_material and sub materials (returning true is an error) */
		bool SearchRenderMaterialCycle(GPURenderMaterial const * searched_material) const;

	protected:

		/** the program */
		shared_ptr<GPUProgram> program;
		/** parent material */
		shared_ptr<GPURenderMaterial> parent_material;
		/** some rendering states */
		GPUProgramProvider uniform_provider;
		/** whether the material is null (force to use no program => no rendering) */
		bool hidden_material = false;
		/** if the array is not empty, only render of the following submaterial names */
		std::vector<std::string> enabled_submaterials;
		/** children materials (pair submaterial_name / material) */
		std::vector<std::pair<std::string, shared_ptr<GPURenderMaterial>>> sub_materials;
	};


}; // namespace chaos
