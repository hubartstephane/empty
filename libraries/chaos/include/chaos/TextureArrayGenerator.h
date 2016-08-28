#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/Texture.h>
#include <chaos/GLTools.h>
#include <chaos/ImageTools.h>

namespace chaos
{

  class ImageDescriptionGeneratorProxy
  {
  public:

    /** constructor */
    ImageDescriptionGeneratorProxy() {}
    /** destructor */
    virtual ~ImageDescriptionGeneratorProxy() {}

    /** get the image description */
    virtual ImageDescription GetImageDescription() = 0;
  };

  /*
  class ImageDescriptionGeneratorProxy : public ImageDescriptionGeneratorProxy
  {


  };
  */

  /**
  * ImageDescriptionGenerator : used to generate proxy for TextureArrayGeneration
  */

  class ImageDescriptionGenerator
  {
  public:

    /** constructor */
    ImageDescriptionGenerator() {}
    /** destructor */
    virtual ~ImageDescriptionGenerator() {}
    /** proxy generation method */
    virtual ImageDescriptionGeneratorProxy * CreateProxy() const = 0;
  };

  /**
   * TextureArrayGenerator : an helper class that is used to generate texture array    GL_TEXTURE_1D_ARRAY,    GL_TEXTURE_2D_ARRAY or    GL_TEXTURE_CUBE_ARRAY
   */

  class TextureArrayGenerator
  {
  public:

    /** constructor */
    TextureArrayGenerator();
    /** destructor */
    virtual ~TextureArrayGenerator();

    /** the insertion method (returns the slice considered) */
    int AddGenerator(ImageDescriptionGenerator const & generator);
    /** clean all generators */
    void Clean();
    /** generate all meshes */
    boost::intrusive_ptr<Texture> GenerateTexture(GenTextureParameters const & parameters) const;

  protected:

    /** ensure some image format is compatible with Texture Array requirements */
    bool IsValid(ImageDescription const & desc) const;
    /** ensure the image format is compatible with previous format */
    bool IsCompatible(ImageDescription const & desc1, ImageDescription const & desc2) const;

  protected:


    /** the registered element to generate */
    std::vector<ImageDescriptionGeneratorProxy *> generators;
  };

};
