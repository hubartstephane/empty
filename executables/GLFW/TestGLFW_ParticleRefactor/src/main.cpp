#include <chaos/StandardHeaders.h> 
#include <chaos/FileTools.h> 
#include <chaos/Buffer.h> 
#include <chaos/LogTools.h> 
#include <chaos/MyGLFWSingleWindowApplication.h> 
#include <chaos/MyGLFWWindow.h> 
#include <chaos/WinTools.h> 
#include <chaos/Application.h>
#include <chaos/IrrklangTools.h>
#include <chaos/MathTools.h>
#include <chaos/SoundManager.h>
#include <chaos/JSONTools.h>
#include <chaos/FileTools.h>
#include <chaos/GLTextureTools.h>
#include <chaos/TextureLoader.h>
#include <chaos/GPUProgramLoader.h>
#include <chaos/VertexDeclaration.h>
#include <chaos/ParticleTools.h>
#include <chaos/Hotpoint.h>

#include <chaos/ParticleManager.h>


// ==============================================================
// Particles 
// ==============================================================

class ParticleExample
{
public:

	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec2 size;
	chaos::ParticleTexcoords texcoords;
	float lifetime;
	float remaining_time;
};

class VertexExample
{
public:

	glm::vec2 position;
	glm::vec3 texcoord;
	glm::vec4 color;
};

class ParticleExampleTrait : public chaos::ParticleLayerTrait<ParticleExample, VertexExample>
{
public:

	bool UpdateParticle(float delta_time, ParticleExample * particle)
	{
		particle->position += particle->velocity * delta_time;
		particle->remaining_time -= delta_time;
		return (particle->remaining_time <= 0.0f);
	}

	size_t ParticleToVertex(ParticleExample const * particle, VertexExample * vertices, size_t vertices_per_particle) const
	{
		chaos::ParticleCorners corners = chaos::ParticleTools::GetParticleCorners(particle->position, particle->size, chaos::Hotpoint::CENTER);

		chaos::ParticleTools::GenerateBoxParticle(corners, particle->texcoords, vertices);

		float alpha = particle->remaining_time / particle->lifetime;
		for (size_t i = 0 ; i < 6 ; ++i)
			vertices[i].color = glm::vec4(1.0f, 0.5f, 0.25f, alpha);
		
		return vertices_per_particle;
	}

	chaos::VertexDeclaration GetVertexDeclaration() const
	{
		chaos::VertexDeclaration result;
		result.Push(chaos::SEMANTIC_POSITION, 0, chaos::TYPE_FLOAT2);
		result.Push(chaos::SEMANTIC_TEXCOORD, 0, chaos::TYPE_FLOAT3);
		result.Push(chaos::SEMANTIC_COLOR,    0, chaos::TYPE_FLOAT4);
		return result;
	}
};

using ParticleLayerDescExample = chaos::TParticleLayerDesc<ParticleExampleTrait>;







// ==============================================================
// Application 
// ==============================================================

class MyGLFWWindowOpenGLTest1 : public chaos::MyGLFW::Window
{
	

protected:

	virtual bool OnDraw(glm::ivec2 size) override
	{
		float VIEWPORT_WANTED_ASPECT = (16.0f / 9.0f);

		// clear the buffers
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float far_plane = 1000.0f;
		glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);
		
		// change  the viewport 
		chaos::GLTools::SetViewportWithAspect(size, VIEWPORT_WANTED_ASPECT);

		//
		chaos::DisableLastReferenceLost<chaos::GPUProgramProvider> uniform_provider;


		float WORLD_X = 1000.0f;
		glm::vec2 world_size     = glm::vec2(WORLD_X, WORLD_X / VIEWPORT_WANTED_ASPECT);
		glm::vec2 world_position = glm::vec2(0.0f, 0.0f);

		glm::vec3 scale = glm::vec3(2.0f / world_size.x, 2.0f / world_size.y, 1.0f);
		glm::vec3 tr    = glm::vec3(-world_position.x, -world_position.y, 0.0f); 

		glm::mat4 local_to_cam =  glm::scale(scale) * glm::translate(tr);

		uniform_provider.AddVariableValue("local_to_cam", local_to_cam);




		// draw
		particle_manager->Display(&uniform_provider);

		return true;
	}

	virtual void Finalize() override
	{
		particle_manager = nullptr;
		particle_layers.clear();
		range_allocations.clear();
	}

	virtual bool Tick(double delta_time) override
	{
		particle_manager->Tick((float)delta_time);

		return true; // no redraw
	}

	virtual void OnMouseButton(int button, int action, int modifier) override
	{




	}

	virtual bool InitializeFromConfiguration(nlohmann::json const & config, boost::filesystem::path const & config_path) override
	{
		chaos::MyGLFW::SingleWindowApplication * application = chaos::MyGLFW::SingleWindowApplication::GetGLFWApplicationInstance();
		if (application == nullptr)
			return false;

		chaos::GPUResourceManager * gpu_manager = application->GetGPUResourceManager();

		chaos::RenderMaterial * RM1 = gpu_manager->FindRenderMaterial("mat1");
		chaos::RenderMaterial * RM2 = gpu_manager->FindRenderMaterial("mat2");
		chaos::RenderMaterial * RM3 = gpu_manager->FindRenderMaterial("mat3");

		chaos::RenderMaterial * materials[] = { RM1, RM2, RM3 };

		particle_manager = new chaos::ParticleManager;

		int const LAYER_COUNT    = 5;
		int const MATERIAL_COUNT = 5;
		for (int i = 0; i < LAYER_COUNT; ++i)
		{
			for (int j = 0; j < MATERIAL_COUNT; ++j)
			{
				chaos::ParticleLayer * particle_layer = new chaos::ParticleLayer(new ParticleLayerDescExample());
				particle_layer->SetRenderOrder(i);
				particle_layer->SetLayerID(j + i * MATERIAL_COUNT);

				int material_index = rand() % 3;
				int particle_count = rand() % 50 + 5;

				particle_layer->SetRenderMaterial(materials[material_index]);

				chaos::ParticleRangeAllocation * range = particle_layer->SpawnParticlesAndKeepRange(particle_count);
				range_allocations.push_back(range);

				size_t pc = particle_layer->GetParticleCount(range->GetParticleRange());
				
				ParticleExample * particles = (ParticleExample*)particle_layer->GetParticleBuffer(range->GetParticleRange());
				if (particles)
					InitializeParticles(particles, pc);

				particle_manager->AddLayer(particle_layer);
			}
		}
		return true;
	}

	void InitializeParticles(ParticleExample * particles, size_t count)
	{
		float WORLD_SIZE = 1000.0f * 9.0f / 16.0f;

		for (size_t i = 0; i < count; ++i)
		{		
			float size  = WORLD_SIZE * chaos::MathTools::RandFloat() * 0.04f;
			float alpha = chaos::MathTools::RandFloat() * 6.28f;
			float speed = WORLD_SIZE * chaos::MathTools::RandFloat() * 0.1f;
			float lifetime = 2.0f + chaos::MathTools::RandFloat() * 2.0f;

			particles[i].position = glm::vec2(0.0f, 0.0f);
			particles[i].velocity = glm::vec2(
				speed * chaos::MathTools::Cos(alpha),
				speed * chaos::MathTools::Sin(alpha));
			particles[i].size = glm::vec2(size, size);
			particles[i].lifetime = lifetime;
			particles[i].remaining_time = lifetime;			
		}
		


	}

	virtual void TweakHints(chaos::MyGLFW::WindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const override
	{
		chaos::MyGLFW::Window::TweakHints(hints, monitor, pseudo_fullscreen);
		hints.toplevel = 0;
		hints.decorated = 1;
	}

protected:

	boost::intrusive_ptr<chaos::ParticleManager> particle_manager;

	std::vector<boost::intrusive_ptr<chaos::ParticleLayer>> particle_layers;
	std::vector<boost::intrusive_ptr<chaos::ParticleRangeAllocation>> range_allocations;
};



// ===============================================


int _tmain(int argc, char ** argv, char ** env)
{
	chaos::MyGLFW::SingleWindowApplicationParams params;
	params.monitor = nullptr;
	params.width = 500;
	params.height = 500;
	params.monitor_index = -1;
	chaos::MyGLFW::RunWindowApplication<MyGLFWWindowOpenGLTest1>(argc, argv, env, params);

	return 0;
}


