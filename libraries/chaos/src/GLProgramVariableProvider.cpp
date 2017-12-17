﻿#include <chaos/GLProgramVariableProvider.h>
#include <chaos/GLTools.h>

namespace chaos
{

  bool GLProgramVariableProviderTexture::DoProcessAction(char const * name, GLProgramVariableAction & action, GLProgramVariableProvider const * top_provider) const
  {
    if (handled_name != name)
      return false;
    return action.Process(name, value.get());
  }

  void GLProgramVariableProviderChain::Clear()
  {
    children_providers.clear();
  }

  void GLProgramVariableProviderChain::AddVariableProvider(GLProgramVariableProvider * provider)
  {
    if (provider != nullptr)
      children_providers.push_back(provider);
  }

  bool GLProgramVariableProviderChain::DoProcessAction(char const * name, GLProgramVariableAction & action, GLProgramVariableProvider const * top_provider) const
  {
    // handle children providers
    size_t count = children_providers.size();
    for (size_t i = 0; i < count; ++i)
      if (children_providers[i].get()->DoProcessAction(name, action, top_provider))
        return true;
    // failure
    return false;
  }
}; // namespace chaos
