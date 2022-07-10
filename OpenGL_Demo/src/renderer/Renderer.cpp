#include "pch.h"
#include "renderer/Renderer.h"
#include "glad/glad.h"
#include "Light.h"
#include "geometry/GeoData.h"

Renderer::RenderData* Renderer::s_Data = nullptr;

static constexpr short DIFF_TEX_SLOT = 0;
static constexpr short SPEC_TEX_SLOT = 1;
static constexpr short SKYBOX_TEX_SLOT = 2;

void Renderer::SetUniformBuffer(const Ref<UBO> ubo, const short slot,
	std::vector<ShaderType> shTypes)
{
	ubo->Bind(slot);
	for (ShaderType type : shTypes)
	{
		auto shader = s_Data->Shader[type];
		unsigned index = glGetUniformBlockIndex(shader->Id(), ubo->Name());
		ASSERT(index != GL_INVALID_INDEX, "Uniform buffer not found");
		glUniformBlockBinding(shader->Id(), index, slot);
	}
}


void Renderer::Draw(const glm::mat4& modelMat, const Ref<VAO> vao,
	const Ref<Texture> diffuse)
{
	Ref<Shader> sh = s_Data->Shader[ShaderType::Diffuse];
	BindShader(sh);
	sh->setMat4f("u_ModelMat", modelMat);

	BindTexture(diffuse, DIFF_TEX_SLOT);
	BindVAO(vao);
	
	glDrawArrays(GL_TRIANGLES, 0, vao->Count());
}

void Renderer::Draw(const glm::mat4& modelMat, const Ref<VAO> vao,
	const Ref<Texture> diffuse, const Ref<Texture> specular)
{
	Ref<Shader> sh = s_Data->Shader[ShaderType::DiffNSpec];
	BindShader(sh);
	sh->setMat4f("u_ModelMat", modelMat);

	BindTexture(diffuse, DIFF_TEX_SLOT);
	BindTexture(specular, SPEC_TEX_SLOT);
	BindVAO(vao);

	glDrawArrays(GL_TRIANGLES, 0, vao->Count());
}

void Renderer::Draw(const glm::mat4& modelMat, const Ref<VAO> vao, const glm::vec4& color)
{
	Ref<Shader> sh = s_Data->Shader[ShaderType::Color];
	BindShader(sh);
	sh->setMat4f("u_ModelMat", modelMat);

	sh->setFloat4("u_Color", color);

	BindVAO(vao);
	glDrawArrays(GL_TRIANGLES, 0, vao->Count());
}

void Renderer::DrawSkybox()
{
	glDepthFunc(GL_LEQUAL);
	Ref<Shader> sh = s_Data->Shader[ShaderType::Skybox];
	BindShader(sh);
	//s_Data->Shader[ShaderType::Skybox]->setInt("u_SkyboxTex", SKYBOX_TEX_SLOT);
	sh->setMat4f("u_ProjMat", s_Data->ProjMat);
	sh->setMat4f("u_ViewMat",
		glm::mat4(glm::mat3(s_Data->ViewMat)));

	BindTexture(s_Data->skyboxData.SkyboxTex, SKYBOX_TEX_SLOT);
	BindVAO(s_Data->skyboxData.SkyboxVAO);

	glDrawArrays(GL_TRIANGLES, 0, s_Data->skyboxData.SkyboxVAO->Count());
	glDepthFunc(GL_LESS);
}




void Renderer::Init()
{
	s_Data = new RenderData();

	CreateShaders();
	CreateSkybox();

	s_Data->SceneUBO = CreateRef<UBO>(
		"SceneData", (const void*)NULL,
		sizeof(glm::mat4) + sizeof(glm::vec3));

	SetUniformBuffer(s_Data->SceneUBO, 0,
		{ ShaderType::Diffuse, ShaderType::DiffNSpec, ShaderType::Color });

	int lightsCount = 1;
	std::size_t dirSize = 12 * 4;
	std::size_t pointSize = 12 * 4 + 4*3;
	std::size_t spotSize = 12 * 5 + 4 * 5;
	s_Data->LightSSBO = CreateRef<UBO>(
		"LightData", (const void*)NULL,
		dirSize + spotSize + pointSize * lightsCount, GL_SHADER_STORAGE_BUFFER);

	SetUniformBuffer(s_Data->SceneUBO, 0,
		{ ShaderType::DiffNSpec });
}

void Renderer::CreateShaders()
{
	s_Data->Shader[ShaderType::Color] = CreateRef<Shader>("color.shader");

	s_Data->Shader[ShaderType::Diffuse] = CreateRef<Shader>("diffuse.shader");
	s_Data->Shader[ShaderType::Diffuse]->Bind();
	s_Data->Shader[ShaderType::Diffuse]->setInt(  "material.diffuse", DIFF_TEX_SLOT);
	s_Data->Shader[ShaderType::Diffuse]->setFloat("material.specular", 0.5f);
	s_Data->Shader[ShaderType::Diffuse]->setFloat("material.shininess", 32.0f);

	s_Data->Shader[ShaderType::DiffNSpec] = CreateRef<Shader>("diffNSpec.shader");
	s_Data->Shader[ShaderType::DiffNSpec]->Bind();
	s_Data->Shader[ShaderType::DiffNSpec]->setInt("material.diffuse", DIFF_TEX_SLOT);
	s_Data->Shader[ShaderType::DiffNSpec]->setInt("material.specular", SPEC_TEX_SLOT);
	s_Data->Shader[ShaderType::DiffNSpec]->setFloat("material.shininess", 32.0f);

	s_Data->Shader[ShaderType::Skybox] = CreateRef<Shader>("skybox.shader");
	s_Data->Shader[ShaderType::Skybox]->Bind();
	s_Data->Shader[ShaderType::Skybox]->setInt("u_SkyboxTex", SKYBOX_TEX_SLOT);
}

void Renderer::CreateSkybox()
{
	const char* SKYBOX_FACES[] = {
		"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
	};

	s_Data->skyboxData.SkyboxVAO = CreateRef<VAO>();
	s_Data->skyboxData.SkyboxVBO = CreateRef<VBO>(
		GeoData::SKYBOX_DATA, sizeof(GeoData::SKYBOX_DATA), 36);
	VertexLayout layout{ {GL_FLOAT, 3, GL_FALSE} };
	s_Data->skyboxData.SkyboxVBO->SetLayout(layout);
	s_Data->skyboxData.SkyboxVAO->AddBuffer(*s_Data->skyboxData.SkyboxVBO);
	s_Data->skyboxData.SkyboxTex = CreateRef<Texture>("skybox", SKYBOX_FACES);
}


void Renderer::UploadSpotlightData(const Spotlight& light)
{
	glBindBuffer(, s_Data->LightSSBO->Id());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(light), &light);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool Renderer::BindShader(const Ref<Shader> shader)
{
	if (s_Data->boundShaderId != shader->Id())
	{
		shader->Bind();
		s_Data->boundShaderId = shader->Id();
		return true;
	}
	return false;
}

void Renderer::BindVAO(const Ref<VAO> vao)
{
	if (s_Data->boundVaoId != vao->Id())
	{
		vao->Bind();
		s_Data->boundVaoId = vao->Id();
	}
}

void Renderer::BindTexture(const Ref<Texture> tex, const short slot)
{
	if (s_Data->TexSlotId[slot] != tex->Id())
	{
		tex->Bind(slot);
		s_Data->TexSlotId[slot] = tex->Id();
	}
}

void Renderer::BeginScene(const Camera& camera)
{
	s_Data->SceneUBO->Upload(
		glm::value_ptr(camera.GetProjViewMat()), sizeof(glm::mat4), 0);
	s_Data->SceneUBO->Upload(
		glm::value_ptr(camera.Position()), sizeof(glm::vec3), 64);

	s_Data->ViewMat = camera.GetViewMat();
	s_Data->ProjMat = camera.GetProjMat();
}

void Renderer::EndScene()
{
}

void Renderer::Shutdown()
{
	delete s_Data;
}


