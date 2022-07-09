#pragma once
#include "Test.h"

#include "geometry/Object.h"
#include "renderer/Renderer.h"

namespace test
{

    class TestRenderer : public Test
    {
    public:
        TestRenderer(Window& window);
        ~TestRenderer() {}

        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;
    private:
        Object m_Cube;
        Object m_LightCube;
        Camera m_Camera;
        float m_CamSpeed;
        Ref<UBO> m_CameraUbo;
        Ref<UBO> m_LightUbo;
        glm::vec3 m_LightPos;
    };
}