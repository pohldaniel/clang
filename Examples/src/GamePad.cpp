#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <SDL3/SDL.h>

#include "GamePad.h"

GLFWGamePad* GLFWGamePad::s_Players[4] = { nullptr, nullptr, nullptr, nullptr };

GLFWGamePad::GLFWGamePad(){
	m_playerIndex = -1;
	m_pCurrButtonStates = m_buttonStates[0];
	m_pPrevButtonStates = m_buttonStates[1];

	memset(m_pCurrButtonStates, false, 21);
	memset(m_pPrevButtonStates, false, 21);
	m_connected = false;
    m_leftStickX = 0.0f;
	m_leftStickY = 0.0f;
	m_rightStickX = 0.0f;
	m_rightStickY = 0.0f;
	m_leftTrigger = 0.0f;
	m_rightTrigger = 0.0f;
}

void GLFWGamePad::update() {
    std::swap(m_pCurrButtonStates, m_pPrevButtonStates);
    std::memset(m_pCurrButtonStates, 0, 21 * sizeof(bool));

    GLFWgamepadstate state;
    if (glfwGetGamepadState(m_playerIndex, &state)) {
        m_connected = true;
        m_pCurrButtonStates[GAMEPAD_A] = (state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_B] = (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_X] = (state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_Y] = (state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_LEFT_SHOULDER] = (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_RIGHT_SHOULDER] = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_BACK] = (state.buttons[GLFW_GAMEPAD_BUTTON_BACK] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_START] = (state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_LEFT_THUMB] = (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] == GLFW_PRESS);
        m_pCurrButtonStates[GAMEPAD_RIGHT_THUMB] = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] == GLFW_PRESS);

        bool up    = (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS);
        bool down  = (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS);
        bool left  = (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS);
        bool right = (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS);

        m_pCurrButtonStates[DPAD_UP]    = up;
        m_pCurrButtonStates[DPAD_DOWN]  = down;
        m_pCurrButtonStates[DPAD_LEFT]  = left;
        m_pCurrButtonStates[DPAD_RIGHT] = right;

        m_pCurrButtonStates[DPAD_UP_RIGHT]   = up && right;
        m_pCurrButtonStates[DPAD_DOWN_RIGHT] = down && right;
        m_pCurrButtonStates[DPAD_UP_LEFT]    = up && left;
        m_pCurrButtonStates[DPAD_DOWN_LEFT]  = down && left;

        m_pCurrButtonStates[GAMEPAD_LEFT_TRIGGER]  = (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.0f); 
        m_pCurrButtonStates[GAMEPAD_RIGHT_TRIGGER] = (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.0f);

        m_leftTrigger = (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) * 0.5f;
        m_rightTrigger = (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) * 0.5f;

        if (glfwJoystickPresent(m_playerIndex)){
            float rawRX = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
            float rawRY = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];

            float rawLX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            float rawLY = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

            float magnitudeR = std::sqrt(rawRX * rawRX + rawRY * rawRY);
            if (magnitudeR > DEADZONE) {

                if (magnitudeR > 1.0f) magnitudeR = 1.0f;
                
                float normRX = rawRX / magnitudeR;
                float normRY = rawRY / magnitudeR;
                float normalizedMagnitude = (magnitudeR - DEADZONE) / (1.0f - DEADZONE);
                
                m_leftStickX = normRX * normalizedMagnitude;
                m_leftStickY = normRY * normalizedMagnitude;
            } else {
                m_leftStickX = 0.0f;
                m_leftStickY = 0.0f;
            }

            float magnitudeL = std::sqrt(rawLX * rawLX + rawLY * rawLY);
            if (magnitudeL > DEADZONE) {
                if (magnitudeL > 1.0f) magnitudeL = 1.0f;
                
                float normLX = rawLX / magnitudeL;
                float normLY = rawLY / magnitudeL;
                
                float normalizedMagnitude = (magnitudeL - DEADZONE) / (1.0f - DEADZONE);
                
                m_rightStickX = normLX * normalizedMagnitude;
                m_rightStickY = normLY * normalizedMagnitude;
            }else{
                m_rightStickX = 0.0f;
                m_rightStickY = 0.0f;
            }
        }
    }else {
		if (m_connected) {
			m_connected = false;
			memset(m_pCurrButtonStates, false, 21);
		}		
	}
}

void GLFWGamePad::setVibration(float leftMotor, float rightMotor) {
    leftMotor = std::clamp(leftMotor, 0.0f, 1.0f);
    rightMotor = std::clamp(rightMotor, 0.0f, 1.0f);
}

float GLFWGamePad::getLeftStickX() const {
     return m_leftStickX;
}

float GLFWGamePad::getLeftStickY() const {
    return m_leftStickY;
}

float GLFWGamePad::getRightStickX() const {
    return m_rightStickX;
}

float GLFWGamePad::getRightStickY() const {
     return m_rightStickY;
}

float GLFWGamePad::getLeftTrigger() const {
    return m_leftTrigger;
}

float GLFWGamePad::getRightTrigger() const {
     return m_rightTrigger;
}

bool GLFWGamePad::buttonDown(GamepadButton button) const {
    return m_pCurrButtonStates[button];
}

bool GLFWGamePad::buttonPressed(GamepadButton button) const {
    return m_pCurrButtonStates[button] && !m_pPrevButtonStates[button];
}

bool GLFWGamePad::buttonUp(GamepadButton button) const {
    return !m_pCurrButtonStates[button] && m_pPrevButtonStates[button];
}

bool GLFWGamePad::isConnected() {
	return m_connected;
}

IGamePad* GLFWGamePad::GetPlayer(int playerIndex) {
    if (playerIndex >= 0 && playerIndex < 4) 
        return s_Players[playerIndex];
    return nullptr;
}

IGamePad* GLFWGamePad::GetPlayer(){
    for (int i = 0; i < 4; ++i) {
		if (s_Players[i]->isConnected()) {			
			return s_Players[i];
		}
	}
    return nullptr;
}

void GLFWGamePad::Initialize() {
    std::ifstream file("res/gamecontrollerdb.txt");
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        glfwUpdateGamepadMappings(buffer.str().c_str());        
    }
    file.close();
	for (int i = 0; i < 4; ++i) {
		s_Players[i] = new GLFWGamePad();
		s_Players[i]->m_playerIndex = i;
	}
}

void GLFWGamePad::Update() {
	for (int i = 0; i < 4; ++i) {
		IGamePad* player = GLFWGamePad::GetPlayer(i);
		if (player) {
			player->update();
		}
	}
}

void GLFWGamePad::Shutdown() {
	for (int i = 0; i < 4; ++i) {
		if (s_Players[i] != nullptr) {			
			delete s_Players[i];
			s_Players[i] = nullptr;
		}
	}
}
///////////////////////////////////////////////////////////////////////
SDLGamePad* SDLGamePad::s_Players[4] = { nullptr, nullptr, nullptr, nullptr };

SDLGamePad::SDLGamePad(){
	m_playerIndex = -1;
	m_pCurrButtonStates = m_buttonStates[0];
	m_pPrevButtonStates = m_buttonStates[1];

	memset(m_pCurrButtonStates, false, 21);
	memset(m_pPrevButtonStates, false, 21);
	m_connected = false;
    m_leftStickX = 0.0f;
	m_leftStickY = 0.0f;
	m_rightStickX = 0.0f;
	m_rightStickY = 0.0f;
	m_leftTrigger = 0.0f;
	m_rightTrigger = 0.0f;
}

void SDLGamePad::update() {
    std::swap(m_pCurrButtonStates, m_pPrevButtonStates);
    std::memset(m_pCurrButtonStates, 0, 21 * sizeof(bool));

    m_leftStickX = 0.0f;
	m_leftStickY = 0.0f;
	m_rightStickX = 0.0f;
	m_rightStickY = 0.0f;
	m_leftTrigger = 0.0f;
	m_rightTrigger = 0.0f;

    if (m_connected && m_sdlGamepad) {
        m_pCurrButtonStates[GAMEPAD_A] = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_SOUTH);
        m_pCurrButtonStates[GAMEPAD_B] = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_EAST);
        m_pCurrButtonStates[GAMEPAD_X] = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_WEST);
        m_pCurrButtonStates[GAMEPAD_Y] = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_NORTH);
        
        m_pCurrButtonStates[GAMEPAD_LEFT_SHOULDER]  = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
        m_pCurrButtonStates[GAMEPAD_RIGHT_SHOULDER] = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
        m_pCurrButtonStates[GAMEPAD_BACK]           = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_BACK);
        m_pCurrButtonStates[GAMEPAD_START]          = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_START);
        m_pCurrButtonStates[GAMEPAD_LEFT_THUMB]     = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
        m_pCurrButtonStates[GAMEPAD_RIGHT_THUMB]    = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);

        bool up    = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
        bool down  = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        bool left  = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
        bool right = SDL_GetGamepadButton(m_sdlGamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

        m_pCurrButtonStates[DPAD_UP]    = up;
        m_pCurrButtonStates[DPAD_DOWN]  = down;
        m_pCurrButtonStates[DPAD_LEFT]  = left;
        m_pCurrButtonStates[DPAD_RIGHT] = right;

        m_pCurrButtonStates[DPAD_UP_RIGHT]   = up && right;
        m_pCurrButtonStates[DPAD_DOWN_RIGHT] = down && right;
        m_pCurrButtonStates[DPAD_UP_LEFT]    = up && left;
        m_pCurrButtonStates[DPAD_DOWN_LEFT]  = down && left;

        float rawLeftTrigger  = SDL_GetGamepadAxis(m_sdlGamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
        float rawRightTrigger = SDL_GetGamepadAxis(m_sdlGamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

        m_pCurrButtonStates[GAMEPAD_LEFT_TRIGGER]  = (rawLeftTrigger > 0.0f);
        m_pCurrButtonStates[GAMEPAD_RIGHT_TRIGGER] = (rawRightTrigger > 0.0f);


        m_leftTrigger  = (rawLeftTrigger + 1.0f) * 0.5f;
        m_rightTrigger = (rawRightTrigger + 1.0f) * 0.5f;

        float rawLX = static_cast<float>(SDL_GetGamepadAxis(m_sdlGamepad, SDL_GAMEPAD_AXIS_LEFTX));
        float rawLY = static_cast<float>(SDL_GetGamepadAxis(m_sdlGamepad, SDL_GAMEPAD_AXIS_LEFTY));
        float rawRX = static_cast<float>(SDL_GetGamepadAxis(m_sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTX));
        float rawRY = static_cast<float>(SDL_GetGamepadAxis(m_sdlGamepad, SDL_GAMEPAD_AXIS_RIGHTY));

        float magnitudeL = std::sqrtf(rawLX * rawLX + rawLY * rawLY);
        if (magnitudeL < DEADZONE) {
            m_rightStickX = 0.0f;
            m_rightStickY = 0.0f;
        }else {
            float normLX = rawLX / magnitudeL;
            float normLY = rawLY / magnitudeL;
            float normalizedMagnitude = (magnitudeL - DEADZONE) / (32767.0f - DEADZONE);

            normalizedMagnitude = std::min(normalizedMagnitude, 1.0f);

            m_rightStickX = normLX * normalizedMagnitude;
            m_rightStickY = -normLY * normalizedMagnitude;
        }

        float magnitudeR = std::sqrtf(rawRX * rawRX + rawRY * rawRY);
        if (magnitudeR < DEADZONE) {
            m_leftStickX = 0.0f;
            m_leftStickY = 0.0f;
        }else {
            float normRX = rawRX / magnitudeR;
            float normRY = rawRY / magnitudeR;
            float normalizedMagnitude = (magnitudeR - DEADZONE) / (32767.0f - DEADZONE);

            normalizedMagnitude = std::min(normalizedMagnitude, 1.0f);

            m_leftStickX = normRX * normalizedMagnitude;
            m_leftStickY = -normRY * normalizedMagnitude;
        }
    } else {
        if (m_connected) {
            m_connected = false;
            std::memset(m_pCurrButtonStates, 0, 21 * sizeof(bool));
        }       
    }
}

void SDLGamePad::Update() {
    SDL_UpdateJoysticks();

    int gamepadCount = 0;
    const SDL_JoystickID* connectedGamepads = SDL_GetGamepads(&gamepadCount);

    for (int i = 0; i < 4; ++i) {
        SDLGamePad* player = s_Players[i];
        if (!player) continue;

        if (player->m_sdlGamepad && !SDL_GamepadConnected(player->m_sdlGamepad)) {
            SDL_CloseGamepad(player->m_sdlGamepad);
            player->m_sdlGamepad = nullptr;
            player->m_connected = false;
        }

        if (player->m_sdlGamepad == nullptr && i < gamepadCount && connectedGamepads != nullptr) {
            SDL_JoystickID targetId = connectedGamepads[i];
            bool alreadyAssigned = false;
            for (int p = 0; p < 4; ++p) {
                if (s_Players[p] && s_Players[p]->m_sdlGamepad && 
                    SDL_GetGamepadID(s_Players[p]->m_sdlGamepad) == targetId) {
                    alreadyAssigned = true;
                    break;
                }
            }

            if (!alreadyAssigned) {
                player->m_sdlGamepad = SDL_OpenGamepad(targetId);
                if (player->m_sdlGamepad) {
                    player->m_connected = true;
                }
            }
        }
        player->update();
    }
}

void SDLGamePad::setVibration(float leftMotor, float rightMotor) {
    if (!m_connected || !m_sdlGamepad) 
      return;

    leftMotor = std::clamp(leftMotor, 0.0f, 1.0f);
    rightMotor = std::clamp(rightMotor, 0.0f, 1.0f);

    Uint16 left = static_cast<Uint16>(leftMotor * 65535.0f);
    Uint16 right = static_cast<Uint16>(rightMotor * 65535.0f);
    SDL_RumbleGamepad(m_sdlGamepad, left, right, 0xFFFF);
}

float SDLGamePad::getRightStickX() const { 
    return m_rightStickX; 
}

float SDLGamePad::getRightStickY() const { 
    return m_rightStickY; 
}

float SDLGamePad::getLeftStickX() const { 
    return m_leftStickX; 
}

float SDLGamePad::getLeftStickY() const { 
    return m_leftStickY; 
}

float SDLGamePad::getLeftTrigger() const { 
    return m_leftTrigger; 
}

float SDLGamePad::getRightTrigger() const { 
    return m_rightTrigger; 
}

bool SDLGamePad::isConnected() { 
    return m_connected; 
}

bool SDLGamePad::buttonPressed(GamepadButton button) const {
    return m_pCurrButtonStates[button] && !m_pPrevButtonStates[button];
}

bool SDLGamePad::buttonDown(GamepadButton button) const {
    return m_pCurrButtonStates[button];
}

bool SDLGamePad::buttonUp(GamepadButton button) const {
    return !m_pCurrButtonStates[button] && m_pPrevButtonStates[button];
}

IGamePad* SDLGamePad::GetPlayer(int playerIndex) {
    if (playerIndex >= 0 && playerIndex < 4) {
        return s_Players[playerIndex];
    }
    return nullptr;
}

IGamePad* SDLGamePad::GetPlayer(){
    for (int i = 0; i < 4; ++i) {
		if (s_Players[i]->isConnected()) {			
			return s_Players[i];
		}
	}
    return nullptr;
}

void SDLGamePad::Initialize() {
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");
    SDL_InitSubSystem(SDL_INIT_GAMEPAD);
    int addedMappings = SDL_AddGamepadMappingsFromFile("res/gamecontrollerdb.txt");
    for (int i = 0; i < 4; ++i) {
		s_Players[i] = new SDLGamePad();
		s_Players[i]->m_playerIndex = i;
	}
}

void SDLGamePad::Shutdown() {
    for (int i = 0; i < 4; ++i) {
        if (s_Players[i]) {
            if (s_Players[i]->m_sdlGamepad) {
                SDL_CloseGamepad(s_Players[i]->m_sdlGamepad);
            }
            delete s_Players[i];
            s_Players[i] = nullptr;
        }
    }
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
    SDL_Quit();
}