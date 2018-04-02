#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ImageTools.h>
#include <chaos/SkyBoxTools.h>
#include <chaos/Texture.h>
#include <chaos/GPUFileResource.h>
#include <chaos/FilePath.h>

namespace chaos
{

	class GLPixelFormat
	{
	public:

		/** constructor */
		GLPixelFormat() = default;

		GLPixelFormat(GLenum in_format, GLenum in_internal_format);

		/** returns true whether the pixel format is supported */
		bool IsValid() const;

	public:

		/** how the data should be stored in CPU side */
		GLenum format = GL_NONE;
		/** how the data should be stored in GPU side */
		GLenum internal_format = GL_NONE;
	};

	/**
	* GenTextureParameters : some parameters to load textures
	**/

	class GenTextureParameters
	{
	public:

		/** parameters for glTexParameteri(...) */
		GLenum wrap_s =  GL_REPEAT;
		/** parameters for glTexParameteri(...) */
		GLenum wrap_t = GL_REPEAT;
		/** parameters for glTexParameteri(...) */
		GLenum wrap_r = GL_REPEAT;
		/** parameters for glTexParameteri(...) */
		GLenum mag_filter = GL_LINEAR;
		/** parameters for glTexParameteri(...) */
		GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR;
		/** parameters for glTexImageXX(...) */
		GLint  level = 0;
		/** parameters for glTexImageXX(...) */
		GLint  border = 0;
		/** enable build mipmaps */
		bool build_mipmaps = true;
		/** enable the texture to be used has rectangular instead of GL_TEXTURE_1D or GL_TEXTURE_2D */
		bool rectangle_texture = false;
	};


	/**
	* GLTextureTools : used to have some generic functions for OpenGL
	**/

	class GLTextureTools : protected GPUFileResourceFriend  // give the hability to change path and names to the resource
	{
	public:

		/** get buffer containing the texture data */
		static char * GetTextureImage(GLuint texture_id, GLint level, ImageDescription & desc);
		/** get the pixel format of the texture */
		static PixelFormat GetTexturePixelFormat(GLuint texture_id, GLint level);

		/** returns true whether type is an array texture type */
		static bool IsArrayTextureType(GLenum type);
		/** returns true whether type is not an array texture type */
		static bool IsFlatTextureType(GLenum type);

		/** transform a texture type into an array type */
		static GLenum ToArrayTextureType(GLenum type);
		/** transform a texture type into a flat type */
		static GLenum ToFlatTextureType(GLenum type);

		/** Generate a texture from a json content */
		static Texture * GenTextureObject(nlohmann::json const & json, boost::filesystem::path const & config_path, GenTextureParameters const & parameters = GenTextureParameters());
		/** Generate a 1D/2D/rectangle texture from an file */
		static Texture * GenTextureObject(FilePathParam const & path, GenTextureParameters const & parameters = GenTextureParameters());
		/** Generate a 1D/2D/rectangle texture from an image */
		static Texture * GenTextureObject(ImageDescription const & image, GenTextureParameters const & parameters = GenTextureParameters());
		/** Generate a 1D/2D/rectangle texture from an image */
		static Texture * GenTextureObject(FIBITMAP * image, GenTextureParameters const & parameters = GenTextureParameters());
		/** Generate a cube texture from a skybox */
		static Texture * GenTextureObject(SkyBoxImages const * skybox, PixelFormatMergeParams const & merge_params = PixelFormatMergeParams(), GenTextureParameters const & parameters = GenTextureParameters());

    /** Generate a texture from lambda */
    template<typename T, typename GENERATOR>
    static Texture * GenTextureObject(int width, int height, GENERATOR const & generator, GenTextureParameters const & parameters = GenTextureParameters())
    {
      Texture * result = nullptr;

      PixelFormat pixel_format = PixelFormat::GetPixelFormat<T>();

      int buffer_size = ImageTools::GetMemoryRequirementForAlignedTexture(pixel_format, width, height);

      char * buffer = new char[buffer_size];
      if (buffer != nullptr)
      {
        ImageDescription desc = ImageTools::GetImageDescriptionForAlignedTexture(pixel_format, width, height, buffer);
        generator(desc);
        result = GenTextureObject(desc, parameters);
        delete[](buffer);
      }
      return result;
    }

		/** returns the maximum number of mipmap */
		static int GetMipmapLevelCount(int width, int height);
		static int GetMipmapLevelCount(int width);
		/** for cubemap texture, returns a layer index depending on the face considered */
		static int GetCubeMapLayerValueFromSkyBoxFace(int face, int level = 0);
		/** Get Format/Internal Format pair from the description */
		static GLPixelFormat GetGLPixelFormat(PixelFormat const & pixel_format);

		/** utility function to compute target (GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE) from dimension */
		static GLenum GetTextureTargetFromSize(int width, int height, bool rectangle_texture);

		/** prepare store parameters */
		static char * PrepareGLTextureTransfert(ImageDescription const & desc);

		/** utility function for texture loading */
		static void GenTextureApplyParameters(GLuint texture_id, TextureDescription const & texture_description, GenTextureParameters const & parameters);
	};

}; // namespace chaos
