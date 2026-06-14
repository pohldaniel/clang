#pragma once

#include <stdint.h>

#define FIXED_STEP 0.0166666666666667f

struct GLFWwindow;
class StateMachine;

class Application {

	friend void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	friend void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	friend void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
	friend void glfwWindowScroll(GLFWwindow* window, double xpos, double ypos);
	friend void glfwFramebufferResizeCallback(GLFWwindow* m_window, int width , int height);


public:

	Application(float& dt, float& fdt);
	~Application();

	bool isRunning();

	static void MessageLoop(void *arg);
	static void Resize(uint32_t width, uint32_t height);
	static bool IsInitialized();
	static void OnSurfaceChange();
	static void Cleanup();
	
	static int Width;
	static int Height;
	static GLFWwindow* Window;
    
private:

	void initWindow();
	void initWebGPU();
	void initImGUI();
	void initStates();

	void fixedUpdate();
	void update();
	
	float& fdt;
	float& dt;
	double last;
	double accumulator;

	static StateMachine* Machine;
	static double Time;
	static bool Init;
};