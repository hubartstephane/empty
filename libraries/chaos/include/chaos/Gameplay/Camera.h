#ifdef CHAOS_FORWARD_DECLARATION

namespace chaos
{
	class CameraTransform;
	class Camera;

}; // namespace chaos

#else

namespace chaos
{

	// =============================================
	// CameraTransform
	// =============================================

	class CameraTransform
	{
	public:

		/** returns the transformation matrix from a camera obox */	
		static glm::mat4x4 GetCameraTransform(obox2 const & obox);
	};

	// =============================================
	// Camera
	// =============================================

	class Camera : public Tickable, public JSONSerializable
	{
		CHAOS_GAMEPLAY_ALLFRIENDS;

		CHAOS_DECLARE_OBJECT_CLASS2(Camera, Tickable);

	public:

		CHAOS_DECLARE_GAMEPLAY_GETTERS();

		/** initialization method */
		bool Initialize(LevelInstance * in_level_instance);

		/** set the camera box */
		void SetCameraBox(box2 const & in_box) { camera_box = in_box;}

		/** get the camera box */
		box2  GetCameraBox(bool apply_modifiers = true) const;
		/** get the camera OBox */
		obox2 GetCameraOBox(bool apply_modifiers = true) const;


		/** get the camera OBox when level instance is started */
		obox2 const & GetInitialCameraOBox() const { return initial_camera_obox;}
		/** set the camera initial OBox */
		void SetInitialCameraOBox(obox2 const & in_obox) { initial_camera_obox = in_obox; }

		/** get the safe zone of the camera */
		glm::vec2 const & GetSafeZone() const { return safe_zone; }
		/** set the safe zone of the camera */
		void SetSafeZone(glm::vec2 const & in_safe_zone) { safe_zone = in_safe_zone; }

		/** the processor may save its configuration into a JSON file */
		virtual bool SerializeIntoJSON(nlohmann::json& json_entry) const override;
		/** the processor may save its configuration from a JSON file */
		virtual bool SerializeFromJSON(nlohmann::json const& json_entry) override;

		/** Camera is a CameraComponent owner */
		CHAOS_DECLARE_COMPONENT_OWNER(CameraComponent, Component, components)

	protected:

		/** override */
		virtual bool DoTick(float delta_time) override;

	protected:

		/** the camera bounding box */
		box2 camera_box;
		/** the initial camera obox (at level startup) */
		obox2 initial_camera_obox;
		/** the safe zone of the camera */
		glm::vec2 safe_zone = glm::vec2(0.8f, 0.8f);

		/** the game instance owning the camera */
		LevelInstance * level_instance = nullptr;

		/** the components */
		std::vector<shared_ptr<CameraComponent>> components;

	};

}; // namespace chaos

#endif // CHAOS_FORWARD_DECLARATION