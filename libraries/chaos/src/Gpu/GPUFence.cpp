﻿#include <chaos/Chaos.h>

namespace chaos
{

	GPUFence::GPUFence(GLsync in_fence) :
		fence(in_fence)
	{		
	}

	GPUFence::GPUFence() // the GL object is not yet generated
	{
		CreateGPUFence();
	}

	GPUFence::~GPUFence()
	{
		DoRelease();
	}

	bool GPUFence::CreateGPUFence()
	{
		// resource already existing
		if (IsValid())
			return true;
		fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); // a fence object is pushed inside the OpenGL command queue
		return IsValid();
	}

	bool GPUFence::WaitForCompletion(float timeout)
	{
		if (fence == nullptr) // fence not initialized yet
			return false;

		GLuint64   timeout64 = (GLuint64)(timeout * 10.0e9);
		GLbitfield flags     = 0;  // GL_SYNC_FLUSH_COMMANDS_BIT ???

		GLenum result = glClientWaitSync(fence, flags, timeout64); // ??? glWaitSync glFlush
		if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
			return true;
		return false; // GL_TIMEOUT_EXPIRED or GL_WAIT_FAILED
	}


	bool GPUFence::DoRelease()
	{
		if (fence == nullptr)
			return false;
		glDeleteSync(fence);
		fence = nullptr;
		return true;	
	}

}; // namespace chaos