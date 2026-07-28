
#include "Keyboard.h"
#include "Application.h"

Keyboard &Keyboard::instance(){
	static Keyboard instance;
	return instance;
}

Keyboard::Keyboard(){

}

Keyboard::~Keyboard(){

}

void Keyboard::update(){
   std::memcpy(prevKeys, currentKeys, sizeof(currentKeys));

   for (int key = 0; key <= GLFW_KEY_LAST; ++key) {
         currentKeys[key] = (glfwGetKey(Application::Window, key) == GLFW_PRESS);
    }
}

bool Keyboard::keyDown(int key) const {
    return currentKeys[key];
}

bool Keyboard::keyPressed(int key) const {
    return currentKeys[key] && !prevKeys[key];
}

bool Keyboard::keyUp(int key) const {
    return !currentKeys[key];
}