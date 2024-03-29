#include "pch.h"
#include "renderer/Texture.h"
#include "glad/glad.h"
#include "stb_image.h"

namespace Crave
{
	Texture::Texture(const std::string& texName, bool useRelativePath)
		: m_Id(-1), m_BoundSlot(-1), m_Target(GL_TEXTURE_2D)
	{
		glGenTextures(1, &m_Id);
		glBindTexture(GL_TEXTURE_2D, m_Id);
		stbi_set_flip_vertically_on_load(1);
		std::string fullPath = texName;
		if (useRelativePath)
			fullPath = BASE_TEXTURE_PATH + fullPath;
		int width, height, BPP;
		unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &BPP, 4);
		if (!data)
			std::cerr << "Error: Failed to load texture! " << fullPath << std::endl;

		// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glBindTexture(GL_TEXTURE_2D, 0);
		if (data)
			stbi_image_free(data);
	}

	Texture::Texture(const std::string& folderName, const char* faces[])
		: m_Id(-1), m_BoundSlot(-1), m_Target(GL_TEXTURE_CUBE_MAP)
	{
		std::string basePath = BASE_CUBEMAP_PATH + folderName;
		std::string fullPath = basePath;
		glGenTextures(1, &m_Id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_Id);
		stbi_set_flip_vertically_on_load(false);

		int width, height, BPP;
		for (unsigned i = 0; i < 6; i++)
		{
			fullPath = basePath + '/' + faces[i];

			unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &BPP, 4);
			if (!data)
			{
				std::cerr << "Error: Failed to load texture! " << fullPath << std::endl;
				stbi_image_free(data);
				return;
			}
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	Texture::Texture(glm::vec2 dimensions, Config config)
		: m_Id(-1), m_BoundSlot(-1)
	{
		m_Target = (int)config.target;

		glGenTextures(1, &m_Id);
		
		glBindTexture(m_Target, m_Id);

		switch (config.target)
		{
		case Target::Texture2D:
			switch (config.type)
			{
			case Type::RGBA:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				break;
			case Type::Depth:
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, dimensions.x, dimensions.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
				break;
			}
			case Type::DepthNStencil:
				glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, dimensions.x, dimensions.y);
				break;
			case Type::Integer:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, dimensions.x, dimensions.y, 0, GL_RED_INTEGER, GL_INT, nullptr);
				break;
			default:
				ASSERT(false, "Unsupported type");
			}

			break;
		case Target::TextureCubeMap:
			switch (config.type)
			{
			case Type::RGBA:
				ASSERT(false, "Unsupported type");
				break;
			case Type::Depth:
				for (unsigned int i = 0; i < 6; ++i)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
						dimensions.x, dimensions.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				break;
			case Type::Integer:
				ASSERT(false, "Unsupported type");
				break;
			default:
				ASSERT(false, "Unsupported type");
			}

			break;
		default:
			ASSERT(false, "Unsopported target");
		}
		if (config.filter != MMFilter::None)
		{
			glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, (int)config.filter); 
			glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, (int)config.filter); 

		}
		if (config.wrapMode != WrapMode::None)
		{
			glTexParameteri(m_Target, GL_TEXTURE_WRAP_S,	   (int)config.wrapMode); 
			glTexParameteri(m_Target, GL_TEXTURE_WRAP_T,	   (int)config.wrapMode);
			glTexParameteri(m_Target, GL_TEXTURE_WRAP_R,	   (int)config.wrapMode);
		}
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_Id);
	}

	void Texture::Bind(int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(m_Target, m_Id);
		m_BoundSlot = slot;
	}

	void Texture::Unbind() const
	{
		glBindTexture(m_Target, 0);
	}

}