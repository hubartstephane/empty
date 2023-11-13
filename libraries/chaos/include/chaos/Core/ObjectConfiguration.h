namespace chaos
{

#ifdef CHAOS_FORWARD_DECLARATION

	class ObjectConfigurationBase;
	class ChildObjectConfiguration;
	class RootObjectConfiguration;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* ObjectConfigurationBase: some configuration that may be used for reading and writing (for persistent data)
	*/

	class CHAOS_API ObjectConfigurationBase : public Object
	{
		friend class ConfigurableInterfaceBase;
		friend class ChildObjectConfiguration;
		friend class RootObjectConfiguration;

	public:

		/** create a child configuration */
		ChildObjectConfiguration* CreateChildConfiguration(std::string key);

		/** get the interface used for reading */
		JSONReadConfiguration GetJSONReadConfiguration() const;
		/** get the interface used for reading & writing*/
		JSONWriteConfiguration GetJSONWriteConfiguration() const;

		/** gets the root configuration */
		RootObjectConfiguration* GetRootConfiguration();
		/** gets the root configuration */
		RootObjectConfiguration const* GetRootConfiguration() const;

		/** recursively send notifications */
		void PropagateNotifications();

		/** reload the read configuration part. Only affect current node and its children value (not its parents nor siblings nodes) */
		bool Reload(bool send_notifications);

	protected:

		/** call this whenever the configuration is being changed */
		void CompleteUpdateOperation(bool send_notifications);
		/** helper function used to reload the hierarchy */
		static nlohmann::json const* ReloadHelper(nlohmann::json& new_root_storage, ObjectConfigurationBase* src, std::string_view in_key);

	protected:

		/** cannot be construct except by CreateChildConfiguration(...) */
		ObjectConfigurationBase() = default;

		/** the children configurations */
		std::vector<shared_ptr<ChildObjectConfiguration>> child_configurations;
		/** the configurable object owning this */
		weak_ptr<Object> configurable_object;

		/** the json node to read info from */
		nlohmann::json* read_config = nullptr;
		/** the json node to write info into */
		nlohmann::json* write_config = nullptr;

		/** the storage for json node to read info from */
		nlohmann::json storage_read_config;
	};


	/**
	* ChildObjectConfiguration: a specialization that has a parent node and whose content depend on that
	*/

	class CHAOS_API ChildObjectConfiguration : public ObjectConfigurationBase
	{
		friend class ObjectConfigurationBase;
		friend class RootObjectConfiguration;

	public:

		/** destructor */
		virtual ~ChildObjectConfiguration();
		/** override */
		virtual void SubReference() override;

	protected:

		/** cannot be construct except by CreateChildConfiguration(...) */
		ChildObjectConfiguration() = default;

		/** recursively update child configuration */
		void PropagateUpdates();
		/** update json nodes from parent configuration */
		void UpdateFromParent();
		/** remove the configuration from its parent */
		void RemoveFromParent();

	protected:

		/** the parent configuration */
		weak_ptr<ObjectConfigurationBase> parent_configuration;
		/** the path from parent to this configuration */
		std::string key;
	};

	/**
	* RootObjectConfiguration: a specialization store the whole JSON hierarchy and lend sub-content to its children
	*/

	class CHAOS_API RootObjectConfiguration : public ObjectConfigurationBase
	{
		friend class ObjectConfigurationBase;

	public:

		/** constructor */
		RootObjectConfiguration();

		/** change the read config path */
		void SetReadConfigPath(FilePathParam const& in_read_config_path);
		/** change the write config path */
		void SetWriteConfigPath(FilePathParam const& in_write_config_path);

		/** read config from files */
		bool LoadConfigurations(bool load_read = true, bool load_write = true, bool send_notifications = true);
		/** save the persistent data */
		bool SaveWriteConfiguration();

	protected:

		/** the path for the read configuration */
		boost::filesystem::path read_config_path;
		/** the path for the write configuration */
		boost::filesystem::path write_config_path;

		/** the storage for json node to write info into */
		nlohmann::json storage_write_config;
	};

#endif

}; // namespace chaos