#pragma once

#include <unordered_map>
#include <GLFW/glfw3.h>

struct GLFWwindow;

class Keyboard {

public:

    static Keyboard &instance();
    void update();
    
    bool keyDown(int key) const;
    bool keyUp(int key) const ;
    bool keyPressed(int key) const;

private:

    Keyboard();
    ~Keyboard();
   
    inline static bool currentKeys[GLFW_KEY_LAST + 1] = { false };
    inline static bool prevKeys[GLFW_KEY_LAST + 1] = { false };
};