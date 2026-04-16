#ifndef __mouseH__
#define __mouseH__

struct GLFWwindow;

class Mouse{

public:

    static Mouse &instance();
    void update();
    
    void attach(GLFWwindow* window, bool hideCursor = true, bool reattach = false, bool reset = false);
    void detach();
    void hideCursor(bool hideCursor);
    const float xDelta() const;
    const float yDelta() const;
    const bool isAttached() const;
    
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
	bool m_reset;

	float m_centerX, m_centerY;
};

#endif