#ifdef CHAOS_FORWARD_DECLARATION

namespace chaos
{
	namespace BitmapAtlas
	{
		class AtlasHTMLOutputParams;
		class AtlasHTMLGenerator;

	}; // namespace BitmapAtlas

}; // namespace chaos

#else 

namespace chaos
{
	namespace BitmapAtlas
	{
		/**
		* AtlasHTMLOutputParams : settings to output an atlas into an HTML file
		*/

		class AtlasHTMLOutputParams
		{
		public:

			/** show the HTML header */
			bool  show_header = true;
			/** show the atlas header */
			bool  show_atlas_header = true;
			/** show the textures */
			bool  show_textures = true;
			/** show the texture names */
			bool  show_textures_names = true;
			/** should the document refresh itself ("http-equiv", "refresh") every 2 seconds */
			bool  auto_refresh = true;
			/** a scale factor to be applied for the rendering of the textures (-1 for auto factor dependent on the atlas size) */
			float texture_scale = -1.0f;
		};

		/**
		* AtlasHTMLGenerator : use to generate HTML from an atlas
		*/

		class AtlasHTMLGenerator
		{
		public:

			/** create an XML document and output debug information */
			static tinyxml2::XMLDocument * OutputToHTMLDocument(Atlas const & atlas, AtlasHTMLOutputParams params = AtlasHTMLOutputParams());
			/** create an XML document and output debug information */
			static void OutputToHTMLDocument(Atlas const & atlas, tinyxml2::XMLDocument * doc, AtlasHTMLOutputParams params = AtlasHTMLOutputParams());
			/** output the atlas trees in HTML format */
			static bool OutputToHTMLFile(Atlas const & atlas, FilePathParam const & path, AtlasHTMLOutputParams params = AtlasHTMLOutputParams());

		protected:

			/** utility methods to iterate over BitmapSets or FontInfos and display their entries informations into HTML */
			template<typename T>
			static void OutputElementsToHTMLDocument(char const * folder_path, std::vector<T> const & elements, XMLTools & html, tinyxml2::XMLElement * TABLE, tinyxml2::XMLElement * &TR, int bitmap_index, int & count);
			/** utility methods to iterate over elements of a folders and display their entries informations into HTML */
			static void OutputElementsToHTMLDocument(char const * folder_path, FolderInfo const * folder_info, XMLTools & html, tinyxml2::XMLElement * TABLE, tinyxml2::XMLElement * &TR, int bitmap_index, int & count);

			/** utility methods to iterate over BitmapSets or FontInfos and display the texture rectangle into HTML */
			template<typename T>
			static void OutputBitmapsToHTMLDocument(std::vector<T> const & elements, XMLTools & html, tinyxml2::XMLElement * SVG, int bitmap_index, float scale);
			/** utility method to recursively draw the bitmaps and characters from folders */
			static void OutputBitmapsToHTMLDocument(FolderInfo const * folder_info, XMLTools & html, tinyxml2::XMLElement * SVG, int bitmap_index, float scale);

			/** utility methods to iterate over BitmapSets or FontInfos and display the texture filename into HTML */
			template<typename T>
			static void OutputBitmapFilenamesToHTMLDocument(std::vector<T> const & elements, XMLTools & html, tinyxml2::XMLElement * SVG, int bitmap_index, float scale);
			/** utility method to recursively display bitmaps and characters filenames */
			static void OutputBitmapFilenamesToHTMLDocument(FolderInfo const * folder_info, XMLTools & html, tinyxml2::XMLElement * SVG, int bitmap_index, float scale);
		};
	}; // namespace BitmapAtlas

}; // namespace chaos

#endif // CHAOS_FORWARD_DECLARATION
