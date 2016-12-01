#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/StringTools.h>
#include <chaos/BitmapAtlas.h>

namespace chaos
{

	/** during parsing, some 'commands' are put on the stack for text formating, TextParseStackElement is such a command */
	class TextParseStackElement
	{
	public:

		/** the color to use */
		glm::vec3 color;
		/** the character set selected */
		BitmapAtlas::CharacterSet const * character_set{nullptr};
	};

	class TextParseToken
	{
	public:

		static int const TOKEN_NONE       = 0;
		static int const TOKEN_BITMAP     = 1;
		static int const TOKEN_CHARACTER  = 2;
		static int const TOKEN_WHITESPACE = 3;

		/** get the width of the token after renormalization */
		float GetWidth(class ParseTextParams const & params) const;

	public:

		/** the type of the token */
		int type{TOKEN_NONE};
		/** the character */
		char character{0};
		/** the position of the generated image */
		glm::vec2 position{0.0f, 0.0f};
		/** the corresponding bitmap (if valid) */
		BitmapAtlas::BitmapEntry const * bitmap_entry{nullptr};
		/** the corresponding character (if valid) */
		BitmapAtlas::CharacterEntry const * character_entry{nullptr};
		/** the corresponding character set (if valid) */
		BitmapAtlas::CharacterSet const * character_set{ nullptr };
	};


	/** a structure used to contains data during parsing */
	class TextParserData
	{
	public:

		/** the constructor */
		TextParserData(BitmapAtlas::AtlasBase const & in_atlas) : atlas(in_atlas) {}

		/** start the markup */
		bool StartMarkup(char const * text, int & i, class TextParser & parser, ParseTextParams const & params);
		/** utility method to emit characters */
		void EmitCharacters(char c, int count, ParseTextParams const & params);
		/** utility method to emit character */
		void EmitCharacter(char c, BitmapAtlas::CharacterEntry const * entry, BitmapAtlas::CharacterSet const * character_set, ParseTextParams const & params);
		/** emit a bitmap */
		void EmitBitmap(BitmapAtlas::BitmapEntry const * entry, ParseTextParams const & params);
		/** end the current line */
		void EndCurrentLine(ParseTextParams const & params);
		/** insert a token */
		void InsertTokenInLine(TextParseToken & token, ParseTextParams const & params);

		/** duplicate the last stack element */
		void PushDuplicate();
		/** add an element on parse stack : keep color, but change current character_set */
		void PushCharacterSet(BitmapAtlas::CharacterSet const * character_set);
		/** add an element on parse stack : keep character_set, but change current color */
		void PushColor(glm::vec3 const & color);

		/** get a character set from its name */
		BitmapAtlas::CharacterSet const * GetCharacterSetFromName(char const * character_set_name) const;

	public:

		/** the atlas in used */
		BitmapAtlas::AtlasBase const & atlas;
		/** current line position */
		glm::vec2 position{0.0f, 0.0f};
		/** the lines */
		std::vector<std::vector<TextParseToken>> lines;
		/** the stack used for parsing */
		std::vector<TextParseStackElement> parse_stack;
	};


	/** some parameters used during text parsing */
	class ParseTextParams
	{
	public:

		static const int ALIGN_LEFT = 0;
		static const int ALIGN_RIGHT = 1;
		static const int ALIGN_CENTER = 2;
		static const int ALIGN_JUSTIFY = 3;

		/** the size to use for the characters */
		float character_height{ 32.0f };
		/** the text limits */
		float max_text_width{ 0.0f };
		/** word wrap enabled */
		bool word_wrap{ true };
		/** the line alignment */
		int alignment{ ALIGN_LEFT };
		/** the color to use by default */
		glm::vec3 default_color{ 1.0f, 1.0f, 1.0f };
		/** the font to use by default */
		std::string character_set_name;
		/** tab size */
		int tab_size{ 2 };
	};


	class TextParser
	{
	public:

		/** constructor with atlas initialization */
		TextParser(BitmapAtlas::AtlasBase const & in_atlas) : atlas(in_atlas) {}

		/** add a named color in the parser */
		bool AddColor(char const * name, glm::vec3 const & color);
		/** add a named bitmap in the parser */
		bool AddBitmap(char const * name, char const * bitmap_set_name, char const * bitmap_name);
		/** add a named bitmap in the parser */
		bool AddBitmap(char const * name, BitmapAtlas::BitmapEntry const * entry);

		/** add a named character set in the parser */
		bool AddCharacterSet(char const * name, char const * font_name);
		/** add a named character set in the parser */
		bool AddCharacterSet(char const * name, BitmapAtlas::CharacterSet const * character_set);

		/** the main method to parse a text */
		bool ParseText(char const * text, ParseTextParams const & params = ParseTextParams());

		/** get a color by its name */
		glm::vec3 const * GetColor(char const * name) const;
		/** get a bitmap by its name */
		BitmapAtlas::BitmapEntry const * GetBitmap(char const * name) const;
		/** get a character set by its name */
		BitmapAtlas::CharacterSet const * GetCharacterSet(char const * name) const;
		/** test whether a name is a key in one of the following maps : colors, bitmaps, character_sets */
		bool IsNameValid(char const * name) const;

	protected:

		/** generate the lines, without cutting them */
		bool GenerateLines(char const * text, ParseTextParams const & params, TextParserData & parse_data);
		/** cut the lines so they are not too big. Cut them only when it is possible */
		bool CutLines(ParseTextParams const & params, TextParserData & parse_data);
		/** utility method to cut one line an insert it into a new result */
		void CutOneLine(float & y, std::vector<TextParseToken> const & line, std::vector<std::vector<TextParseToken>> & result_lines, ParseTextParams const & params, TextParserData & parse_data);
		/** goto next line */
		void FlushLine(float & x, float & y, std::vector<TextParseToken> & current_line, std::vector<std::vector<TextParseToken>> & result_lines, ParseTextParams const & params);
		/** insert all tokens of a group in one line */
		void InsertAllTokensInLine(float & x, float & y, std::pair<size_t, size_t> const & group, std::vector<TextParseToken> const & line, std::vector<TextParseToken> & current_line);
		/** remove whitespaces at end of lines, and empty lines at the end */
		bool RemoveUselessWhitespaces(ParseTextParams const & params, TextParserData & parse_data);
		/** update lines according to justification */
		bool JustifyLines(ParseTextParams const & params, TextParserData & parse_data);
		/** generate the sprites */
		bool GenerateSprites(ParseTextParams const & params, TextParserData & parse_data);
		/** group tokens */
		std::vector<std::pair<size_t, size_t>> GroupTokens(std::vector<TextParseToken> const & line);

	public:

		/** the colors to use, indexed by a joker name */
		std::map<std::string, glm::vec3, StringTools::ci_less> colors;
		/** the bitmaps to use, indexed by a joker name */
		std::map<std::string, BitmapAtlas::BitmapEntry const *, StringTools::ci_less> bitmaps;
		/** the character_set to use, indexed by a joker name */
		std::map<std::string, BitmapAtlas::CharacterSet const *, StringTools::ci_less> character_sets;
		/** the atlas where to find entries */
		BitmapAtlas::AtlasBase const & atlas;
	};




};