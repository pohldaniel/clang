#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>

#include <GLFW/glfw3.h>

#include <WebGPU/WgpContext.h>
#include <States/StateMachine.h>
#include <States/Wireframe.h>

#include "Mouse.h"
#include "Application.h"

GLFWwindow* Application::Window = nullptr;
StateMachine* Application::Machine = nullptr;
int Application::Width;
int Application::Height;
double Application::Time;
bool Application::Init = false;

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void glfwWindowScroll(GLFWwindow* window, double xoffset, double yoffset);
void glfwWindowResizeCallback(GLFWwindow* window, int width, int height);
void glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height);

void Application::MessageLopp(void *arg) {
  Application* application  = reinterpret_cast<Application*>(arg);

  Time = glfwGetTime();
  application->dt = float(Time - application->last);
  application->last = Time;

  application->messageLopp();
}

Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
  Application::Width = 1260;
  Application::Height = 720;

  initWindow();
  initWebGPU();
  initImGUI();
  initStates();

  glfwSetWindowUserPointer(Window, this);
  glfwSetFramebufferSizeCallback(Window, glfwFramebufferResizeCallback);
  glfwSetCursorPosCallback(Window, glfwMouseMoveCallback);
  glfwSetMouseButtonCallback(Window, glfwMouseButtonCallback);
  glfwSetScrollCallback(Window, glfwWindowScroll);

  Application::Init = true;
}

Application::~Application() {
  Cleanup();
}

void Application::initWindow() {
  if (!glfwInit()) {
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
  Window = glfwCreateWindow(Width, Height, "WebGPU window", nullptr, nullptr);
}

void Application::initWebGPU(){
  wgpInit(Window, 4u);
}

void Application::initImGUI() {
	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = NULL;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ImGui_ImplWGPU_InitInfo initInfo = {};
	initInfo.Device = wgpContext.device;
	initInfo.RenderTargetFormat = wgpContext.colorformat;
	initInfo.DepthStencilFormat = wgpContext.depthformat;
  initInfo.PipelineMultisampleState.count = wgpContext.msaaSampleCount;

	ImGui_ImplGlfw_InitForOther(Window, true);
	ImGui_ImplWGPU_Init(&initInfo);
}

void Application::initStates(){
  Machine = new StateMachine(dt, fdt);
	Machine->addStateAtTop(new Wireframe(*Machine));
}

bool Application::isRunning(){
  messageLopp();
  return !glfwWindowShouldClose(Window);
}

void Application::messageLopp(){
    glfwPollEvents();
    Mouse::instance().update();
    Machine->update();
    Machine->render();
}

void Application::Resize(uint32_t width, uint32_t height){
  if(Init){
    wgpResize(Width, Height);
    Machine->getStates().top()->resize(0, 0);
  }
}

bool Application::IsInitialized(){
  return Init;
}

void Application::Cleanup(){
  delete Machine;
  wgpShutDown();
  ImGui_ImplWGPU_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  glfwDestroyWindow(Window);
  glfwTerminate();
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if(ImGui::GetIO().WantCaptureMouse){  
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    return;
  }

  switch(key){
    case (GLFW_KEY_Z - 1):
      if(action == GLFW_PRESS)
        Application::Machine->ToggleWireframe();
    return;
    case GLFW_KEY_V:
      if(action == GLFW_PRESS)
        wgpToggleVerticalSync();
    return;
    default:{
      Event event;
      event.data.keyboard.keyCode = key;

      if (action == GLFW_PRESS){
        event.type = Event::KEYDOWN;
        Application::Machine->getStates().top()->OnKeyDown(event.data.keyboard);
      }

      if(action == GLFW_RELEASE){
        event.type = Event::KEYUP;
        Application::Machine->getStates().top()->OnKeyUp(event.data.keyboard);
      }
    }
  }
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  if(ImGui::GetIO().WantCaptureMouse && !Mouse::instance().isAttached()){
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  }else{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    Event event; 
    event.data.mouseButton.x = static_cast<int>(xpos);
    event.data.mouseButton.y = static_cast<int>(ypos);
    event.data.mouseButton.button = (button == GLFW_MOUSE_BUTTON_RIGHT) ? Event::MouseButtonEvent::MouseButton::BUTTON_RIGHT : Event::MouseButtonEvent::MouseButton::BUTTON_LEFT;

    if (action == GLFW_PRESS){
      event.type = Event::MOUSEBUTTONDOWN;
      Application::Machine->getStates().top()->OnMouseButtonDown(event.data.mouseButton);
    }

    if(action == GLFW_RELEASE){
      event.type = Event::MOUSEBUTTONUP;
      Application::Machine->getStates().top()->OnMouseButtonUp(event.data.mouseButton);
    }
  }
}

void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	Event event;
  event.type = Event::MOUSEMOTION;
  event.data.mouseMove.x = static_cast<int>(xpos);
  event.data.mouseMove.y = static_cast<int>(ypos);
    
  Application::Machine->getStates().top()->OnMouseMotion(event.data.mouseMove);
}

void glfwWindowScroll(GLFWwindow* window, double xoffset, double yoffset) {
	Application::Machine->getStates().top()->OnScroll(xoffset, yoffset);
}

void glfwWindowResizeCallback(GLFWwindow* window, int width, int height){

}

void glfwFramebufferResizeCallback(GLFWwindow* window, int width , int height) {
	glfwSetWindowSize(window, width, height);
  Application::Width = static_cast<int>(width);
  Application::Height = static_cast<int>(height);
  Application::Resize(Application::Width, Application::Height);
}
