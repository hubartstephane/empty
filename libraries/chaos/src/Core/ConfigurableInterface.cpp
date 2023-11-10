#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	// ---------------------------------------------------------------------
	// ConfigurableInterfaceBase implementation
	// ---------------------------------------------------------------------

	void ConfigurableInterfaceBase::SetObjectConfiguration(ObjectConfigurationBase* in_configuration)
	{
		if (configuration.get() != in_configuration)
		{
			// erase previous configuration
			if (configuration != nullptr)
				configuration->configurable_object = nullptr;
			// set new configuration
			configuration = in_configuration;
			if (configuration != nullptr)
				configuration->configurable_object = auto_cast(this);
		}
	}

	JSONReadConfiguration ConfigurableInterfaceBase::GetJSONReadConfiguration() const
	{
		if (configuration != nullptr)
			return configuration->GetJSONReadConfiguration();
		return {};
	}

	JSONWriteConfiguration ConfigurableInterfaceBase::GetJSONWriteConfiguration() const
	{
		if (configuration != nullptr)
			return configuration->GetJSONWriteConfiguration();
		return {};
	}

	bool ConfigurableInterfaceBase::GiveChildConfiguration(ConfigurableInterfaceBase* other_configurable, std::string_view key)
	{
		if (configuration != nullptr)
		{
			other_configurable->SetObjectConfiguration(configuration->CreateChildConfiguration(key));
			return true;
		}
		return false;
	}

}; // namespace chaos