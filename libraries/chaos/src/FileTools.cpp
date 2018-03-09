#include <chaos/FileTools.h>
#include <chaos/LogTools.h>
#include <chaos/StringTools.h>
#include <chaos/AllocatorTools.h>

namespace chaos
{

  char const * FileTools::GetFilenameExtension(char const * filename)
  {
    assert(filename != nullptr);
    char const * result = strchr(filename, '.');
    while (result != nullptr)
    {
      char const * next_result = strchr(result + 1, '.');
      if (next_result == nullptr)
        break;
      result = next_result;
    }
    if (result != nullptr)
    {
      if (result[0] == '.') // do not include separator
        ++result;
      if (result[0] == 0) // empty extension is considered has no extension
        result = nullptr;
    }
    return result;
  }

  bool FileTools::IsTypedFile(char const * filename, char const * expected_ext)
  {
    assert(filename != nullptr);
    assert(expected_ext != nullptr);
    char const * ext = GetFilenameExtension(filename);
    if (ext != nullptr)
      return (_stricmp(ext, expected_ext) == 0);
    return false;
  }

  Buffer<char> FileTools::LoadFile(boost::filesystem::path const & filename, bool ascii)
  {
    return LoadFile(filename.string().c_str(), ascii);
  }

  Buffer<char> FileTools::LoadFile(char const * filename, bool ascii)
  {
    assert(filename != nullptr);

    Buffer<char> result;

    std::ifstream file(filename, std::ifstream::binary); // never want to format data
    if (file)
    {
      std::streampos start = file.tellg();
      file.seekg(0, std::ios::end);
      std::streampos end = file.tellg();
      file.seekg(0, std::ios::beg);

      size_t file_size = (size_t)(end - start);

      result = SharedBufferPolicy<char>::NewBuffer(file_size + (ascii ? 1 : 0));

      if (result != nullptr)
      {
        file.read((char *)result.data, file_size);
        if (file.gcount() != file_size) // read all or failure
          result = Buffer<char>();
        else if (ascii)
          result.data[file_size] = 0;
      }
    }
    return result;
  }
  bool FileTools::CreateTemporaryDirectory(char const * pattern, boost::filesystem::path & result)
  {
    boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();

    boost::filesystem::path uniq_path = (pattern == nullptr) ?
      boost::filesystem::unique_path(temp_path / "%%%%-%%%%-%%%%-%%%%") :
      boost::filesystem::unique_path(temp_path / StringTools::Printf("%s_%%%%-%%%%-%%%%-%%%%", pattern));

    if (boost::filesystem::create_directories(uniq_path))
    {
      result = std::move(uniq_path);
      return true;
    }

    return false;
  }

  std::vector<std::string> FileTools::ReadFileLines(char const * filename)
  {
    std::vector<std::string> result;

    std::ifstream file(filename);
    if (file)
    {
      std::copy(std::istream_iterator<std::string>(file),
        std::istream_iterator<std::string>(),
        std::back_inserter(result));
    }
    return result;
  }

  bool FileTools::WriteFileLines(char const * filename, std::vector<std::string> const & lines)
  {
    std::ofstream file(filename);
    if (file)
    {
      for (std::string const & str : lines)
        file << str << std::endl;
      return true;
    }
    return false;
  }

}; // namespace chaos