namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class ViewportLayout;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION


#if 0

	/**
	 * The layout is responsible for computing the surface to reserve for all viewports owned by a ViewportWindow
	 */

	class CHAOS_API ViewportLayout : public Object
	{
		CHAOS_DECLARE_OBJECT_CLASS(ViewportLayout, Object);

		friend class ViewportServerWindow;
		friend class ViewportComposer;

	public:

		/** compute placement for all viewports in the list */
		virtual void SetAllViewportPlacements(aabox2 const& in_placement, std::vector<shared_ptr<Viewport>>& viewports);

		/** gets the composer using the layout */
		AutoCastable<ViewportComposer> GetViewportComposer() { return viewport_composer; }
		/** gets the composer using the layout */
		AutoConstCastable<ViewportComposer> GetViewportComposer() const { return viewport_composer; }

		/** Update the window placements */
		void UpdatePlacementHierarchy();

	protected:

		/** the viewport owning this layout */
		ViewportComposer* viewport_composer = nullptr;
	};

#endif

#endif

}; // namespace chaos