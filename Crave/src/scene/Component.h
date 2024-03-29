#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer/Renderer.h"
#include "renderer/Mesh.h"

#include "scene/Scene.h"

namespace Crave
{
	namespace Component
	{
		constexpr const int NUMBER_OF_COMPONENTS = 4;
		constexpr const char* COMPONENT_LIST[NUMBER_OF_COMPONENTS] = {
			"Tag", "Transform", "MeshInstance", "Light"
		};

		struct Tag
		{
			std::string String;

			Tag() = default;
			Tag(const Tag&) = default;
			Tag(const std::string& tag)
				: String(tag) {}

			friend bool operator==(const Tag& lhs, const Tag& rhs)
			{
				return lhs.String == rhs.String;
			}
		};

		struct Transform
		{
			Entity Owner{};
			Entity Parent{};

			std::vector<Entity> Children{};

			glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
			glm::quat Quaternion{};
			glm::vec3 EulerAngles{};
			glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

			glm::mat4 PrevTransform{};

			bool UpdatedLastFrame() 
			{ 
				glm::mat4 tr = GetTransform();
				bool wasUpdated = tr != PrevTransform;
				PrevTransform = tr;
				return wasUpdated;
			}

			void ScaleF(const float units);

			void RotateTo(const float angle, const glm::vec3& axis);

			void RotateTo(const glm::vec3& eulerAngles);

			void RotateAroundPoint(const glm::vec3& point, const float angle, const glm::vec3& axis);

			glm::mat4 GetTransform() const;
			glm::mat4 GetTransformNoParent() const;
			glm::mat4 GetParentTransform() const;

			Transform(Entity owner, Entity parent);

			Transform(const Transform&) = default;
			Transform(const glm::vec3& pos)
				: Position(pos) {}

			friend bool operator==(const Transform& lhs, const Transform& rhs)
			{
				return lhs.Owner == rhs.Owner && lhs.Parent == rhs.Parent &&
					lhs.Children == rhs.Children && lhs.Position == rhs.Position &&
					lhs.Quaternion == rhs.Quaternion && lhs.EulerAngles == rhs.EulerAngles &&
					lhs.Scale == rhs.Scale;
			}

			friend bool operator!=(const Transform& lhs, const Transform& rhs)
			{
				return !(lhs == rhs);
			}

			operator glm::mat4() { return GetTransform(); }
			operator const glm::mat4() const { return GetTransform(); }
		};

		struct MeshInstance
		{
			Ref<Mesh> PMesh{};
			bool HasTextures{ true };
			glm::vec4 Color{ 1.f, 0.f, 1.f, 1.f }; //magenta

			void Draw(int drawID, const glm::mat4& modelMat)
			{
				Renderer::DrawMesh(drawID, modelMat, PMesh, HasTextures, Color);
			}

			void DrawOutlined(int drawID, const glm::mat4& modelMat)
			{
				Renderer::DrawOutlined(drawID, modelMat, PMesh, HasTextures, Color);
			}

			MeshInstance() = default;
			MeshInstance(const MeshInstance&) = default;
			MeshInstance(Ref<Mesh> mesh, bool hasTex = true)
				: PMesh(mesh), HasTextures(hasTex)
			{
			}
		};

		struct Light
		{
			LightData Data{};
			bool	  Enabled{ true };
			bool	  IsDynamic{ false };
			unsigned  ShaderIndex{};
		
			
			void UpdateViewMat(const glm::mat4& transform);

			void SubmitDataToRenderer()
			{
				if (Enabled)
					Renderer::SubmitLightData(Data, ShaderIndex);
			}

			Light()
				: Data(Renderer::GetDefaultLightData(LightType::Point)), IsDynamic(true)
			{
				ShaderIndex = Renderer::AddNewLight(Data);
			}
			//More suitable for dynamic lights
			//Position, direction etc. will be updated according to transform component.
			Light(LightType type, bool isDynamic)
				: Data(Renderer::GetDefaultLightData(type)), IsDynamic(isDynamic)
			{
				ShaderIndex = Renderer::AddNewLight(Data);
			}
			//More suitable for non-dynamic light since it won't be updated with transform
			//Allows to directly set light's position, direction etc.
			Light(const LightData& lightData, bool isDynamic)
				: Data(lightData), IsDynamic(isDynamic)
			{
				ShaderIndex = Renderer::AddNewLight(Data);
				
			}
			
		};
	}
}