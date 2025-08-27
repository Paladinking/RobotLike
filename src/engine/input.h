#ifndef INPUT_00_H
#define INPUT_00_H
#include <memory>
#include <string>
#include <utility>
#include <SDL3/SDL.h>
#include "exceptions.h"

/**
 * Exception type for bad binding names.
 */
class binding_exception : public base_exception {
	public:
		explicit binding_exception(std::string msg) : base_exception(std::move(msg)) {};
	
};

/**
 * Class for press inputs, used in event handling methods.
 */
class PressInput {
	public:
		/**
		 * Returns true if this input was the target of a key or mouse event.
		 */
		virtual bool is_targeted(SDL_Keycode key, Uint32 mouseButton) = 0;

        virtual ~PressInput() = default;
};

/**
 * Class for a press input targeted by mouse.
 */
class MousePressInput : public PressInput {
	public:
		explicit MousePressInput(const Uint32 mouse_button) : mouse_button(mouse_button) {}
		
		/**
		 * Returns true if the mouse targets equals the one for this input.
		 */
		bool is_targeted(const SDL_Keycode key, const Uint32 mouse) override {
			return mouse == mouse_button;
		}

	private:
		const Uint32 mouse_button;
};

/**
 * Class for a press input that can never be triggered.
 */
class EmptyPressInput : public PressInput {
	public:
		/**
		 * Always returns false.
		 */
		bool is_targeted(const SDL_Keycode key, const Uint32 mouse) override {
			return false;
		}
};

/**
 * Class for a press input triggered by keyboard.
 */
class KeyPressInput : public PressInput {
	public:
		explicit KeyPressInput(const SDL_Keycode key_code) : key_code(key_code) {}
		
		/**
		 * Returns true if the key target equals the one for this input.
		 */
		bool is_targeted(const SDL_Keycode key, const Uint32 mouseButton) override {
			return key_code == key;
		}

	private:
		const SDL_Keycode key_code;
};

/**
 * Class for toggle-like inputs where only the first press and release are relevant.
 */
class OnePressInput {
    public:
        virtual bool is_targeted(SDL_Keycode key, Uint32 mouseButton, bool press) = 0;
        virtual ~OnePressInput() = default;
    protected:
        bool has_been_pressed = false;
};

class OnePressKeyInput : public OnePressInput {
    public:
        explicit OnePressKeyInput(const SDL_Keycode key_code) : key_code(key_code) {}
        bool is_targeted(SDL_Keycode key, Uint32 mouse_button, bool press) override;
    private:
        const SDL_Keycode key_code;
};

class OnePressMouseInput : public OnePressInput {
    public:
        explicit OnePressMouseInput(Uint32 mouseButton) : mouse_button(mouseButton) {}
        bool is_targeted(SDL_Keycode key, Uint32 mouseButton, bool press) override;
    private:
        const Uint32 mouse_button;
};

class EmptyOnePressInput : public OnePressInput {
    public:
        bool is_targeted(SDL_Keycode key, Uint32 mouseButton, bool press) override {
            return false;
        };
};

/**
 * Class for hold inputs, used when polling an input every frame.
 */
class HoldInput {
	public:
		/**
		 * Returns true if this input is currently pressed.
		 */
		virtual bool is_pressed(const Uint8* keys, Uint32  mouseButton) = 0;

        virtual ~HoldInput() = default;
};

/**
 * Class for hold inputs based on the mouse.
 */
class MouseHoldInput : public HoldInput {
	public:
		explicit MouseHoldInput(const Uint32 mouse_mask) : mouse_mask(mouse_mask) {}

		/**
		 * Returns true if the mouseButton of this input is pressed.
		 */
		bool is_pressed(const Uint8* keys, const Uint32 mouseButton) override {
			return (mouse_mask & mouseButton) != 0;
		}

	private:
		const Uint32 mouse_mask;
};

/**
 * Class for hold inputs that can never be pressed.
 */
class EmptyHoldInput : public HoldInput {
	public:

		/**
		 * Always returns false.
		 */ 
		bool is_pressed(const Uint8* keys, const Uint32 mouseButton) override {
			return false;
		}
};

/**
 * Class for a hold input targeted by keyboard.
 */
class KeyHoldInput : public HoldInput {
	public:
		explicit KeyHoldInput(const SDL_Scancode key) : key(key) {}

		/**
		 * Returns true if the key matching this input is pressed.
		 */
		bool is_pressed(const Uint8* keys, const Uint32 mouseButton) override {
			return keys[key];
		}

	private:
		const SDL_Scancode key;
};

/**
 * Returns the name of an input based on a down-event.
 * If mouse contains an SDL_BUTTON_*something* value, the key is ignored.
 * If no name matches, "None" is returned. Passing "None" to get_*hold/press*_input 
 * will return an EmptyHold/Press/Input ptr.
 */
std::string get_input_name(SDL_Keycode key, Uint32 mouse);

/**
 * Returns a pointer to a PressInput with the name name.
 * If no input matches, a binding_exception is thrown.
 */
std::unique_ptr<PressInput> get_press_input(const std::string& name);
/**
 * Returns a pointer to a HoldInput with the name name.
 * If no input matches, a binding_exception is thrown.
 */
std::unique_ptr<HoldInput> get_hold_input(const std::string& name);

/**
 * Returns a pointer to a PressInput with the name name.
 * If no input matches, default_name is tried instead.
 * If default_name also has no match, a binding_exception is thrown.
 */
std::unique_ptr<PressInput> get_press_input(const std::string& name, const std::string& default_name);

/**
 * Returns a pointer to a HoldInput with the name name.
 * If no input matches, default_name is tried instead.
 * If default_name also has no match, a binding_exception is thrown.
 */
std::unique_ptr<HoldInput> get_hold_input(const std::string& name, const std::string& default_name);

std::unique_ptr<OnePressInput> get_one_press_input(const std::string& name);

#endif
