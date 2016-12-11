﻿#include <chaos/GLProgram.h>
#include <chaos/GLProgramUniformProvider.h>
#include <chaos/GLProgramAttributeProvider.h>
 
namespace chaos
{

  GLProgram::GLProgram(GLuint in_id) : program_id(in_id)  
  {
    if (in_id != 0)
      program_data = GLProgramData::GetData(in_id);
  }

  GLProgram::~GLProgram()
  {
    Release();
  }

  void GLProgram::Release()
  {
    if (program_id != 0)
    {
      glDeleteProgram(program_id);
      program_id = 0;    
    }  
  }

  bool GLProgram::UseProgram(class GLProgramUniformProvider * uniform_provider, class GLProgramAttributeProvider * attribute_provider)
  {
    if (!IsValid())
      return false;

    glUseProgram(program_id);

    program_data.BindUniforms(uniform_provider);

    return true;
  }

}; // namespace chaos
