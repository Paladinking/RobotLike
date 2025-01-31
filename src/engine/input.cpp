#include "input.h"

std::string get_input_name(const SDL_Keycode key, const Uint32 mouse) {
	if (mouse == SDL_BUTTON_LEFT) {
		return "Mouse Left";
	} else if (mouse == SDL_BUTTON_MIDDLE) {
		return "Mouse Middle";
	} else if (mouse == SDL_BUTTON_RIGHT) {
		return "Mouse Right";
	}
	
	const char* name = SDL_GetKeyName(key);
	if (strcmp(name, "") != 0) {
		return {name};
	}
	return "None";
}

std::unique_ptr<PressInput> get_press_input(const std::string& name, const std::string& default_name) {
	try {
		return get_press_input(name);
	} catch (const binding_exception&) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    R"(Invalid key "%s", using "%s")", name.c_str(), default_name.c_str());
		return get_press_input(default_name);
	}
}

std::unique_ptr<HoldInput> get_hold_input(const std::string& name, const std::string& default_name) {
	try {
		return get_hold_input(name);
	} catch (const binding_exception&) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    R"(Invalid key "%s", using "%s")", name.c_str(), default_name.c_str());
		return get_hold_input(default_name);
	}	
}

std::unique_ptr<PressInput> get_press_input(const std::string& name) {

	if (name == "Mouse Left") {
		return std::unique_ptr<PressInput>(new MousePressInput(SDL_BUTTON_LEFT));
	} else if (name == "Mouse Middle") {
		return std::unique_ptr<PressInput>(new MousePressInput(SDL_BUTTON_MIDDLE));
	} else if (name == "Mouse Right") {
		return std::unique_ptr<PressInput>(new MousePressInput(SDL_BUTTON_RIGHT));
	} else if (name == "None") {
		return std::unique_ptr<PressInput>(new EmptyPressInput());
	}
	SDL_Keycode keycode = SDL_GetKeyFromName(name.c_str());
	if (keycode == SDLK_UNKNOWN) {
		throw binding_exception("Invalid key name \"" + name + "\"");
	}
	return std::unique_ptr<PressInput>(new KeyPressInput(keycode));	
}

std::unique_ptr<HoldInput> get_hold_input(const std::string& name) {

	if (name == "Mouse Left") {
		return std::unique_ptr<HoldInput>(new MouseHoldInput(SDL_BUTTON_LMASK));
	} else if (name == "Mouse Middle") {
		return std::unique_ptr<HoldInput>(new MouseHoldInput(SDL_BUTTON_MMASK));
	} else if (name == "Mouse Right") {
		return std::unique_ptr<HoldInput>(new MouseHoldInput(SDL_BUTTON_RMASK));
	} else if (name == "None") {
		return std::unique_ptr<HoldInput>(new EmptyHoldInput());
	}
	
	SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());
	if (scancode == SDL_SCANCODE_UNKNOWN) {
		throw binding_exception("Invalid key name \"" + name + "\"");
	}
	return std::unique_ptr<HoldInput>(new KeyHoldInput(scancode));
}

std::unique_ptr<OnePressInput> get_one_press_input(const std::string &name) {
    if (name == "Mouse Left") {
        return std::unique_ptr<OnePressInput>(new OnePressMouseInput(SDL_BUTTON_LEFT));
    } else if (name == "Mouse Middle") {
        return std::unique_ptr<OnePressInput>(new OnePressMouseInput(SDL_BUTTON_MIDDLE));
    } else if (name == "Mouse Right") {
        return std::unique_ptr<OnePressInput>(new OnePressMouseInput(SDL_BUTTON_RIGHT));
    } else if (name == "None") {
        return std::unique_ptr<OnePressInput>(new EmptyOnePressInput());
    }
    SDL_Keycode keycode = SDL_GetKeyFromName(name.c_str());
    if (keycode == SDLK_UNKNOWN) {
        throw binding_exception("Invalid key name \"" + name + "\"");
    }
    return std::unique_ptr<OnePressInput>(new OnePressKeyInput(keycode));
}

bool OnePressKeyInput::is_targeted(SDL_Keycode key, Uint32 mouseButton, bool press) {
    if(key_code == key) {
        if(press && !has_been_pressed) {
            has_been_pressed = true;
            return true;
        } else if (!press && has_been_pressed) {
            has_been_pressed = false;
            return true;
        }
    }
    return false;
}

bool OnePressMouseInput::is_targeted(SDL_Keycode key, Uint32 mouseButton, bool press) {
    if(mouse_button == mouseButton) {
        if(press && !has_been_pressed) {
            has_been_pressed = true;
            return true;
        } else if (!press && has_been_pressed) {
            has_been_pressed = false;
            return true;
        }
    }
    return false;
}
