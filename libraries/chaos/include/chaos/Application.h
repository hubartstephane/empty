#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/FileManager.h>
#include <chaos/InputMode.h>
#include <chaos/InputEventReceiver.h>
#include <chaos/ReferencedObject.h>
#include <chaos/AutoCast.h>

namespace chaos
{
	/**
	* Application : used to store generic application data
	*/
	class Application : public chaos::ReferencedObject, public InputEventReceiver
	{

	public:

		/** constructor */
		Application();
		/** destructor */
		virtual ~Application();

		/** the user main method */
		bool Run(int argc, char ** argv, char ** env);

		/** getter of the singleton instance */
		static AutoCastable<Application> GetInstance() { return singleton_instance; }
		/** getter of the singleton instance */
		static AutoConstCastable<Application> GetConstInstance() { return singleton_instance; }

		/** get the name of the application */
		char const * GetApplicationName() const { return application_name.c_str(); }

		/** get an environment value */
		char const * GetEnvironment(char const * key) const;
		/** get an environment value */
		static char const * GetApplicationEnvironment(char const * key);

		/** get the application arguments */
		inline std::vector<std::string> const & GetArguments() const { return arguments; }
		/** get the application path */
		inline boost::filesystem::path const & GetApplicationPath() const { return application_path; }
		/** get the resources path */
		inline boost::filesystem::path const & GetResourcesPath() const { return resources_path; }
		/** get the application name */
		inline boost::filesystem::path const & GetApplicationFilename() const { return application_filename; }
		/** get the application local path for execution data */
		inline boost::filesystem::path const & GetUserLocalPath() const { return userlocal_path; }
		/** get the application local temp path for execution data */
		inline boost::filesystem::path const & GetUserLocalTempPath() const { return userlocal_temp_path; }

		/** gets the file manager */
		FileManager * GetFileManager() { return file_manager.get(); }
		/** gets the file manager */
		FileManager const * GetFileManager() const { return file_manager.get(); }

		/** get the configuration */
		nlohmann::json const & GetConfiguration() const { return configuration;}
		/** get the configuration file */
		boost::filesystem::path const & GetConfigurationPath() const { return configuration_path; }

		/** open the temp local user directory (for debugging purpose) */
		boost::filesystem::path const & ShowUserLocalTempDirectory() const;	
		/** create the use local temp directory */
		boost::filesystem::path const & CreateUserLocalTempDirectory() const;

		/* get the current input mode (search the application) */
		static InputMode GetApplicationInputMode();
		/** change the current input mode (search the application) */
		static void SetApplicationInputMode(InputMode new_mode);

		/** check whether -flag_name is in command line */
		bool HasCommandLineFlag(char const * flag_name) const;
		/** check whether -flag_name is in command line */
		static bool HasApplicationCommandLineFlag(char const * flag_name);

		/** reloading the configuration file (do not apply it to any object at all) */
		bool ReloadConfigurationFile(nlohmann::json & result) const;
		
#if _DEBUG
		/** set redirection file directories */
		void SetFileRedirectionDirectories(boost::filesystem::path const & build_path, boost::filesystem::path const & src_path);
		/** get the redirected source directory */
		boost::filesystem::path const & GetRedirectionSourcePath() const { return redirection_source_path;};
		/** get the redirected build directory */
		boost::filesystem::path const & GetRedirectionBuildPath() const { return redirection_build_path; };
#endif

	protected:

		/** the method that should be override */
		virtual bool Main();
		/** store the application parameters */
		virtual void StoreParameters(int argc, char ** argv, char ** env);
		/** Initialize the application with the main data */
		virtual bool Initialize();
		/** Finalization method */
		virtual bool Finalize();
		/** standard library initialization */
		virtual bool InitializeStandardLibraries();
		/** standard library finalization */
		virtual bool FinalizeStandardLibraries();

		/** initialize the managers */
		virtual bool InitializeManagers();
		/** finalize the managers */
		virtual bool FinalizeManagers();

		/** loading the configuration file */
		bool LoadConfigurationFile();

	protected:

		/** the single application instance */
		static Application * singleton_instance;

		/** the name of the application */
		std::string application_name;
		/** the application parameters */
		std::vector<std::string> arguments;
		/** the application environments */
		std::vector<std::pair<std::string, std::string>> environments;
		/** the filename of the application */
		boost::filesystem::path application_filename;
		/** the path of the application */
		boost::filesystem::path application_path;
		/** the path of the resources */
		boost::filesystem::path resources_path;
		/** path of the application to store user data */
		boost::filesystem::path userlocal_path;
		/** path of the application to store user temp data */
		boost::filesystem::path userlocal_temp_path;

		/** the JSON configuration file if existing */
		nlohmann::json configuration;
		/** the path where the configuration file is */
		boost::filesystem::path configuration_path;

		/** redirection source directories */
#if _DEBUG
		boost::filesystem::path redirection_source_path;
		boost::filesystem::path redirection_build_path;
#endif

		/** the file manager */
		shared_ptr<FileManager> file_manager;

		/** whether to show the console by default (can be overriden in constructor) */
#if _DEBUG
		bool show_console = true;
#else
		bool show_console = false;
#endif
	};

	template<typename APPLICATION_TYPE>
	bool RunApplication(int argc, char ** argv, char ** env)
	{
		chaos::shared_ptr<APPLICATION_TYPE> application = new APPLICATION_TYPE();
		if (application != nullptr)
			return application->Run(argc, argv, env);
		return false;
	}

}; // namespace chaos
