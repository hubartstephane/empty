#pragma once

#include <chaos/StandardHeaders.h>

namespace chaos
{
	class JSONTools
	{
	public:

		/** parsing a JSON file (catch exceptions) */
		static nlohmann::json Parse(char const * buffer);

		/** reading an attribute (catch exceptions) */
		template<typename T>
		static bool GetAttribute(nlohmann::json entry, char const * name, T & result)
		{
			assert(name != nullptr);
			try
			{
				result = entry.value(name, result);
			}
			catch (...)
			{
				return false;
			}
			return true;
		}
		static bool GetAttribute(nlohmann::json entry, char const * name, bool & result) // sepcialization for bool
		{
			assert(name != nullptr);
			try
			{
				result = (entry.value(name, 0) > 0);
			}
			catch (...)
			{
				return false;
			}
			return true;
		}
		/** reading an attribute (catch exceptions) with default value */
		template<typename T, typename Y>
		static bool GetAttribute(nlohmann::json entry, char const * name, T & result, Y default_value)
		{
			assert(name != nullptr);
			try
			{
				result = entry.value(name, default_value);
			}
			catch (...)
			{
				result = default_value;
				return false;
			}
			return true;
		}
		static bool GetAttribute(nlohmann::json entry, char const * name, bool & result, bool default_value) // specialization for bool
		{
			result = default_value;
			assert(name != nullptr);
			try
			{
				result = (entry.value(name, 0) > 0);
			}
			catch (...)
			{
				return false;
			}
			return true;
		}
	};

}; // namespace chaos
