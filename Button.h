
class Button {
public:
	Button(uint8_t pin, uint8_t debounce = 3) {
		_clickState = debounce;
		_pin = pin;
		pinMode(_pin, INPUT_PULLUP);
	}

	void update() {
		bool pressed = !digitalRead(_pin);
		if (_stateCounter == _clickState) {
			// Already in clicked state, go to pressed state.
			_stateCounter--;
		} else if (_stateCounter > 0) {
			// Pressed state, count down if the button is not pressed now.
			// Count holding if the button remains pressed
			if (pressed) {
				if (_holdCounter < 255) _holdCounter++;
			} else {
				// Release hold if the state goes to not pressed.
				if (--_stateCounter == 0) _holdCounter = 0;
			}
		} else {
			// Not already pressed, go to click state if the button is pressed now.
			if (pressed) _stateCounter = _clickState;
		}
	}

	inline bool isClicked() const {
		return _stateCounter == _clickState;
	}

	inline bool isPressed() const {
		return _stateCounter > 0;
	}

	inline bool isReleased() const {
		return _stateCounter == 0;
	}

	inline bool isHeld(uint8_t hold = 10) {
		return _holdCounter >= hold;
	}

private:
	uint8_t _pin = -1;
	uint8_t _stateCounter = 0,
		_holdCounter = 0,
		_clickState;
};
