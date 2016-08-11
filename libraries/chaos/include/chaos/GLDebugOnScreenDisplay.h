#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/VertexDeclaration.h>
#include <chaos/GLProgramData.h>

namespace chaos
{

  /**
   * GLDebugOnScreenDisplay : used to display some text on screen with timeout from a bitmap font (a texture)
   **/

class GLDebugOnScreenDisplay
{
public:

  /** class that is used to for initialization */
  class Params
  {
  public:

    /** constructor */
    Params():
      font_characters_per_line(0),
      character_size(20),
      horiz_spacing(0),
      vert_spacing(0){}

    /** the path of the bitmap texture */
    boost::filesystem::path texture_path;
    /** the characters contained inside the font bitmap, in order */
    std::string font_characters;
    /** the number of characters on a line of the bitmap */
    int         font_characters_per_line;
    /** character size in pixel */
    int         character_size;
    /** some space to add (or remove between each characters) in pixels */
    int         horiz_spacing;
    /** some space to add (or remove between each characters) in pixels */
    int         vert_spacing;
  };

  /** constructor */
  GLDebugOnScreenDisplay();

  /** display the debug text */
  void Display(int width, int height) const;
  /** tick all lines (returns true whether there is a change) */
  bool Tick(double delta_time);
  /** Add a line in the log (character size and position are in pixels) */
  void AddLine(char const * line, float duration = -1);
  /** clear all the lines */
  void Clear();

  /** Initialize the displayer */
  bool Initialize(GLDebugOnScreenDisplay::Params const & params);
  /** Finalize */
  void Finalize();

protected:

  /** internal initialization method */
  bool DoInitialize(GLDebugOnScreenDisplay::Params const & params);
  /** internal method to build the vertex buffer */
  bool BuildVertexBuffer(int width, int height) const;

protected:

  /** the lines of the log (string + lifetime */
  std::vector<std::pair<std::string, float>> lines;
  /** indicates whether the buffer as to be updated */
  mutable bool rebuild_required;
  /** size of the screen (useful to know when lines have to change) */
  mutable int screen_width;


  /** the program to run */
  GLuint program;
  /** the definition of the program */
  GLProgramData program_data;
  /** the declaration of the vertex buffer */
  VertexDeclaration declaration;



  /** the bitmap texture */
  GLuint texture_id;

  /** the vertex array */
  GLuint vertex_array;
  /** the vertex buffer */
  GLuint vertex_buffer;

  /** number of element to draw */
  mutable size_t draw_count;

  /** a copy of the parameters */
  Params mesh_builder_params;

  /** the source for the vertex shader */
  static char const * vertex_shader_source;
  /** the source for the pixel shader */
  static char const * pixel_shader_source;
};

};