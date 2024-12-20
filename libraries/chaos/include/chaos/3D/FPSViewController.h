namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class FPSViewControllerConfiguration;
	class FPSViewControllerInputConfiguration;
	class FPSViewController;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* FPSViewControllerConfiguration : a class that describes displacement speed for the FPS view
	*/

	class CHAOS_API FPSViewControllerConfiguration
	{
	public:

		/** mouse speed */
		float mouse_sensibility = 1.0f;

		/** some self descriptive speed */
		float strafe_speed = 20.0f;
		/** some self descriptive speed */
		float forward_speed = 20.0f;
		/** some self descriptive speed */
		float back_speed = 20.0f;
		/** some self descriptive speed */
		float up_speed = 20.0f;
		/** some self descriptive speed */
		float down_speed = 20.0f;

		/** some self descriptive speed */
		float yaw_speed = 90.0f;
		/** some self descriptive speed */
		float pitch_speed = 90.0f;

		/** whether we need to capture to move the camera */
		bool must_click_to_rotate = true;
	};

	class CHAOS_API FPSViewControllerInputConfiguration
	{
	public:

		/** self descriptive key */
		KeyboardButton left_button = KeyboardButton::LEFT;
		/** self descriptive key */
		KeyboardButton right_button = KeyboardButton::RIGHT;
		/** self descriptive key */
		KeyboardButton forward_button = KeyboardButton::UP;
		/** self descriptive key */
		KeyboardButton backward_button = KeyboardButton::DOWN;
		/** self descriptive key */
		KeyboardButton up_button = KeyboardButton::PAGE_UP;
		/** self descriptive key */
		KeyboardButton down_button = KeyboardButton::PAGE_DOWN;

		/** self descriptive key */
		KeyboardButton yaw_left_button = KeyboardButton::KP_4;
		/** self descriptive key */
		KeyboardButton yaw_right_button = KeyboardButton::KP_6;
		/** self descriptive key */
		KeyboardButton pitch_up_button = KeyboardButton::KP_8;
		/** self descriptive key */
		KeyboardButton pitch_down_button = KeyboardButton::KP_2;

		/** self descriptive key */
		MouseButton rotation_button = MouseButton::BUTTON_1;
	};

	/**
	* FPSViewController : an utility class to simply handle a FPS camera in a GLFW application. Handle keys ...
	*/

	class CHAOS_API FPSViewController
	{
		static double constexpr INVALID_MOUSE_VALUE = std::numeric_limits<double>::max();

	public:

		/** the tick method */
		virtual void Tick(GLFWwindow* glfw_window, float delta_time);

		/** matrix getter */
		inline glm::mat4 GlobalToLocal() const { return fps_view.GlobalToLocal(); }
		/** matrix getter */
		inline glm::mat4 LocalToGlobal() const { return fps_view.LocalToGlobal(); }

		/** returns true whether mouse capture is enabled */
		bool IsMouseEnabled() const { return mouse_enabled; }
		/** change the mouse capture policy */
		void SetMouseEnabled(bool in_mouse_enabled);

	protected:

		/** handle the mouse displacement */
		void HandleMouseInputs(GLFWwindow* glfw_window, float delta_time);
		/** handle the keyboard inputs */
		void HandleKeyboardInputs(float delta_time);

		/** check whether keyboard input is down */
		bool CheckKeyboardInput(KeyboardButton button) const;
		/** check whether mouse input is down */
		bool CheckMouseInput(MouseButton button) const;

	protected:

		/** whether the mouse is enabled or not */
		bool mouse_enabled = true;
		/** whether the mouse has been captured */
		bool mouse_captured = false;
		/** position of the mouse once captured */
		double previous_mouse_x = INVALID_MOUSE_VALUE;
		/** position of the mouse once captured */
		double previous_mouse_y = INVALID_MOUSE_VALUE;

	public:

		/** the fps matrix handler */
		FPSView fps_view;
		/** the speed for the displacement */
		FPSViewControllerConfiguration config;
		/** the mapping for the displacement */
		FPSViewControllerInputConfiguration input_config;
	};

#endif

}; // namespace chaos