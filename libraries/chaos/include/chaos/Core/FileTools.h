namespace chaos
{
	/**
	* FileTools is namespace-class for methods to handle files
	*/

	namespace FileTools
	{
#if !defined CHAOS_FORWARD_DECLARATION && !defined CHAOS_TEMPLATE_IMPLEMENTATION

		/** returns true if the extension of a file correspond to a string */
		bool IsTypedFile(FilePathParam const & path, char const * expected_ext);
		/** loading a whole file into memory */
		Buffer<char> LoadFile(FilePathParam const & path, bool ascii, bool * success_open = nullptr);

		/** try path redirection and call func (until it returns true) */
		bool ForEachRedirectedPath(FilePathParam const& path, std::function<bool(boost::filesystem::path const& p)> func);
		/** iterate over all entries in all possible directories (until func returns true) */
		bool ForEachRedirectedDirectoryContent(FilePathParam const& path, std::function<bool(boost::filesystem::path const& p)> func);

		/** returns a filepath that is unused */
		boost::filesystem::path GetUniquePath(FilePathParam const & path, char const * format, bool create_empty_file, int max_iterations = -1);

		/** create a temporary directory */
		bool CreateTemporaryDirectory(char const * pattern, boost::filesystem::path & result);

		/** read file as a vector of strings */
		std::vector<std::string> ReadFileLines(FilePathParam const & path, bool * success_open = nullptr);
		/** write a file with a vector of strings */
		bool WriteFileLines(FilePathParam const & path, std::vector<std::string> const & lines);

		/** redirect any access (under conditions) to the direct resources path of the project (not the build directory) */
#if _DEBUG
		bool GetRedirectedPath(boost::filesystem::path const & p, boost::filesystem::path const & build_path, boost::filesystem::path const& src_path, boost::filesystem::path & redirected_path);
#endif

#endif

	}; // namespace FileTools

}; // namespace chaos



