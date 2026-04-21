#include "Application.h"

#ifdef WASM
  #include <emscripten.h>

  extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void Resize(int width, int height){
      Application::Resize(width, height);
    }

    EMSCRIPTEN_KEEPALIVE
    bool IsInitialized(){
      return Application::IsInitialized();
    }

    EMSCRIPTEN_KEEPALIVE
    void Cleanup(){
      emscripten_cancel_main_loop();
      Application::Cleanup();
    }

    EMSCRIPTEN_KEEPALIVE
    int GetWidth(){
      return Application::Width;
    }

    EMSCRIPTEN_KEEPALIVE
    int GetHeight(){
      return Application::Height;
    }
  }
#endif

int main(int argc, const char* argv[]) {
  float deltaTime = 0.0f;
	float fixedDeltaTime = 0.0f;
  Application application(deltaTime, fixedDeltaTime);

#ifdef WASM
  emscripten_set_main_loop_arg(Application::MessageLopp, &application, 0, true);
#else
  while(application.isRunning()){}
#endif

  return 0;
}