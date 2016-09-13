#include <chaos/SimpleWin32Class.h>
#include <chaos/SimpleWin32Window.h>
#include <chaos/WinTools.h>
#include <chaos/StringTools.h>
#include <chaos/TextureAtlas.h>
#include <chaos/MathTools.h>
#include <chaos/Application.h>
#include <chaos/FileTools.h>
#include <chaos/FontTools.h>

void TestAtlasReload(boost::filesystem::path const & src_dir, char const * pattern)
{
  chaos::TextureAtlasData data;
  data.LoadAtlas(src_dir, pattern);
}

void TestAtlasDebugMode(boost::filesystem::path const & dest_p, boost::filesystem::path const & resources_path)
{
  chaos::TextureAtlasData data;

  data.AddFakeImageSource("A");
  data.AddFakeImageSource("B");
  data.AddFakeImageSource("C");
  data.AddFakeImageSource("D");
  data.AddFakeImageSource("E");
  data.AddFakeImageSource("F");
  data.AddFakeImageSource("G");
  data.AddFakeImageSource("H");
  data.AddFakeImageSource("I");
  data.AddFakeImageSource("J");

  int atlas_width   = 256;
  int atlas_height  = 256;
  int atlas_padding = 3;

  chaos::TextureAtlasHTMLOutputParams params;
  params.show_header       = true;
  params.show_atlas_header = true;  
  params.texture_scale     = 3.0f;
  params.auto_refresh      = false;  

  chaos::TextureAtlasCreator atlas_creator;
  
  if (atlas_creator.ComputeResult(data, atlas_width, atlas_height, atlas_padding))
  {  
    boost::filesystem::path html_path = dest_p / "Atlas_Final.html";
    atlas_creator.OutputToHTMLFile(html_path.string().c_str(), params);

    boost::filesystem::path dst_dir = dest_p / "AtlasResultFake";
    data.SaveAtlas(dst_dir.string().c_str(), "MyAtlas");
  }
}

void TestAtlasNormalMode(boost::filesystem::path const & dest_p, boost::filesystem::path const & resources_path)
{
  // XXX : the resources directory contains
  //        - a bmp image whose extension has been swapped to ".txt" => detected has an image
  //        - a text file                                            => not detected has an image
  // correct behavior 

  int atlas_width   = 512;
  int atlas_height  = 512;
  int atlas_padding = 10;

  boost::filesystem::path result_path = dest_p / "AtlasResult";

  chaos::TextureAtlasCreator::CreateAtlasFromDirectory(
    resources_path.string().c_str(), 
    result_path, 
    "MyAtlas",
    atlas_width, 
    atlas_height, 
    atlas_padding);
}

void TestAtlasFont(boost::filesystem::path const & dest_p, boost::filesystem::path const & resources_path)
{
  chaos::TextureAtlasData data;

  boost::filesystem::path font_path = resources_path / "unispace bold italic.ttf";
  if (chaos::FontTools::GenerateTextureAtlas(nullptr, font_path.string().c_str(), data, nullptr))
  {
    boost::filesystem::path dst_dir = dest_p / "AtlasResultFont";    
    data.SaveAtlas(dst_dir.string().c_str(), "MyAtlas");
  }
}

int _tmain(int argc, char ** argv, char ** env)
{
  chaos::Application::Initialize<chaos::Application>(argc, argv, env);

  chaos::WinTools::AllocConsoleAndRedirectStdOutput();

  chaos::MathTools::ResetRandSeed();

  boost::filesystem::path dst_p;
  if (chaos::FileTools::CreateTemporaryDirectory("TestAtlas", dst_p))
  {
    boost::filesystem::path resources_path = chaos::Application::GetInstance()->GetResourcesPath();

    TestAtlasDebugMode(dst_p, resources_path);
    TestAtlasNormalMode(dst_p, resources_path);
    TestAtlasFont(dst_p, resources_path);

    TestAtlasReload(dst_p / "AtlasResult", "MyAtlas");

    chaos::WinTools::ShowFile(dst_p.string().c_str());
  }

  chaos::WinTools::PressToContinue();

  chaos::Application::Finalize();

  return 0;
}


