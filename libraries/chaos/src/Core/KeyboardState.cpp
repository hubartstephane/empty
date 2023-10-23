#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	//
	// KeyboardState
	//

	std::array<ButtonState, GLFW_KEY_LAST> KeyboardState::keyboard_state;

	std::array<ButtonState, GLFW_MOUSE_BUTTON_LAST + 1> KeyboardState::mouse_button_state;

	void KeyboardState::SetKeyboardButtonState(KeyboardButton key, int action)
	{
		int raw_value = int(key);
		if (raw_value >= 0 && raw_value < keyboard_state.size())
			keyboard_state[raw_value].SetValue(action == GLFW_PRESS || action == GLFW_REPEAT);
	}

	void KeyboardState::SetMouseButtonState(MouseButton key, int action)
	{
		int raw_value = int(key);
		if (raw_value >= 0 && raw_value < mouse_button_state.size())
			mouse_button_state[raw_value].SetValue(action == GLFW_PRESS || action == GLFW_REPEAT);
	}

	ButtonState const * KeyboardState::GetKeyState(Key key)
	{
		int raw_value = key.GetRawValue();

		if (key.GetType() == KeyType::KEYBOARD)
		{
			if (raw_value >= 0 && raw_value < keyboard_state.size())
				return &keyboard_state[raw_value];
		}
		else if (key.GetType() == KeyType::MOUSE)
		{
			if (raw_value >= 0 && raw_value < mouse_button_state.size())
				return &mouse_button_state[raw_value];
		}
		return nullptr;
	}

	void KeyboardState::UpdateKeyStates(float delta_time)
	{
		for (ButtonState& button : keyboard_state)
			button.UpdateSameValueTimer(delta_time);
		for (ButtonState& button : mouse_button_state)
			button.UpdateSameValueTimer(delta_time);
	}

}; // namespace chaos