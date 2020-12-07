
#include <chaos/Chaos.h>

namespace chaos
{



#define CHAOS_KEYDEF(KEY) std::make_pair((Key)GLFW_KEY_##KEY, #KEY)

	std::vector<std::pair<Key, char const *>> const KeyDefinition::key_map =
	{
CHAOS_KEYDEF(SPACE),
CHAOS_KEYDEF(APOSTROPHE),
CHAOS_KEYDEF(COMMA),
CHAOS_KEYDEF(MINUS),
CHAOS_KEYDEF(PERIOD),
CHAOS_KEYDEF(SLASH),
CHAOS_KEYDEF(0),
CHAOS_KEYDEF(1),
CHAOS_KEYDEF(2),
CHAOS_KEYDEF(3),
CHAOS_KEYDEF(4),
CHAOS_KEYDEF(5),
CHAOS_KEYDEF(6),
CHAOS_KEYDEF(7),
CHAOS_KEYDEF(8),
CHAOS_KEYDEF(9),
CHAOS_KEYDEF(SEMICOLON),
CHAOS_KEYDEF(EQUAL),
CHAOS_KEYDEF(A),
CHAOS_KEYDEF(B),
CHAOS_KEYDEF(C),
CHAOS_KEYDEF(D),
CHAOS_KEYDEF(E),
CHAOS_KEYDEF(F),
CHAOS_KEYDEF(G),
CHAOS_KEYDEF(H),
CHAOS_KEYDEF(I),
CHAOS_KEYDEF(J),
CHAOS_KEYDEF(K),
CHAOS_KEYDEF(L),
CHAOS_KEYDEF(M),
CHAOS_KEYDEF(N),
CHAOS_KEYDEF(O),
CHAOS_KEYDEF(P),
CHAOS_KEYDEF(Q),
CHAOS_KEYDEF(R),
CHAOS_KEYDEF(S),
CHAOS_KEYDEF(T),
CHAOS_KEYDEF(U),
CHAOS_KEYDEF(V),
CHAOS_KEYDEF(W),
CHAOS_KEYDEF(X),
CHAOS_KEYDEF(Y),
CHAOS_KEYDEF(Z),
CHAOS_KEYDEF(LEFT_BRACKET),
CHAOS_KEYDEF(BACKSLASH),
CHAOS_KEYDEF(RIGHT_BRACKET),
CHAOS_KEYDEF(GRAVE_ACCENT),
CHAOS_KEYDEF(WORLD_1),
CHAOS_KEYDEF(WORLD_2),
CHAOS_KEYDEF(ESCAPE),
CHAOS_KEYDEF(ENTER),
CHAOS_KEYDEF(TAB),
CHAOS_KEYDEF(BACKSPACE),
CHAOS_KEYDEF(INSERT),
CHAOS_KEYDEF(DELETE),
CHAOS_KEYDEF(RIGHT),
CHAOS_KEYDEF(LEFT),
CHAOS_KEYDEF(DOWN),
CHAOS_KEYDEF(UP),
CHAOS_KEYDEF(PAGE_UP),
CHAOS_KEYDEF(PAGE_DOWN),
CHAOS_KEYDEF(HOME),
CHAOS_KEYDEF(END),
CHAOS_KEYDEF(CAPS_LOCK),
CHAOS_KEYDEF(SCROLL_LOCK),
CHAOS_KEYDEF(NUM_LOCK),
CHAOS_KEYDEF(PRINT_SCREEN),
CHAOS_KEYDEF(PAUSE),
CHAOS_KEYDEF(F1),
CHAOS_KEYDEF(F2),
CHAOS_KEYDEF(F3),
CHAOS_KEYDEF(F4),
CHAOS_KEYDEF(F5),
CHAOS_KEYDEF(F6),
CHAOS_KEYDEF(F7),
CHAOS_KEYDEF(F8),
CHAOS_KEYDEF(F9),
CHAOS_KEYDEF(F10),
CHAOS_KEYDEF(F11),
CHAOS_KEYDEF(F12),
CHAOS_KEYDEF(F13),
CHAOS_KEYDEF(F14),
CHAOS_KEYDEF(F15),
CHAOS_KEYDEF(F16),
CHAOS_KEYDEF(F17),
CHAOS_KEYDEF(F18),
CHAOS_KEYDEF(F19),
CHAOS_KEYDEF(F20),
CHAOS_KEYDEF(F21),
CHAOS_KEYDEF(F22),
CHAOS_KEYDEF(F23),
CHAOS_KEYDEF(F24),
CHAOS_KEYDEF(F25),
CHAOS_KEYDEF(KP_0),
CHAOS_KEYDEF(KP_1),
CHAOS_KEYDEF(KP_2),
CHAOS_KEYDEF(KP_3),
CHAOS_KEYDEF(KP_4),
CHAOS_KEYDEF(KP_5),
CHAOS_KEYDEF(KP_6),
CHAOS_KEYDEF(KP_7),
CHAOS_KEYDEF(KP_8),
CHAOS_KEYDEF(KP_9),
CHAOS_KEYDEF(KP_DECIMAL),
CHAOS_KEYDEF(KP_DIVIDE),
CHAOS_KEYDEF(KP_MULTIPLY),
CHAOS_KEYDEF(KP_SUBTRACT),
CHAOS_KEYDEF(KP_ADD),
CHAOS_KEYDEF(KP_ENTER),
CHAOS_KEYDEF(KP_EQUAL),
CHAOS_KEYDEF(LEFT_SHIFT),
CHAOS_KEYDEF(LEFT_CONTROL),
CHAOS_KEYDEF(LEFT_ALT),
CHAOS_KEYDEF(LEFT_SUPER),
CHAOS_KEYDEF(RIGHT_SHIFT),
CHAOS_KEYDEF(RIGHT_CONTROL),
CHAOS_KEYDEF(RIGHT_ALT),
CHAOS_KEYDEF(RIGHT_SUPER),
CHAOS_KEYDEF(MENU)
	};
#undef CHAOS_KEYDEF

	std::vector<std::pair<MouseButton, char const*>> const KeyDefinition::mousebutton_map =
	{
		std::make_pair(MouseButton::BUTTON_1, "MOUSE_BUTTON_1"),
		std::make_pair(MouseButton::BUTTON_2, "MOUSE_BUTTON_2"),
		std::make_pair(MouseButton::BUTTON_3, "MOUSE_BUTTON_3"),
		std::make_pair(MouseButton::BUTTON_4, "MOUSE_BUTTON_4"),
		std::make_pair(MouseButton::BUTTON_5, "MOUSE_BUTTON_5"),
		std::make_pair(MouseButton::BUTTON_6, "MOUSE_BUTTON_6"),
		std::make_pair(MouseButton::BUTTON_7, "MOUSE_BUTTON_7"),
		std::make_pair(MouseButton::BUTTON_8, "MOUSE_BUTTON_8"),
	};

	
	Key KeyDefinition::GetKey(char const* name)
	{
		for (auto const& entry : key_map)
			if (StringTools::Stricmp(name, entry.second) == 0)
				return entry.first;
		return Key::UNKNOWN;
	}
	
	char const* KeyDefinition::GetKeyName(Key value)
	{
		for (auto const& entry : key_map)
			if (value == entry.first)
				return entry.second;
		return nullptr;
	}

	MouseButton KeyDefinition::GetMouseButton(char const* name)
	{
		for (auto const& entry : mousebutton_map)
			if (StringTools::Stricmp(name, entry.second) == 0)
				return entry.first;
		return MouseButton::UNKNOWN;
	}

	char const* KeyDefinition::GetMouseButtonName(MouseButton value)
	{
		for (auto const& entry : mousebutton_map)
			if (value == entry.first)
				return entry.second;
		return nullptr;
	}

}; // namespace chaos
