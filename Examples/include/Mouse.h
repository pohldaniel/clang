#pragma once

struct GLFWwindow;

class Mouse{

public:

    static Mouse &instance();
    void update();
    
    void attach(GLFWwindow* window, bool hideCursor = true, bool reset = false, bool reattach = false);
    void detach();
    void hideCursor(bool hideCursor);
    float xDelta() const;
    float yDelta() const;
    float xPos() const;
    float yPos() const;
    bool isAttached() const;
    bool isVisibile();

    bool buttonDown(unsigned int button) const;
    bool buttonDownInvisible(unsigned int button) const;

private:

    Mouse();
    ~Mouse();
   
    void setCursorToMiddle();
    
    GLFWwindow* m_window;
	double m_xPos, m_yPos, m_xPrevPos, m_yPrevPos;
	double m_xLastPos, m_yLastPos;
	int m_wheelDelta;
	int m_prevWheelDelta;
    float m_mouseWheel;
    float m_xDelta;
	float m_yDelta;

    bool m_cursorVisible;
    bool m_attached;

	double m_centerX, m_centerY;
};