namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class Manager;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	// ==============================================================
	// MANAGER
	// ==============================================================

	class Manager : public Object
	{

	public:

		/** start the manager */
		bool StartManager();
		/** stop the manager */
		bool StopManager();

		/** whether the manager is started */
		bool IsManagerStarted() const;

		/** initialize the manager from a configuration file */
		virtual bool InitializeFromConfiguration(JSONConfig const & config);

	protected:

		/** internally starts the manager */
		virtual bool DoStartManager();
		/** internally stops the manager */
		virtual bool DoStopManager();

		/** an utility method to initialize a single object in an JSON array/object */
		template<typename LOADER>
		static void DoLoadObjectsRecurseDirectories(JSONConfig const & config, LOADER const& loader)
		{
			if (config.json.is_array())
			{
				for (nlohmann::json::const_iterator it = config.json.begin(); it != config.json.end(); ++it)
					if (it->is_string())
						DoLoadObjectsRecurseDirectories(JSONConfig(*it, config.config_path), loader); // recursive call to this function
			}
			else if (config.json.is_string())
			{
				std::string directory_name = config.json.get<std::string>();

				FileTools::ForEachRedirectedDirectoryContent(directory_name, [&loader](boost::filesystem::path const& p)
				{
					LOADER other_loader = loader;
					other_loader.LoadObject(p.string());
					return false; // don't stop
				});
			}
		}

		/** an utility method to initialize a single object in an JSON array/object */
		template<bool RECURSE, typename LOADER>
		static auto DoLoadObjectsFromConfiguration(char const* name, JSONConfig const & config, LOADER loader) -> typename LOADER::resource_type* // LOADER passed by copy is important to ensure reset for all loaded objects
		{
			// 1 - recurse over some directories
			if constexpr (RECURSE)
			{
				if (name != nullptr && StringTools::Stricmp(name, "[recurse]") == 0)
				{
					DoLoadObjectsRecurseDirectories(config, loader);
					return nullptr;
				}
			}
			// 2 - we receive a key and its is valid (starts with '@')
			if (name != nullptr && name[0] == '@' && name[1] != 0)
			{
				return loader.LoadObject(name + 1, config);
			}
			// 3 - try to find a member 'name'
			nlohmann::json::const_iterator name_json_it = config.json.find("name");
			if (name_json_it != config.json.end() && name_json_it->is_string())
			{
				std::string name = name_json_it->get<std::string>();
				if (!name.empty())
					return loader.LoadObject(name.c_str(), config);
			}
			// 4 - anonymous object
			return loader.LoadObject(nullptr, config);
		}

		/** an utility method to initialize a list of objects from a JSON object or array */
		template<bool RECURSE, typename LOADER>
		static bool LoadObjectsFromConfiguration(char const* object_names, JSONConfig const & config, LOADER loader) // LOADER passed by copy is important to ensure reset for all loaded objects
		{
			nlohmann::json const* objects_json = JSONTools::GetStructure(config.json, object_names);
			if (objects_json != nullptr)
			{
				for (nlohmann::json::const_iterator it = objects_json->begin(); it != objects_json->end(); ++it)
				{
					if (objects_json->is_array())
						DoLoadObjectsFromConfiguration<RECURSE>(nullptr, JSONConfig(*it, config.config_path), loader);
					else if (objects_json->is_object())
						DoLoadObjectsFromConfiguration<RECURSE>(it.key().c_str(), JSONConfig(*it, config.config_path), loader);
				}
			}
			return true;
		}

		/** utility function to remove an object from a list */
		template<typename T, typename FUNC>
		static void DoRemoveObject(size_t index, T& vector, FUNC remove_func)
		{
			// ensure the index is valid
			size_t count = vector.size();
			if (index >= count)
				return;
			// copy the intrusive_ptr to prevent the destruction
			auto object = vector[index];
			// remove the object from the array
			if (index != count - 1)
				vector[index] = vector[count - 1];
			vector.pop_back();
			// callback then let the unreferencement manage the object lifetime
			remove_func(object.get());
		}

		/** detach all elements from a list */
		template<typename T, typename FUNC>
		static void RemoveAllObjectsFromList(T& vector, FUNC remove_func)
		{
			while (vector.size() > 0)
			{
				// copy the intrusive_ptr to prevent the destruction
				auto object = vector.back();
				// remove the object from the array
				vector.pop_back();
				// callback then let the unreferencement manage the object lifetime
				remove_func(object.get());
			}
		}

		template<typename T, typename U>
		static size_t FindObjectIndexInVector(T* object, U const& vector)
		{
			assert(object != nullptr);
			size_t count = vector.size();
			for (size_t i = 0; i < count; ++i)
				if (vector[i].get() == object)
					return i;
			return count;
		}

		/** utility function to test whether an object can be inserted */
		bool CanAddObject(ObjectRequest request, std::function<bool(ObjectRequest)> can_add_func) const;

	protected:

		/** whether the manager is started */
		bool manager_started = false;
	};

#endif

}; // namespace chaos