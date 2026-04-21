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
	m_reset = false;
}

Mouse::~Mouse(){

}

void Mouse::update(){
    if (m_attached) {
        glfwGetCursorPos(m_window, &m_xPos, &m_yPos);
        m_xDelta = static_cast<float>(m_xPrevPos - m_xPos);
		m_yDelta = static_cast<float>(m_yPrevPos - m_yPos);

        m_xPrevPos = m_xPos;
        m_yPrevPos = m_yPos;       
    }
}

void Mouse::attach(GLFWwindow* window, bool _hideCursor, bool reattach, bool reset){

    if (m_attached && !reattach) return;
        m_window = window;

    glfwGetCursorPos(window, &m_xPos, &m_yPos);
    m_xPrevPos = m_xPos;
    m_yPrevPos = m_yPos;

	if (!m_reset) {
       m_xLastPos = m_xPos;
       m_yLastPos = m_yPos;
	}

    m_centerX = static_cast<float>(Application::Width / 2);
    m_centerY = static_cast<float>(Application::Height / 2);

    if (_hideCursor) {
        hideCursor(true);
        //setCursorToMiddle();
    }
	m_attached = true;
    m_reset = reset;
}

void Mouse::detach() {
    if (!m_attached) return;	
      m_attached = false;

	if (!m_cursorVisible) {
		//glfwSetCursorPos(m_window, m_xLastPos, m_yLastPos);
		hideCursor(false);
	}

	m_xDelta = 0.0f;
	m_yDelta = 0.0f;
	m_window = nullptr;
}

void Mouse::setCursorToMiddle(){
    glfwSetCursorPos(m_window, static_cast<float>(m_centerX), static_cast<float>(m_centerY));
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

const float Mouse::xDelta() const {
    return m_xDelta;
}

const float Mouse::yDelta() const {
    return m_yDelta;
}

const bool Mouse::isAttached() const {
    return m_attached;
}