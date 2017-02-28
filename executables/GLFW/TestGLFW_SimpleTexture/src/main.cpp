#include <chaos/StandardHeaders.h> 
#include <chaos/FileTools.h> 
#include <chaos/LogTools.h> 
#include <chaos/GLTools.h> 
#include <chaos/GLTextureTools.h>
#include <chaos/MyGLFWGamepadManager.h> 
#include <chaos/MyGLFWWindow.h> 
#include <chaos/WinTools.h> 
#include <chaos/GLProgramLoader.h>
#include <chaos/GLProgramData.h>
#include <chaos/Application.h>
#include <chaos/SimpleMeshGenerator.h>
#include <chaos/SkyBoxTools.h>
#include <chaos/GeometryFramework.h>
#include <chaos/GLProgram.h>
#include <chaos/Texture.h>
#include <chaos/GLProgramVariableProvider.h>

class MyGLFWWindowOpenGLTest1 : public chaos::MyGLFWWindow
{

protected:

	virtual void OnKeyEvent(int key, int scan_code, int action, int modifier) override
	{
		if (key == GLFW_KEY_KP_ADD && action == GLFW_RELEASE)
		{
			ChangeTexture(texture_index + 1);
		}
		else if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_RELEASE)
		{
			ChangeTexture(texture_index - 1);
		}
		else if (key == GLFW_KEY_KP_ENTER && action == GLFW_RELEASE)
		{
			ChangeTextureLevel();
		}
	}

	void ChangeTextureLevel()
	{
		if (texture != nullptr)
		{
			chaos::ImageDescription desc = chaos::GLTextureTools::GetTextureImage(texture->GetResourceID(), 0);
			if (desc.data != nullptr)
			{
				boost::intrusive_ptr<chaos::Texture> new_texture = chaos::GLTextureTools::GenTextureObject(desc.GetSubImageDescription(0, 0, desc.width - 7, desc.height - 7));
				if (new_texture != nullptr)
					texture = new_texture;
				delete[](desc.data);
			}
		}
	}

	void ChangeTexture(int index)
	{
		boost::intrusive_ptr<chaos::Texture> new_texture = GenerateTexture(index);
		if (new_texture != nullptr)
		{
			texture_index = index;
			texture = new_texture;
		}
	}

	boost::intrusive_ptr<chaos::Texture> GenerateTexture(int index)
	{
		static int const GENERATE_RGBA = 0;
		static int const GENERATE_GRAY = 1;
		static int const GENERATE_RGB  = 2;
		static int const GENERATE_GRAY_FLOAT = 3;
		static int const GENERATE_RGB_FLOAT = 4;
		static int const GENERATE_RGBA_FLOAT = 5;
		static const int GENERATE_BACKGROUND = 6;
		static const int GENERATE_BACKGROUND_GRAY = 7;
		static const int GENERATE_FLOAT_BACKGROUND = 8;

		boost::intrusive_ptr<chaos::Texture> result;

		if (index == GENERATE_RGBA)
		{
			// test manual buffer
			char * image_buffer = new char[4 * 512 * 512];

			chaos::ImageDescription image_desc = chaos::ImageDescription(image_buffer, 512, 512, chaos::PixelFormat(chaos::PixelFormat::TYPE_UNSIGNED_CHAR, 4), 0);

			for (int i = 0; i < 512; ++i)
			{
				for (int j = 0; j < 512; ++j)
				{
					image_buffer[4 * j + 0 + (i * image_desc.pitch_size)] = i;
					image_buffer[4 * j + 1 + (i * image_desc.pitch_size)] = 0;
					image_buffer[4 * j + 2 + (i * image_desc.pitch_size)] = 0;
					image_buffer[4 * j + 3 + (i * image_desc.pitch_size)] = (char)127;
				}
			}
			result = chaos::GLTextureTools::GenTextureObject(image_desc);
			delete[](image_buffer);
		}

		// test GENERATION GRAY
		if (index == GENERATE_GRAY)
		{
			result = chaos::GLTextureTools::GenTextureObject<chaos::PixelGray>(512, 512, [](chaos::ImageDescription const & desc, chaos::PixelGray* buffer)
			{
				for (int i = 0; i < desc.height; ++i)
					for (int j = 0; j < desc.width; ++j)
						buffer[j + i * desc.width] = (unsigned char)i;
			});
		}
		// test GENERATION RGB
		if (index == GENERATE_RGB)
		{
			result = chaos::GLTextureTools::GenTextureObject<chaos::PixelBGR>(512, 512, [](chaos::ImageDescription const & desc, chaos::PixelBGR * buffer)
			{
				for (int i = 0; i < desc.height; ++i)
				{
					for (int j = 0; j < desc.width; ++j)
					{
						buffer[j + i * desc.width].R = (unsigned char)i;
						buffer[j + i * desc.width].G = 0;
						buffer[j + i * desc.width].B = 0;
					}
				}
			});
		}

		// test GENERATION GRAY FLOAT
		if (index == GENERATE_GRAY_FLOAT)
		{
			result = chaos::GLTextureTools::GenTextureObject<chaos::PixelGrayFloat>(512, 512, [](chaos::ImageDescription const & desc, chaos::PixelGrayFloat* buffer)
			{
				for (int i = 0; i < desc.height; ++i)
					for (int j = 0; j < desc.width; ++j)
						buffer[j + i * desc.width] = chaos::MathTools::CastAndDiv<float>(i, desc.height);
			});
		}

		// test GENERATION RGB FLOAT
		if (index == GENERATE_RGB_FLOAT)
		{
			result = chaos::GLTextureTools::GenTextureObject<chaos::PixelRGBFloat>(512, 512, [](chaos::ImageDescription const & desc, chaos::PixelRGBFloat * buffer)
			{
				for (int i = 0; i < desc.height; ++i)
				{
					for (int j = 0; j < desc.width; ++j)
					{
						buffer[j + i * desc.width].R = chaos::MathTools::CastAndDiv<float>(j, desc.width);
						buffer[j + i * desc.width].G = chaos::MathTools::CastAndDiv<float>(i, desc.height);
						buffer[j + i * desc.width].B = 0.0f;
					}
				}
			});
		}

		// test GENERATION RGBA FLOAT
		if (index == GENERATE_RGBA_FLOAT)
		{
			result = chaos::GLTextureTools::GenTextureObject<chaos::PixelRGBAFloat>(512, 512, [](chaos::ImageDescription const & desc, chaos::PixelRGBAFloat * buffer)
			{
				for (int i = 0; i < desc.height; ++i)
				{
					for (int j = 0; j < desc.width; ++j)
					{
						buffer[j + i * desc.width].R = chaos::MathTools::CastAndDiv<float>(j, desc.width);
						buffer[j + i * desc.width].G = 0.0f;
						buffer[j + i * desc.width].B = chaos::MathTools::CastAndDiv<float>(i, desc.height); 
						buffer[j + i * desc.width].A = 1.0f;
					}
				}
			});
		}

		// test for background
		if (index == GENERATE_BACKGROUND)
		{
			FIBITMAP * image = chaos::ImageTools::GenFreeImage(chaos::PixelFormat(chaos::PixelFormat::TYPE_UNSIGNED_CHAR, 4), 512, 512);
			if (image != nullptr)
			{
				chaos::ImageTools::FillImageBackground(image, glm::vec4(0.0f, 1.0f, 0.0f, 0.9f));

				result = chaos::GLTextureTools::GenTextureObject(image);
				FreeImage_Unload(image);
			}
		}

		if (index == GENERATE_BACKGROUND_GRAY)
		{
			FIBITMAP * image = chaos::ImageTools::GenFreeImage(chaos::PixelFormat(chaos::PixelFormat::TYPE_UNSIGNED_CHAR, 1), 512, 512);
			if (image != nullptr)
			{
				chaos::ImageTools::FillImageBackground(image, glm::vec4(1.0f, 0.0f, 0.0f, 0.9f));

				result = chaos::GLTextureTools::GenTextureObject(image);
				FreeImage_Unload(image);
			}
		}

		if (index == GENERATE_FLOAT_BACKGROUND)
		{
			FIBITMAP * image = chaos::ImageTools::GenFreeImage(chaos::PixelFormat(chaos::PixelFormat::TYPE_FLOAT, 4), 512, 512);
			if (image != nullptr)
			{
				chaos::ImageTools::FillImageBackground(image, glm::vec4(1.0f, 0.0f, 0.0f, 0.9f));

				result = chaos::GLTextureTools::GenTextureObject(image);
				FreeImage_Unload(image);
			}
		}

		return result;
	}

	virtual void OnMouseButton(int button, int action, int modifier) override
	{
		if (button == 0 && action == GLFW_RELEASE)
		{
			chaos::TextureDescription desc = texture->GetTextureDescription();

			int max_mipmap = chaos::GLTextureTools::GetMipmapLevelCount(desc.width, desc.height);

			mipmap_level = (mipmap_level + 1) % max_mipmap;
			glTextureParameteri(texture->GetResourceID(), GL_TEXTURE_BASE_LEVEL, mipmap_level); //GL_TEXTURE_MAX_LEVEL
		}
	}

	virtual bool OnDraw(int width, int height) override
	{
		glm::vec4 clear_color(0.0f, 0.0f, 0.0f, 0.0f);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);

		float far_plane = 1000.0f;
		glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);

		glViewport(0, 0, width, height);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		glUseProgram(program->GetResourceID());

		chaos::GLProgramData const & program_data = program->GetProgramData();

		chaos::GLProgramVariableProviderChain uniform_provider;
		uniform_provider.AddVariableTexture("material", texture);
		uniform_provider.AddVariableValue("screen_size", glm::vec2((float)width, (float)height));
		program_data.BindUniforms(&uniform_provider);

		mesh->Render(program_data, nullptr, 0, 0);

		return true;
	}

	virtual void Finalize() override
	{
		program = nullptr;
		mesh    = nullptr;
		texture = nullptr;
	}

	virtual bool Initialize() override
	{   
		chaos::Application * application = chaos::Application::GetInstance();
		if (application == nullptr)
			return false;

		boost::filesystem::path resources_path       = application->GetResourcesPath();
		boost::filesystem::path fragment_shader_path = resources_path / "pixel_shader.txt";
		boost::filesystem::path vertex_shader_path   = resources_path / "vertex_shader.txt";


		texture = GenerateTexture(texture_index);
		if (texture == nullptr)
			return false;

		chaos::GLProgramLoader loader;
		loader.AddShaderSourceFile(GL_FRAGMENT_SHADER, fragment_shader_path);
		loader.AddShaderSourceFile(GL_VERTEX_SHADER,   vertex_shader_path);

		program = loader.GenerateProgramObject();
		if (program == nullptr)
			return false;

		// create the mesh
		chaos::box2 b = chaos::box2(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));

		mesh = chaos::QuadMeshGenerator(b).GenerateMesh();
		if (mesh == nullptr)
			return false;

		return true;
	}

	virtual void TweakSingleWindowApplicationHints(chaos::MyGLFWWindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const override
	{
		chaos::MyGLFWWindow::TweakSingleWindowApplicationHints(hints, monitor, pseudo_fullscreen);
		hints.toplevel  = 0;
		hints.decorated = 1;
	}

protected:

	boost::intrusive_ptr<chaos::GLProgram>  program;
	boost::intrusive_ptr<chaos::SimpleMesh> mesh;
	boost::intrusive_ptr<chaos::Texture>    texture;

	int mipmap_level{ 0 };
	int texture_index{ 0 };
};



int _tmain(int argc, char ** argv, char ** env)
{
	chaos::Application::Initialize<chaos::Application>(argc, argv, env);

	chaos::WinTools::AllocConsoleAndRedirectStdOutput();

	chaos::MyGLFWSingleWindowApplicationParams params;
	params.monitor       = nullptr;
	params.width         = 500;
	params.height        = 500;
	params.monitor_index = 0;
	chaos::MyGLFWWindow::RunSingleWindowApplication<MyGLFWWindowOpenGLTest1>(params);

	chaos::Application::Finalize();

	return 0;
}


