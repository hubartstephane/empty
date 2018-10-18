#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/NamedObject.h>
#include <chaos/ReferencedObject.h>
#include <chaos/DrawPrimitive.h>
#include <chaos/GPUProgramProvider.h>
#include <chaos/GPURenderMaterial.h>

namespace chaos
{
	class MaterialProvider;
	class RenderParams;
	class Renderable;


	// ========================================================
	// RenderParams : some data for the rendering
	// ========================================================

	class RenderParams
	{

	public:

		/** get the material */
		GPURenderMaterial const * GetMaterial(Renderable const * renderable, GPURenderMaterial const * default_material) const;

	public:

		/** material provider */
		boost::intrusive_ptr<MaterialProvider> material_provider;
		/** the instancing information */
		InstancingInfo instancing;
	};

	// ========================================================
	// MaterialProvider : used to override the material used
	// ========================================================	

	class MaterialProvider : public ReferencedObject
	{
	public:

		/** the material provider main method */
		virtual GPURenderMaterial const * GetMaterial(Renderable const * renderable, GPURenderMaterial const * default_material, RenderParams const & render_params = RenderParams()) const;
	};	


	// ========================================================
	// Renderable : base class for all object that can be rendered
	// ========================================================

	class Renderable : public virtual ReferencedObject, public virtual NamedObject
	{
	public:

		/** public method to render the object */
		int Display(GPUProgramProviderBase const * uniform_provider, RenderParams const & render_params = RenderParams()) const;

		/** show or hide the object */
		void Show(bool in_visible = true);
		/** returns whether the object is visible or not */
		bool IsVisible() const;

	protected:

		/** the user defined method to display the object */
		virtual int DoDisplay(GPUProgramProviderBase const * uniform_provider, RenderParams const & render_params) const;
		/** called whenever object visibility has been changed */
		virtual void OnVisibilityChanged(bool in_visible);

	public:

		/** whether the object is hidden or visible */
		bool visible = true;
	};

	// ========================================================
	// RenderableLayer : used as a sorted container for renderers
	// ========================================================

	class RenderableLayer : public Renderable
	{
	protected:

		/** an utility class to store a a Renderable with a render order */
		class RenderableLayerInfo : public NamedObjectWrapper<Renderable>
		{	
		public:

			/** special method for sorted insertion : lower_bound + insert */
			operator int() const { return render_order; }
			/** the render order */
			int render_order = 0;
		};

	public:

		/** Find a renderable by its name */
		Renderable * FindChildRenderable(char const * name);
		/** Find a renderable by its name */
		Renderable const * FindChildRenderable(char const * name) const;
		/** Find a renderable by its tag */
		Renderable * FindChildRenderable(TagType tag);
		/** Find a renderable by its tag */
		Renderable const * FindChildRenderable(TagType tag) const;
		/** insert a renderable */
		bool AddChildRenderable(Renderable * renderable, int render_order);
		/** remove a renderable */
		bool RemoveChildRenderable(Renderable * renderable);

	protected:

		/** the main rendering method */
		virtual int DoDisplay(GPUProgramProviderBase const * uniform_provider, RenderParams const & render_params) const override;
		/** find a renderable */
		RenderableLayerInfo * FindChildRenderable(Renderable * renderable);
		/** find a renderable */
		RenderableLayerInfo const * FindChildRenderable(Renderable * renderable) const;

	protected:

		/** all child renderable */
		std::vector<RenderableLayerInfo> layers;
	};

}; // namespace chaos
