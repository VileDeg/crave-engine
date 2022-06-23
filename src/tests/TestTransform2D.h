#pragma once
#include "Test.h"

#include "Renderer.h"
#include "Texture.h"

#include <memory>
#include <vector>

namespace test
{

    class TestTransform2D : public Test
    {
    public:
        TestTransform2D();
        ~TestTransform2D();

        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;
    private:
        std::unique_ptr<VAO> m_VAO;
        std::unique_ptr<VBO> m_VBO;
        std::unique_ptr<EBO> m_IndexBuffer;
        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<Texture> m_Texture;

        glm::mat4 m_Proj, m_View;
        glm::vec3 m_TranslationA, m_TranslationB;
    };
}