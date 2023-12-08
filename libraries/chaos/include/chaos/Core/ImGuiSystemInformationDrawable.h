namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class ImGuiSystemInformationDrawable;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* ImGuiSystemInformationDrawable: a drawable that displays some system informations
	*/

	class ImGuiSystemInformationDrawable : public ImGuiDrawableObject
	{
	public:

		CHAOS_DECLARE_OBJECT_CLASS(ImGuiSystemInformationDrawable, ImGuiDrawableObject);

		/** override */
		virtual void DrawImGui(ImGuiDrawMenuMode menu_mode) override;
	};

#endif

}; // namespace chaos