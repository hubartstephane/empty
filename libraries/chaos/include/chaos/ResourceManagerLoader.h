#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/JSONTools.h>
#include <chaos/BoostTools.h>
#include <chaos/FilePath.h>
#include <chaos/NamedObject.h>
#include <chaos/FileResource.h>
#include <chaos/ObjectRequest.h>

namespace chaos
{
	// XXX : the path of a resource (texture/program ...) is given by the first time GenTextureObject( PATH ) is called
	//       for example:
	//
	//          GenTextureObject ( PATH = "file1.json" )
	//            -> load JSON file file1.json
	//            -> decrypt PATH = "file2.xxx"
	//               -> GenTextureObject ( PATH = "file2.xxx" )
	//
	//       so, the PATH that is kept is "file1.json" (and not "file2.xxx" even it is the final call)

	class ResourceManagerLoaderBase
	{
	protected:

		/** search whether the path is already in used in the manager */
		virtual bool IsPathAlreadyUsedInManager(FilePathParam const& path) const { return false; }
		/** search whether the name is already in used in the manager */
		virtual bool IsNameAlreadyUsedInManager(ObjectRequest request) const { return false; }

		/** set the path of currently loaded resource if not already set, and if no collision detected */
		bool CheckResourcePath(FilePathParam const& path) const;
		/** set the name of currently loaded resource if not already set, and if no collision detected */
		bool CheckResourceName(boost::filesystem::path const* in_path, char const* in_name, nlohmann::json const* json) const;
		/** apply the name to resource */
		void ApplyNameToLoadedResource(NamedObject* resource) const;
		/** apply the path to resource */
		void ApplyPathToLoadedResource(FileResource* resource) const;

	protected:

		/** the name of currently loaded resource (the very first name encoutered in loading call chain is the good) */
		mutable std::string resource_name;
		/** the path of currently loaded resource (the very first path encoutered in loading call chain is the good) */
		mutable boost::filesystem::path resolved_path;
	};

	/**
	* ResourceManagerLoader
	**/

	template<typename RESOURCE_TYPE, typename MANAGER_TYPE>
	class ResourceManagerLoader : public ResourceManagerLoaderBase
	{
	public:

		using resource_type = RESOURCE_TYPE;
		using manager_type = MANAGER_TYPE;
		
		/** constructor */
		ResourceManagerLoader(MANAGER_TYPE * in_manager = nullptr) :
			manager(in_manager)
		{			
		}

		/** returns the manager */
		manager_type * GetManager() const { return manager; }

	protected:

		/** the resource manager of interest */
		MANAGER_TYPE * manager = nullptr;
	};

}; // namespace chaos
