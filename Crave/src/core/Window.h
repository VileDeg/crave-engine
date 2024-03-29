#pragma once

#include "renderer/Camera.h"
#include "input/Input.h"

struct GLFWwindow;

namespace Crave
{
    namespace Window
    {
        GLFWwindow* Open(const unsigned width, const unsigned height, const std::string& name);
        
        GLFWwindow* Handle();

        void SetCamera(Ref<Camera> cam);
        void OnUpdate();

        void SetViewportHovered(bool hovered);

        bool Paused();
        int Width();
        int Height();

        float DeltaTime();

        //bool KeyPressed(Key keyCode);
        bool IsOpen();
        bool CursorVisible();
        void HideCursor();
        void ShowCursor();
        void Close();
        void SetKeybinds();

        void GLFWTerminate();
        void GLFWSwapBuffers();
        void GLFWPollEvents();

        void CalcDeltaTime();
    };
}