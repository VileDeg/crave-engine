#pragma once

#include <glm/glm.hpp>
#include "renderer/Renderer.h"
#include "renderer/Mesh.h"
#include "renderer/Model.h"
#include "renderer/Light.h"

struct TagComponent
{
	std::string Tag;

	TagComponent() = default;
	TagComponent(const TagComponent&) = default;
	TagComponent(const std::string& tag)
		: Tag(tag) {}
};

struct TransformComponent
{
	glm::mat4 Transform{ 1.0f };
	float Scale{ 1.f };

	void TranslateTo(const glm::vec3& pos)
	{
		Transform[3] = glm::vec4(pos, Transform[3].w);
	}
	
	void ScaleTo(const float units)
	{
		Transform = glm::scale(Transform, glm::vec3(1.f / Scale));
		Transform = glm::scale(Transform, glm::vec3(units));
		Scale = units;
	}

	void RotateTo(const float angle, const glm::vec3& axis)
	{
		Transform = glm::scale(Transform, glm::vec3(1.f / Scale));
		auto translate = Transform[3];
		Transform[3] = { 0.f, 0.f, 0.f, translate.w };

		Transform = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::normalize(axis));
		Transform[3] = translate;
		Transform = glm::scale(Transform, glm::vec3(Scale));
	}

	const glm::vec3& Position() const
	{
		return Transform[3];
	}

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::mat4& transform)
		: Transform(transform) {}

	operator glm::mat4& () { return Transform; }
	operator const glm::mat4& () const { return Transform; }
};

struct ModelComponent
{
	std::vector<MeshInstance> mis;

	void Draw(const glm::mat4& modelMat)
	{
		for (auto& mesh : mis)
		{
			Renderer::Draw(modelMat, mesh);
		}
	}

	ModelComponent() = default;
	ModelComponent(Mesh& mesh, bool hasTex = true, bool normOut = true)
	{
		mis.push_back({ mesh, hasTex, normOut });
	}
	ModelComponent(Model model, bool hasTex = true, bool normOut = true)
	{
		for (auto& mesh : model.Meshes())
			mis.push_back({ mesh, hasTex, normOut });
	}
	ModelComponent(const ModelComponent& mc) : mis(mc.mis) {}
	ModelComponent(ModelComponent&& mc) : mis(mc.mis) {}

	ModelComponent& operator=(ModelComponent& mc)
	{ 
		if (this == &mc)
			return *this;
		mis = mc.mis;
		return *this;
	};

	operator std::vector<MeshInstance>() { return mis; }
	operator const std::vector<MeshInstance>() const { return mis; }
};

struct LightComponent
{
	Light Data;
	bool  IsDynamic{ false };
	unsigned SSBOindex;

	void UpdatePosition(const glm::vec3& pos)
	{
		Renderer::UpdateLightPosition(glm::value_ptr(pos), SSBOindex);
	}

	LightComponent() = default;
	LightComponent(const Light& lightData, bool isDynamic = false)
		: Data(lightData), IsDynamic(isDynamic)
	{
		SSBOindex = Renderer::UploadLightData(&Data);
	}
};