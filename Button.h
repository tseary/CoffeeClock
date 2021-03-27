
class Button {
public:
	Button(uint8_t pin, uint8_t debounce = 5) {
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
			if (!pressed) _stateCounter--;
		} else {
			// Not pressed, go to click state if the button is pressed now.
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
	
private:
	uint8_t _pin = -1;
	uint8_t _stateCounter = 0,
		_clickState;
};
