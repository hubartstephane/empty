#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/NamedObject.h>
#include <chaos/FontTools.h>
#include <chaos/ImageDescription.h>
#include <chaos/PixelFormat.h>
#include <chaos/FilePath.h>

namespace chaos
{
	namespace BitmapAtlas
	{

		/**
		* FontInfoInputParams : when inserting FontInfoInput into AtlasInput, some glyphs are rendered into bitmaps. This controls the process
		*/

		class FontInfoInputParams
		{
		public:

			/** width of the glyph */
			int max_character_width = 32;
			/** height of the glyph */
			int max_character_height = 32;
			/** the characters to generate */
			std::string characters;
		};

		/**
		* ObjectBaseInput : base object for inputs
		*/

		class ObjectBaseInput : public NamedObject
		{

		};

		/**
		* BitmapInfoInput : Will produced a BitmapInfo in the final Atlas
		*/

		class BitmapInfoInput : public ObjectBaseInput
		{
		public:

			/** constructor */
			BitmapInfoInput() = default;
			/** constructor (copy) */
			BitmapInfoInput(BitmapInfoInput const & src) = default;
			/** constructor (move) */
			BitmapInfoInput(BitmapInfoInput && src);	// XXX : important because the move constructor does nothing by default on pointers
																								//       we do not want double deletion on bitmaps
			/** destructor */
			virtual ~BitmapInfoInput();

			/** copy and move operator */
			BitmapInfoInput & operator = (BitmapInfoInput && src);

			/** the description of the bitmap */
			ImageDescription description;
			/** the bitmap */
			FIBITMAP * bitmap = nullptr;
			/** the animated bitmap */
			FIMULTIBITMAP * animated_bitmap = nullptr;
			/** whether the bitmap is to be destroyed at the end */
			bool release_bitmap = true;
			/** a pointer on the destination info associated */
			BitmapInfo * output_info = nullptr;
		};

		/**
		* CharacterInfoInput : an info in FontInfoInput. Will produced a CharacterInfo in the final Atlas
		*/

		class CharacterInfoInput : public BitmapInfoInput
		{
		public:
			FT_Vector advance{ 0, 0 };
			int       bitmap_left = 0; // from 'CharacterMetrics' class
			int       bitmap_top = 0;
		};

		/**
		* FontInfoInput : this info will produced in the final Atlas a FontInfo (a set of glyphs generated from FreeType)
		*/

		class FontInfoInput : public FontInfoTemplate<CharacterInfoInput, ObjectBaseInput>
		{
			friend class AtlasInput;
			friend class AtlasGenerator;
			friend class FolderInfoInput;

		public:

			/** constructor */
			FontInfoInput() = default;
			/** constructor (copy) */
			FontInfoInput(FontInfoInput const & src) = default;
			/** constructor (move) */
			FontInfoInput(FontInfoInput && src); // XXX : important because the move constructor does nothing by default on pointers
			                                     //       we do not want double deletion on libreary and face
			/** destructor */
			virtual ~FontInfoInput();

			/** copy and move operator */
			FontInfoInput & operator = (FontInfoInput && src);

			/** the Freetype library if appropriate */
			FT_Library library = nullptr;
			/** the Freetype face if appropriate */
			FT_Face    face = nullptr;
			/** should the library be released at destruction */
			bool release_library = true;
			/** should the face be released at destruction */
			bool release_face = true;
		};

		/**
		* FolderInfoInput :  this info will produced in the final Atlas a FolderInfo
		*/

		class FolderInfoInput : public FolderInfoTemplate<BitmapInfoInput, FontInfoInput, FolderInfoInput, ObjectBaseInput>
		{
			friend class AtlasInput;
			friend class AtlasGenerator;

		public:

			/** constructor */
			FolderInfoInput() = default;

			/** insert a Folder set inside the input */
			FolderInfoInput * AddFolder(char const * name, TagType tag);

			/** insert multiple bitmap before computation */
			bool AddBitmapFilesFromDirectory(FilePathParam const & path, bool recursive);

			/** insert a bitmap before computation */
			bool AddBitmap(FilePathParam const & path, char const * name, TagType tag);
			/** insert an image inside the atlas */
			bool AddBitmap(FIBITMAP * bitmap, bool release_bitmap, char const * name, TagType tag);
			/** insert an image inside the atlas */
			bool AddBitmap(FIMULTIBITMAP * animated_bitmap, bool release_bitmap, char const * name, TagType tag);

			/** Add a character set */
			FontInfoInput * AddFont(				
				char const * font_name,
				FT_Library library,
				bool release_library,
				char const * name,
				TagType tag,
				FontInfoInputParams const & params = FontInfoInputParams());
			/** Add a character set */
			FontInfoInput * AddFont(
				FT_Face face,
				bool release_face,
				char const * name,
				TagType tag,
				FontInfoInputParams const & params = FontInfoInputParams());

		protected:

			/** internal method to add a bitmap or a multi bitmap */
			bool AddBitmapImpl(FIBITMAP * bitmap, FIMULTIBITMAP * animated_bitmap, bool release_bitmap, char const * name, TagType tag);

			/** internal method to add a character set */
			FontInfoInput * AddFontImpl(
				FT_Library library,
				FT_Face face,
				bool release_library,
				bool release_face,
				char const * name,
				TagType tag,
				FontInfoInputParams const & params);
		};

		/**
		* AtlasInput : this hold the bitmaps / glyphs used for Atlas generation
		*/

		class AtlasInput : public AtlasBaseTemplate<BitmapInfoInput, FontInfoInput, FolderInfoInput, ReferencedObject>
		{
			friend class AtlasGenerator;

		public:

			/** destructor */
			virtual ~AtlasInput() { Clear(); }

			/** insert a Folder set inside the input */
			FolderInfoInput * AddFolder(char const * name, TagType tag);

			/** insert multiple bitmap before computation */
			bool AddBitmapFilesFromDirectory(FilePathParam const & path, bool recursive);

			/** insert a bitmap before computation */
			bool AddBitmap(FilePathParam const & path, char const * name, TagType tag);
			/** insert an image inside the atlas */
			bool AddBitmap(FIBITMAP * bitmap, bool release_bitmap, char const * name, TagType tag);
			/** insert an image inside the atlas */
			bool AddBitmap(FIMULTIBITMAP * animated_bitmap, bool release_bitmap, char const * name, TagType tag);

			/** Add a character set */
			FontInfoInput * AddFont(
				char const * font_name,
				FT_Library library,
				bool release_library,
				char const * name,
				TagType tag,
				FontInfoInputParams const & params = FontInfoInputParams());
			/** Add a character set */
			FontInfoInput * AddFont(
				FT_Face face,
				bool release_face,
				char const * name,
				TagType tag,
				FontInfoInputParams const & params = FontInfoInputParams());
		};

		/**
		* AtlasGeneratorParams : parameters used when generating an atlas
		*/

		class AtlasGeneratorParams
		{
		public:

			/** contructor */
			AtlasGeneratorParams(int in_width = 0, int in_height = 0, int in_padding = 0, PixelFormatMergeParams const & in_merge_params = PixelFormatMergeParams()) :
				atlas_width(in_width),
				atlas_height(in_height),
				atlas_padding(in_padding),
				merge_params(in_merge_params) {}

			/** whether we have to use power of 2 values */
			bool force_power_of_2 = true;
			/** whether we have to use square bitmap */
			bool force_square = true;
			/** the width of an atlas bitmap */
			int atlas_width = 0;
			/** the height of an atlas bitmap */
			int atlas_height = 0;
			/** the max width of an atlas bitmap (if resized). 0 = no limit */
			int atlas_max_width = 0;
			/** the max height of an atlas bitmap (if resized). 0 = no limit */
			int atlas_max_height = 0;
			/** some padding for the bitmap : should be even */
			int atlas_padding = 0;
			/** the background color */
			glm::vec4 background_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			/** parameters for merging different pixel format */
			PixelFormatMergeParams merge_params;

#if _DEBUG
			/** when generating an atlas, this may dump into a file */
			std::string debug_dump_atlas_dirname;
#endif
		};

		/**
		* Rectangle : a class to represents rectangles
		*/

		class Rectangle
		{
		public:
			/** the top-left corner of the rectangle */
			int x;
			/** the top-left corner of the rectangle */
			int y;
			/** the size of the rectangle */
			int width;
			/** the size of the rectangle */
			int height;
			/** equality test */
			bool operator == (Rectangle const & src) const
			{
				return (x == src.x) && (y == src.y) && (width == src.width) && (height == src.height);
			}
			/** returns true whenever big does fully contains this */
			bool IsFullyInside(Rectangle const & big) const;
			/** returns true whenever there is an intersection between this and big */
			bool IsIntersecting(Rectangle const & big) const;
		};

		/**
		* AtlasGenerator :
		*   each time a BitmapInfo is inserted, the space is split along 4 axis
		*   this creates a grid of points that serve to new positions for inserting next entries ...
		*   it select the best position as the one that minimize space at left, right, top and bottom
		*/

		class AtlasGenerator
		{
			/** an definition is a set of vertical and horizontal lines that split the space */
			class AtlasDefinition
			{
			public:
				unsigned int surface_sum = 0;
				std::vector<int> split_x;
				std::vector<int> split_y;
			};

			/** an utility class used to reference all entries in input */
			using BitmapInfoInputVector = std::vector<BitmapInfoInput *>;

		public:

			/** make destructor virtual */
			virtual ~AtlasGenerator() = default;
			/** compute all BitmapInfo positions */
			bool ComputeResult(AtlasInput const & in_input, Atlas & in_ouput, AtlasGeneratorParams const & in_params = AtlasGeneratorParams());
			/** returns a vector with all generated bitmaps (to be deallocated after usage) */
			std::vector<bitmap_ptr> GenerateBitmaps(BitmapInfoInputVector const & entries, PixelFormat const & final_pixel_format) const;
			/** create an atlas from a directory into another directory */
			static bool CreateAtlasFromDirectory(FilePathParam const & bitmaps_dir, FilePathParam const & path, bool recursive, AtlasGeneratorParams const & in_params = AtlasGeneratorParams());

		protected:

			/** clear the results */
			void Clear();
			/** returns the box for the atlas */
			Rectangle GetAtlasRectangle() const;
			/** add padding to a rectangle */
			Rectangle AddPadding(Rectangle const & r) const;
			/** returns the rectangle corresponding to the BitmapInfo */
			Rectangle GetRectangle(BitmapInfo const & info) const;

			/** fill the entries of the atlas from input (collect all input entries) */
			void FillAtlasEntriesFromInput(BitmapInfoInputVector & result, FolderInfoInput * folder_info_input, FolderInfo * folder_info_output);
			/** test whether there is an intersection between each pair of Entries in an atlas */
			bool EnsureValidResults(BitmapInfoInputVector const & result, std::ostream & stream = std::cout) const;
			/** test whether rectangle intersects with any of the entries */
			bool HasIntersectingInfo(BitmapInfoInputVector const & entries, int bitmap_index, Rectangle const & r) const;

			/** the effective function to do the computation */
			bool DoComputeResult(BitmapInfoInputVector const & entries);
			/** an utility function that gets a score for a rectangle */
			float GetAdjacentSurface(BitmapInfoInput const & info, AtlasDefinition const & atlas_def, std::vector<int> const & collision, size_t x_count, size_t y_count, size_t u, size_t v, size_t dx, size_t dy) const;
			/** returns the position (if any) in an atlas withe the best score */
			float FindBestPositionInAtlas(BitmapInfoInputVector const & entries, BitmapInfoInput const & info, AtlasDefinition const & atlas_def, int & x, int & y) const;
			/** insert an integer in a vector. keep it ordered */
			void InsertOrdered(std::vector<int> & v, int value);
			/** insert a bitmap in an atlas definition */
			void InsertBitmapInAtlas(BitmapInfo & info, AtlasDefinition & atlas_def, int x, int y);

			/** an utility function that returns an array with 0.. count - 1*/
			static std::vector<size_t> CreateIndexTable(size_t count)
			{
				std::vector<size_t> result;
				result.reserve(count);
				for (size_t i = 0; i < count; ++i)
					result.push_back(i);
				return result;
			}
			/** an utility function to generate sub series from a function */
			template<typename FUNC>
			std::vector<size_t> CreateIndirectionTable(size_t count, FUNC func)
			{
				std::vector<size_t> result = CreateIndexTable(count);
				std::sort(result.begin(), result.end(), func);
				return result;
			}

		protected:

			/** the params for generation */
			AtlasGeneratorParams params;
			/** the input files */
			AtlasInput const * input = nullptr;
			/** the result */
			Atlas * output = nullptr;
			/** all definitions */
			std::vector<AtlasDefinition> atlas_definitions;
		};

		/**
		* TextureArrayAtlasGenerator 
		*/

		class TextureArrayAtlasGenerator
		{

		public:

			/** make destructor virtual */
			virtual ~TextureArrayAtlasGenerator() = default;
			/** compute all BitmapInfo positions */
			TextureArrayAtlas * ComputeResult(AtlasInput const & in_input, AtlasGeneratorParams const & in_params = AtlasGeneratorParams());
		};

	}; // namespace BitmapAtlas

}; // namespace chaos
