#include <iostream>
#include <GLFW/glfw3.h>
#include "Application.h"
#include "Mouse.h"

Mouse &Mouse::instance(){
	static Mouse _instance;
	return _instance;
}

Mouse::Mouse(){

	m_window = nullptr;
	m_cursorVisible = true;

	m_wheelDelta = 0;
	m_prevWheelDelta = 0;
	m_mouseWheel = 0.0f;

	m_xPos = 0;
	m_yPos = 0;
	m_xDelta = 0.0f;
	m_yDelta = 0.0f;
	m_attached = false;

    m_currentStates = m_buttonStates[0];
    m_prevStates = m_buttonStates[1];
}

Mouse::~Mouse(){

}

void Mouse::update(){
    if (m_attached) {
        glfwGetCursorPos(m_window, &m_xPos, &m_yPos);
        m_xDelta = m_xPrevPos - m_xPos;
		m_yDelta = m_yPrevPos - m_yPos;
        m_xPrevPos = m_xPos;
        m_yPrevPos = m_yPos;

        bool *tempStates = m_prevStates;
        m_prevStates = m_currentStates;
        m_currentStates = tempStates;
        m_currentStates[0] = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        m_currentStates[1] = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        m_currentStates[2] = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    }
}

void Mouse::attach(GLFWwindow* window, bool _hideCursor, bool reset, bool reattach){

    if (m_attached && !reattach) return;
        m_window = window;

    glfwGetCursorPos(window, &m_xPos, &m_yPos);
    m_xPrevPos = m_xPos;
    m_yPrevPos = m_yPos;

	if (reset) {
       m_xLastPos = m_xPos;
       m_yLastPos = m_yPos;
	}

    m_centerX = static_cast<double>(Application::Width / 2);
    m_centerY = static_cast<double>(Application::Height / 2);

    if (_hideCursor) {
        hideCursor(true);
        setCursorToMiddle();       
    }else{
        glfwSetCursorPos(m_window, m_xLastPos, m_yLastPos);
	    hideCursor(false);
    }

	m_attached = true;
}

void Mouse::detach() {
    if (!m_attached) return;	
      m_attached = false;

	if (!m_cursorVisible) {
		glfwSetCursorPos(m_window, m_xLastPos, m_yLastPos);
		hideCursor(false);
	}

	m_xDelta = 0.0f;
	m_yDelta = 0.0f;
	m_window = nullptr;
}

void Mouse::setCursorToMiddle(){
    glfwSetCursorPos(m_window, m_centerX, m_centerY);
    m_xPrevPos = m_centerX;
    m_yPrevPos = m_centerY;
}

void Mouse::hideCursor(bool hideCursor){

    if (hideCursor) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        m_cursorVisible = false;
    }else {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_cursorVisible = true;
    }
}

float Mouse::xDelta() const {
    return m_xDelta;
}

float Mouse::yDelta() const {
    return m_yDelta;
}

float Mouse::xPos() const{
    return m_xPos;
}

float Mouse::yPos() const{
    return m_yPos;
}

bool Mouse::isAttached() const {
    return m_attached;
}

bool Mouse::isVisibile() {
	return m_cursorVisible;
}

bool Mouse::buttonDown(unsigned int button) const {
    return glfwGetMouseButton(Application::Window, button) == GLFW_PRESS;
}

bool Mouse::buttonDownInvisible(unsigned int button) const {
    return glfwGetMouseButton(Application::Window, button) == GLFW_PRESS && !m_cursorVisible;
}

bool Mouse::buttonPressed(unsigned int button) const{
    return m_currentStates[button] && !m_prevStates[button];
}