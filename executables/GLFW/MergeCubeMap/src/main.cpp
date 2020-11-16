#include <chaos/Chaos.h> 

std::vector<chaos::PixelFormat> pixel_formats =
{
	chaos::PixelFormat::Gray,
	chaos::PixelFormat::GrayFloat,
	chaos::PixelFormat::BGR,
	chaos::PixelFormat::BGRA,
	chaos::PixelFormat::RGBFloat,
	chaos::PixelFormat::RGBAFloat
};

class MyGLFWWindowOpenGLTest1 : public chaos::MyGLFW::Window
{

protected:

	virtual bool OnKeyEventImpl(chaos::KeyEvent const & event) override
	{
		if (event.IsKeyReleased(GLFW_KEY_KP_ADD))
		{
			ChangeSkyBox(skybox_index + 1);
			return true;
		}
		else if (event.IsKeyReleased(GLFW_KEY_KP_SUBTRACT))
		{
			ChangeSkyBox(skybox_index - 1);
			return true;
		}
		return chaos::MyGLFW::Window::OnKeyEventImpl(event);
	}

	void ChangeSkyBox(int index)
	{







		chaos::shared_ptr<chaos::GPUTexture> new_texture = GenerateSkyBox(index);
		if (new_texture != nullptr)
		{
			skybox_index = index;
			texture = new_texture;
			debug_display.Clear();

			chaos::PixelFormat pf = pixel_formats[index];
		
			char const * component_type = (pf.component_type == chaos::PixelComponentType::UNSIGNED_CHAR) ? "unsigned char" : "float";

			debug_display.AddLine(chaos::StringTools::Printf("format : index = [%d] component = [%d] type = [%s]", index, pf.component_count, component_type).c_str());
		}
	}

	chaos::shared_ptr<chaos::GPUTexture> GenerateSkyBox(int index)
	{
		if (index < 0 || index >= pixel_formats.size())
			return nullptr;

		chaos::PixelFormat pixel_format = pixel_formats[index];
		if (!pixel_format.IsValid())
			return nullptr;

		chaos::PixelFormatMergeParams merge_params;
		merge_params.pixel_format = pixel_format;
		
#if 0

		// let OpenGL do the conversion
		return chaos::GPUTextureLoader().GenTextureObject(&skybox, merge_params);

#else
		
		// do the conversion ourselves
		chaos::SkyBoxImages single_skybox = skybox.ToSingleImage(true, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), merge_params);

		if (!single_skybox.IsEmpty())
			return chaos::GPUTextureLoader().GenTextureObject(&single_skybox);				

#endif

		return nullptr;
	}

	virtual bool OnDraw(chaos::GPURenderer * renderer, chaos::box2 const & viewport, glm::ivec2 window_size) override
	{
		glm::vec4 clear_color(0.0f, 0.0f, 0.0f, 0.0f);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);

		float far_plane = 1000.0f;
		glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);   // when viewer is inside the cube

		// XXX : the scaling is used to avoid the near plane clipping      
		static float FOV =  60.0f;
		glm::mat4 projection_matrix      = glm::perspectiveFov(FOV * (float)M_PI / 180.0f, 2.0f * viewport.half_size.x, 2.0f * viewport.half_size.y, 1.0f, far_plane);
		glm::mat4 local_to_world_matrix  = glm::scale(glm::vec3(10.0f, 10.0f, 10.0f));
		glm::mat4 world_to_camera_matrix = fps_view_controller.GlobalToLocal();

		chaos::GPUProgramProvider uniform_provider;
		uniform_provider.AddVariableValue("projection",      projection_matrix);
		uniform_provider.AddVariableValue("local_to_world",  local_to_world_matrix);
		uniform_provider.AddVariableValue("world_to_camera", world_to_camera_matrix);
		uniform_provider.AddVariableTexture("material", texture);

		chaos::GPURenderParams render_params;
		mesh->Render(renderer, program.get(), &uniform_provider, render_params);

		debug_display.Display((int)(2.0f * viewport.half_size.x), (int)(2.0f * viewport.half_size.y));

		return true;
	}

	virtual void Finalize() override
	{
		skybox.Release();

		for (FIBITMAP * bitmap : skybox_bitmaps)
			FreeImage_Unload(bitmap);
		skybox_bitmaps.clear();

		program = nullptr;
		mesh    = nullptr;
		texture = nullptr;

		debug_display.Finalize();
		chaos::MyGLFW::Window::Finalize();
	}

	bool LoadSkyboxBitmaps(boost::filesystem::path const & resources_path)
	{
		// load the images
		boost::filesystem::directory_iterator end;
		for (boost::filesystem::directory_iterator it = chaos::FileTools::GetDirectoryIterator(resources_path / "images"); it != end; ++it)
		{
			FIBITMAP * bitmap = chaos::ImageTools::LoadImageFromFile(it->path());
			if (bitmap == nullptr)
				continue;
			skybox_bitmaps.push_back(bitmap);		
		}
		if (skybox_bitmaps.size() != 6)
			return false;

		// search the minimum size
		int size = -1;
		for (FIBITMAP * bitmap : skybox_bitmaps)
		{
			chaos::ImageDescription desc = chaos::ImageTools::GetImageDescription(bitmap);
			if (size < 0 || desc.width > size)
				size = desc.width;
			if (size < 0 || desc.height > size)
				size = desc.height;		
		}
		if (size <= 0)
			return false;

		// resize the image
		unsigned char c1[4] = { 0, 0, 0, 0 };
		glm::vec4 c2 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

		for (FIBITMAP * & bitmap : skybox_bitmaps)
		{
			//
			// FreeImage_EnlargeCanvas : enlarge or shrink image (returns a new image)
			//
			// FreeImage_CreateView : there is an issue, it created a new image that points the same buffer => at destruction of one, the buffer is release (no ref count)
			//

			chaos::ImageDescription desc = chaos::ImageTools::GetImageDescription(bitmap);

			void * color = (desc.pixel_format.component_type == chaos::PixelComponentType::UNSIGNED_CHAR) ? (void*)&c1[0] : (void*)&c2; // select a color for background

			int dx = size - desc.width;
			int dy = size - desc.height;

			FIBITMAP * old_bitmap = bitmap;
			bitmap = FreeImage_EnlargeCanvas(bitmap, dx / 2, dy / 2, dx - dx / 2, dy - dy / 2 , color);
			if (bitmap == nullptr)
				return false;

			FreeImage_Unload(old_bitmap);
		}

		// generate the skybox
		for (size_t i = 0; i < skybox_bitmaps.size() ; ++i)
		{
			skybox.SetImage((chaos::SkyBoxImageType)i, skybox_bitmaps[i], false);
		}
		return true;
	}

	virtual bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path) override
	{   
		if (!chaos::MyGLFW::Window::InitializeFromConfiguration(config, config_path))
			return false;

		chaos::Application * application = chaos::Application::GetInstance();
		if (application == nullptr)
			return false;

		boost::filesystem::path resources_path = application->GetResourcesPath();

		if (!LoadSkyboxBitmaps(resources_path))
			return false;

		boost::filesystem::path image_path  = resources_path / "font.png";

		chaos::GLDebugOnScreenDisplay::Params debug_params;
		debug_params.texture_path               = image_path;
		debug_params.font_characters            = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
		debug_params.font_characters_per_line   = 10;
		debug_params.font_characters_line_count = 10;
		debug_params.character_width            = 20;
		debug_params.spacing                    = glm::ivec2( 0, 0);
		debug_params.crop_texture               = glm::ivec2(15, 7);

		if (!debug_display.Initialize(debug_params))
			return false;

		debug_display.AddLine("Press +/- to change skybox");

		texture = GenerateSkyBox(0);
		if (texture == nullptr)
			return false;

		chaos::GPUProgramGenerator program_generator;
		program_generator.AddShaderSourceFile(GL_FRAGMENT_SHADER, resources_path / "pixel_shader_cube.txt");
		program_generator.AddShaderSourceFile(GL_VERTEX_SHADER,   resources_path / "vertex_shader.txt");

		program = program_generator.GenProgramObject();
		if (program == nullptr)
			return false;

		chaos::box3 b = chaos::box3(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

		mesh = chaos::GPUCubeMeshGenerator(b).GenerateMesh(); 
		if (mesh == nullptr)
			return false;

		return true;
	}

	virtual void TweakHints(chaos::MyGLFW::WindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const override
	{
		chaos::MyGLFW::Window::TweakHints(hints, monitor, pseudo_fullscreen);

		hints.toplevel  = 1;
		hints.decorated = 1;
	}

	virtual bool Tick(float delta_time) override
	{
		fps_view_controller.Tick(glfw_window, delta_time);

		debug_display.Tick(delta_time);

		return true; // refresh
	}

	virtual bool OnMouseButtonImpl(int button, int action, int modifier) override
	{
		if (button == 1 && action == GLFW_RELEASE)
		{
			debug_display.AddLine("HelloWorld");
			return true;
		}
		return false;
	}

protected:

	std::vector<FIBITMAP*> skybox_bitmaps;

	chaos::SkyBoxImages skybox;
			
	chaos::shared_ptr<chaos::GPUProgram>  program;
	chaos::shared_ptr<chaos::GPUSimpleMesh> mesh;
	chaos::shared_ptr<chaos::GPUTexture>    texture;

	chaos::FPSViewInputController fps_view_controller;

	chaos::GLDebugOnScreenDisplay debug_display;

	int skybox_index = 0;
};

int CHAOS_MAIN(int argc, char ** argv, char ** env)
{
	chaos::MyGLFW::WindowParams params;
	params.monitor = nullptr;
	params.width = 700;
	params.height = 700;
	params.monitor_index = 0;

	chaos::MyGLFW::WindowHints hints;

	return chaos::MyGLFW::RunWindowApplication<MyGLFWWindowOpenGLTest1>(argc, argv, env, params, hints);
}


