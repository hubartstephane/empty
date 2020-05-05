#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/TextureArrayAtlas.h>
#include <chaos/NamedObject.h>
#include <chaos/FontTools.h>
#include <chaos/ImageDescription.h>
#include <chaos/PixelFormat.h>
#include <chaos/FilePath.h>
#include <chaos/BitmapAtlasInput.h>

namespace chaos
{
	namespace BitmapAtlas
	{

		/**
		* AtlasGeneratorParams : parameters used when generating an atlas
		*/

		class AtlasGeneratorParams
		{
		public:

			/** contructor */
			AtlasGeneratorParams() = default;
			/** copy contructor */
			AtlasGeneratorParams(AtlasGeneratorParams const& src) = default;
			/** contructor with initialization */
			AtlasGeneratorParams(int in_width, int in_height, int in_padding, PixelFormatMergeParams const & in_merge_params) :
				atlas_width(in_width),
				atlas_height(in_height),
				atlas_padding(in_padding),
				merge_params(in_merge_params) {}

			/** whether we have to use power of 2 values */
			bool force_power_of_2 = true;
			/** whether we have to use square bitmap */
			bool force_square = true;
			/** whether each image in the atlas should have extra border that is the duplication of the origin image (usefull for texel interpolation in shaders) */
			bool duplicate_image_border = true;
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
		};

		/** load from JSON */
		bool LoadFromJSON(nlohmann::json const& json_entry, AtlasGeneratorParams& dst);
		/** save into JSON */
		bool SaveIntoJSON(nlohmann::json& json_entry, AtlasGeneratorParams const& src);

		/**
		* Rectangle : a class to represents rectangles
		*/

		class Rectangle
		{
		public:
			/** the top-left corner of the rectangle */
			int x = 0;
			/** the top-left corner of the rectangle */
			int y = 0;
			/** the size of the rectangle */
			int width = 0;
			/** the size of the rectangle */
			int height = 0;
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

				std::vector<Rectangle>  collision_rectangles;
				std::vector<glm::ivec2> potential_bottomleft_corners;
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
			/** returns the rectangle corresponding to the BitmapLayout */
			Rectangle GetRectangle(BitmapLayout const & layout) const;

			/** fill the entries of the atlas from input (collect all input entries) */
			void FillAtlasEntriesFromInput(BitmapInfoInputVector & result, FolderInfoInput * folder_info_input, FolderInfo * folder_info_output);
			/** test whether there is an intersection between each pair of Entries in an atlas */
			bool EnsureValidResults(BitmapInfoInputVector const & result, std::ostream & stream = std::cout) const;
			/** test whether rectangle intersects with any of the entries */
			bool HasIntersectingInfo(Rectangle const & r, std::vector<Rectangle> const & collision_rectangles) const;

			/** the effective function to do the computation */
			bool DoComputeResult(BitmapInfoInputVector const & entries);
			/** returns the position (if any) in an atlas withe the best score */
			float FindBestPositionInAtlas(BitmapInfoInputVector const & entries, BitmapInfoInput const & info, AtlasDefinition const & atlas_def, glm::ivec2 & position) const;
			/** insert a bitmap in an atlas definition */
			void InsertBitmapLayoutInAtlas(BitmapLayout & layout, AtlasDefinition & atlas_def, glm::ivec2 const & position);

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
			TextureArrayAtlas * ComputeResult(AtlasInput const & in_input, AtlasGeneratorParams const & in_params = AtlasGeneratorParams(), char const * dump_atlas_dirname = nullptr);
		};

	}; // namespace BitmapAtlas

}; // namespace chaos
