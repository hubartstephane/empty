#include <chaos/TiledMap.h>
#include <chaos/FileTools.h>
#include <chaos/XMLTools.h>
#include <chaos/BoostTools.h>
#include <chaos/PixelTypes.h>
#include <chaos/StringTools.h>
#include <chaos/MyBase64.h>
#include <chaos/MyZLib.h>
#include <chaos/TiledMapTools.h>

#define CHAOS_REVERSE_Y_AXIS 1

namespace chaos
{
	namespace TiledMap
	{
		// ==========================================
		// Utility methods
		// ==========================================

		int GetHEXCharacterCount(char const * c)
		{
			int result = 0;
			while (StringTools::GetHexValueForChar(c[result]) < 16)
				++result;
			return result;
		}

		bool ReadAttributeHEX(tinyxml2::XMLElement const * element, char const * attribute_name, unsigned int & result)
		{
			std::string result_string;
			if (!XMLTools::ReadAttribute(element, attribute_name, result_string))
				return false;

			result = StringTools::AtoiH(result_string.c_str());
			return true;
		}

		static bool ReadXMLColor(std::string const& str, glm::vec4& result)
		{
			char const* c = str.c_str(); // #00000000 is a valid attribute too, as well as hexadecimal strings
			if (c[0] == '#')
				c = c + 1;
			else
				c = StringTools::SkipHEXPrefix(c);

			PixelRGBAFloat rgba_float;
			PixelConverter::Convert(rgba_float, PixelBGRA(StringTools::AtoiH(c)));

			int count = GetHEXCharacterCount(c);
			if (count == 6)
				rgba_float.A = 1.0f;

			result = rgba_float;
			return true;
		}

		static bool ReadXMLColor(tinyxml2::XMLElement const* element, char const* attribute_name, glm::vec4& result)
		{
			std::string result_string;
			if (!XMLTools::ReadAttribute(element, attribute_name, result_string))
				return false;

			return ReadXMLColor(result_string, result);
		}

		// ==========================================
		// BaseObject methods
		// ==========================================

		BaseObject::BaseObject(BaseObject * in_owner) :
			owner(in_owner)
		{
		}

		Manager * BaseObject::GetManager()
		{
			return GetOwner<Manager>(true);
		}

		Manager const * BaseObject::GetManager() const
		{
			return GetOwner<Manager>(true);
		}

		Map * BaseObject::GetMap()
		{
			return GetOwner<Map>(true);
		}

		Map const * BaseObject::GetMap() const
		{
			return GetOwner<Map>(true);
		}

		boost::filesystem::path BaseObject::GetOwnerPath() const
		{
			// XXX : order is important, because some tilesets are embedded inside a Map directly
			//       and so their real path is given by MAP->GetPath()

			// try to get the map
			Map const * map = GetOwner<Map>();
			if (map != nullptr)
				return map->GetPath();
			// try to get the tileset
			TileSet const * tileset = GetOwner<TileSet>();
			if (tileset != nullptr)
				return tileset->GetPath();
			// default path
			return boost::filesystem::path();
		}

		// ==========================================
		// Property methods
		// ==========================================
		
		int* Property::GetIntProperty() 
		{
			PropertyTemplate<int, PropertyType::INT>* p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr; 
		}

		int const* Property::GetIntProperty() const 
		{ 
			PropertyTemplate<int, PropertyType::INT> const * p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}
		
		float* Property::GetFloatProperty() 
		{ 
			PropertyTemplate<float, PropertyType::FLOAT> * p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}

		float const* Property::GetFloatProperty() const 
		{ 
			PropertyTemplate<float, PropertyType::FLOAT> const* p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}
		
		bool* Property::GetBoolProperty() 
		{ 
			PropertyTemplate<bool, PropertyType::BOOL> * p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}

		bool const* Property::GetBoolProperty() const 
		{ 
			PropertyTemplate<bool, PropertyType::BOOL> const* p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}
		
		std::string* Property::GetStringProperty() 
		{ 
			PropertyTemplate<std::string, PropertyType::STRING> * p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}

		std::string const* Property::GetStringProperty() const 
		{ 
			PropertyTemplate<std::string, PropertyType::STRING> const* p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}
		
		glm::vec4* Property::GetColorProperty() 
		{ 
			PropertyTemplate<glm::vec4, PropertyType::COLOR> * p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}

		glm::vec4 const* Property::GetColorProperty() const 
		{ 
			PropertyTemplate<glm::vec4, PropertyType::COLOR> const* p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}

		int* Property::GetObjectProperty()
		{
			PropertyTemplate<int, PropertyType::OBJECT>* p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}

		int const* Property::GetObjectProperty() const
		{
			PropertyTemplate<int, PropertyType::OBJECT> const* p = auto_cast(this);
			if (p != nullptr)
				return &p->value;
			return nullptr;
		}

		// ==========================================
		// PropertyOwner methods
		// ==========================================

		Property * PropertyOwner::FindProperty(char const * name, PropertyType type_id)
		{
			return FindInternalProperty(name, type_id);
		}

		Property const * PropertyOwner::FindProperty(char const * name, PropertyType type_id) const
		{
			return FindInternalProperty(name, type_id);
		}


		Property * PropertyOwner::FindInternalProperty(char const * name, PropertyType type_id)
		{
			assert(name != nullptr);
			for (auto & property : properties)
				if (type_id == PropertyType::ANY || type_id == property->GetPropertyType())
					if (StringTools::Stricmp(property->GetName(), name) == 0)
						return property.get();
			return nullptr;
		}

		Property const * PropertyOwner::FindInternalProperty(char const * name, PropertyType type_id) const
		{
			assert(name != nullptr);
			for (auto & property : properties)
				if (type_id == PropertyType::ANY || type_id == property->GetPropertyType())
					if (StringTools::Stricmp(property->GetName(), name) == 0)
						return property.get();
			return nullptr;
		}

		int * PropertyOwner::FindPropertyInt(char const * name)
		{
			Property * property = FindProperty(name, PropertyType::INT);
			if (property == nullptr)
				return nullptr;
			return property->GetIntProperty();
		}

		int const * PropertyOwner::FindPropertyInt(char const * name) const
		{
			Property const * property = FindProperty(name, PropertyType::INT);
			if (property == nullptr)
				return nullptr;
			return property->GetIntProperty();
		}

		float * PropertyOwner::FindPropertyFloat(char const * name)
		{
			Property * property = FindProperty(name, PropertyType::FLOAT);
			if (property == nullptr)
				return nullptr;
			return property->GetFloatProperty();
		}

		float const * PropertyOwner::FindPropertyFloat(char const * name) const
		{
			Property const * property = FindProperty(name, PropertyType::FLOAT);
			if (property == nullptr)
				return nullptr;
			return property->GetFloatProperty();
		}

		bool * PropertyOwner::FindPropertyBool(char const * name)
		{
			Property * property = FindProperty(name, PropertyType::BOOL);
			if (property == nullptr)
				return nullptr;
			return property->GetBoolProperty();
		}

		bool const * PropertyOwner::FindPropertyBool(char const * name) const
		{
			Property const * property = FindProperty(name, PropertyType::BOOL);
			if (property == nullptr)
				return nullptr;
			return property->GetBoolProperty();
		}

		std::string * PropertyOwner::FindPropertyString(char const * name)
		{
			Property * property = FindProperty(name, PropertyType::STRING);
			if (property == nullptr)
				return nullptr;
			return property->GetStringProperty();
		}

		std::string const * PropertyOwner::FindPropertyString(char const * name) const
		{
			Property const * property = FindProperty(name, PropertyType::STRING);
			if (property == nullptr)
				return nullptr;
			return property->GetStringProperty();
		}


		glm::vec4 * PropertyOwner::FindPropertyColor(char const* name)
		{
			Property* property = FindProperty(name, PropertyType::COLOR);
			if (property == nullptr)
				return nullptr;
			return property->GetColorProperty();
		}

		glm::vec4 const* PropertyOwner::FindPropertyColor(char const* name) const
		{
			Property const* property = FindProperty(name, PropertyType::COLOR);
			if (property == nullptr)
				return nullptr;
			return property->GetColorProperty();
		}

		int * PropertyOwner::FindPropertyObject(char const* name)
		{
			Property* property = FindProperty(name, PropertyType::OBJECT);
			if (property == nullptr)
				return nullptr;
			return property->GetObjectProperty();
		}

		int const* PropertyOwner::FindPropertyObject(char const* name) const
		{
			Property const* property = FindProperty(name, PropertyType::OBJECT);
			if (property == nullptr)
				return nullptr;
			return property->GetObjectProperty();
		}

#define CHAOS_FIND_PROPERTY_WITH_DEFAULT(suffix, return_type, arg_type)\
		return_type PropertyOwner::GetPropertyValue##suffix(char const * name, arg_type default_value) const\
		{\
			return_type const * result = FindProperty##suffix(name);\
			if (result == nullptr)\
				return default_value;\
			return *result;\
		}		
		CHAOS_FIND_PROPERTY_WITH_DEFAULT(Int, int, int);			
		CHAOS_FIND_PROPERTY_WITH_DEFAULT(Float, float, float);
		CHAOS_FIND_PROPERTY_WITH_DEFAULT(Bool, bool, bool);
		CHAOS_FIND_PROPERTY_WITH_DEFAULT(String, std::string, char const*);
		CHAOS_FIND_PROPERTY_WITH_DEFAULT(Color, glm::vec4, glm::vec4 const&);
		CHAOS_FIND_PROPERTY_WITH_DEFAULT(Object, int, int);
#undef CHAOS_FIND_PROPERTY_WITH_DEFAULT

			bool PropertyOwner::DoLoad(tinyxml2::XMLElement const * element)
		{
			assert(element != nullptr);

			return DoLoadProperties(GetPropertiesChildNode(element));
		}

		tinyxml2::XMLElement const * PropertyOwner::GetPropertiesChildNode(tinyxml2::XMLElement const * element) const
		{
			return element->FirstChildElement("properties");
		}

		bool PropertyOwner::DoLoadProperties(tinyxml2::XMLElement const * properties_element)
		{
			// no properties is not an error
			if (properties_element == nullptr)
				return true;

			tinyxml2::XMLElement const * node = properties_element->FirstChildElement("property");
			for (; node != nullptr; node = node->NextSiblingElement("property"))
			{
				tinyxml2::XMLAttribute const * name_attribute = node->FindAttribute("name"); // name is mandatory
				if (name_attribute == nullptr)
					continue;
				char const * property_name = name_attribute->Value();
				if (FindInternalProperty(property_name, PropertyType::ANY) != nullptr) // and must be unique
					continue;

				tinyxml2::XMLAttribute const * value_attribute = node->FindAttribute("value"); // value is NOT mandatory (for multiline strings, we use node->GetText())
				if (value_attribute == nullptr)
					value_attribute = node->FindAttribute("default"); // for ObjectTypeSets, that use the keyword 'default' instead of 'value'

				tinyxml2::XMLAttribute const * type_attribute = node->FindAttribute("type"); // type is NOT mandatory (default is string)

				char const * property_type = (type_attribute != nullptr) ?
					type_attribute->Value() : nullptr;

				if (property_type == nullptr || StringTools::Stricmp(property_type, "string") == 0)
				{
					char const * value = (value_attribute != nullptr) ? value_attribute->Value() : node->GetText();
					if (value != nullptr)
						InsertPropertyString(property_name, value);
				}
				else if (value_attribute != nullptr) // now, to this point, value_attribute becomes MANDATORY
				{
					if (StringTools::Stricmp(property_type, "int") == 0)
					{
						InsertPropertyInt(property_name, value_attribute->IntValue());
					}
					else if (StringTools::Stricmp(property_type, "float") == 0)
					{
						InsertPropertyFloat(property_name, value_attribute->FloatValue());
					}
					else if (StringTools::Stricmp(property_type, "bool") == 0)
					{
						InsertPropertyBool(property_name, value_attribute->BoolValue());
					}
					else if (StringTools::Stricmp(property_type, "color") == 0)
					{
						glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
						if (ReadXMLColor(value_attribute->Value(), color))
							InsertPropertyColor(property_name, color);					
					}
					else if (StringTools::Stricmp(property_type, "object") == 0)
					{
						InsertPropertyObject(property_name, value_attribute->IntValue());
					}
				}
			}
			return true;
		}

#define CHAOS_INSERT_PROPERTY(suffix, result_type, arg_type)\
		result_type * PropertyOwner::InsertProperty##suffix(char const * name, arg_type value)\
		{\
			result_type* result = new result_type(this);\
			if (result != nullptr)\
			{\
				result->name = name;\
				result->value = value;\
				properties.push_back(result);\
			}\
			return result;\
		}
		CHAOS_INSERT_PROPERTY(Int, PropertyInt, int);
		CHAOS_INSERT_PROPERTY(Float, PropertyFloat, float);
		CHAOS_INSERT_PROPERTY(Bool, PropertyBool, bool);
		CHAOS_INSERT_PROPERTY(String, PropertyString, char const *);
		CHAOS_INSERT_PROPERTY(Color, PropertyColor, glm::vec4 const &);
		CHAOS_INSERT_PROPERTY(Object, PropertyObject, int);
#undef CHAOS_INSERT_PROPERTY

		bool PropertyOwner::IsObjectOfType(char const* in_type) const
		{
			Property const* property = FindProperty(in_type);
			if (property != nullptr)
			{
				bool const* property_bool = property->GetBoolProperty();
				if (property_bool != nullptr)
					return *property_bool;

				int const* property_int = property->GetIntProperty();
				if (property_int != nullptr)
					return (*property_int > 0);
			}
			return false;
		}

		// ==========================================
		// TypedObject methods
		// ==========================================

		bool TypedObject::IsObjectOfType(char const* in_type) const
		{
			if (chaos::StringTools::Stricmp(type, in_type) == 0)
				return true;
			return PropertyOwner::IsObjectOfType(in_type);
		}

		Property * TypedObject::FindProperty(char const * name, PropertyType type_id)
		{
			Property * result = PropertyOwner::FindProperty(name, type_id);
			if (result == nullptr && !StringTools::IsEmpty(type))
			{
				Manager * manager = GetManager();
				if (manager != nullptr)
					result = manager->FindObjectProperty(type.c_str(), name, type_id);
			}
			return result;
		}

		Property const * TypedObject::FindProperty(char const * name, PropertyType type_id) const
		{
			Property const * result = PropertyOwner::FindProperty(name, type_id);
			if (result == nullptr && !StringTools::IsEmpty(type))
			{
				Manager const * manager = GetManager();
				if (manager != nullptr)
					result = manager->FindObjectProperty(type.c_str(), name, type_id);
			}
			return result;
		}

		bool TypedObject::DoLoad(tinyxml2::XMLElement const* element)
		{
			if (!PropertyOwner::DoLoad(element))
				return false;
			XMLTools::ReadAttribute(element, "type", type);
			return true;
		}

		// ==========================================
		// GeometricObject methods
		// ==========================================

		std::vector<glm::vec2> GeometricObject::GetPointArray(tinyxml2::XMLElement const * element, char const * attribute_name)
		{
			std::vector<glm::vec2> result;

			tinyxml2::XMLAttribute const * attribute = element->FindAttribute(attribute_name);
			if (attribute != nullptr)
			{
				char const * values = attribute->Value();
				if (values != nullptr)
				{
					glm::vec2 p = glm::vec2(0.0f, 0.0f);

					int coord = 0;
					int i = 0;
					while (values[i] != 0)
					{
						p[coord++] = (float)atof(&values[i]);
						if (coord == 2)
						{
#if CHAOS_REVERSE_Y_AXIS
							result.push_back(glm::vec2(p.x, -p.y));
#else
							result.push_back(p);
#endif
							p = glm::vec2(0.0f, 0.0f); // flush the point if both axis are found
							coord = 0;
						}

						while (values[i] != 0 && values[i] != ',' && values[i] != ' ') // skip until separator
							++i;
						if (values[i] == ',' || values[i] == ' ')
							++i;
					}
					// flush the point even if there is a missing axis
					if (coord == 1)
					{
#if CHAOS_REVERSE_Y_AXIS
						result.push_back(glm::vec2(p.x, -p.y));
#else
						result.push_back(p);
#endif
					}
				}
			}
			return result;
		}

		bool GeometricObject::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!TypedObject::DoLoad(element))
				return false;
			XMLTools::ReadAttribute(element, "id", id);
			XMLTools::ReadAttribute(element, "name", name);
			XMLTools::ReadAttribute(element, "visible", visible);
			XMLTools::ReadAttribute(element, "x", position.x);
			XMLTools::ReadAttribute(element, "y", position.y);
			XMLTools::ReadAttribute(element, "rotation", rotation);

			// remove useless space in type
			if (type.length() > 0)
				type = StringTools::TrimString(type.c_str(), true, true);

			// reverse the Y axis
#if CHAOS_REVERSE_Y_AXIS
			position.y = -position.y;
#endif
			return true;
		}

		bool GeometricObjectSurface::DoLoad(tinyxml2::XMLElement const* element)
		{
			if (!GeometricObject::DoLoad(element))
				return false;
			XMLTools::ReadAttribute(element, "width", size.x);
			XMLTools::ReadAttribute(element, "height", size.y);
			return true;
		}

		bool GeometricObjectPoint::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!GeometricObject::DoLoad(element))
				return false;
			return true;
		}

		bool GeometricObjectRectangle::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!GeometricObjectSurface::DoLoad(element))
				return false;
			return true;
		}

		bool GeometricObjectEllipse::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!GeometricObjectSurface::DoLoad(element))
				return false;
			return true;
		}

		bool GeometricObjectPolygon::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!GeometricObject::DoLoad(element))
				return false;
			tinyxml2::XMLElement const * polygon_element = element->FirstChildElement("polygon");
			points = GetPointArray(polygon_element, "points");
			return true;
		}

		bool GeometricObjectPolyline::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!GeometricObject::DoLoad(element))
				return false;
			tinyxml2::XMLElement const * polygon_element = element->FirstChildElement("polyline");
			points = GetPointArray(polygon_element, "points");
			return true;
		}

		bool GeometricObjectText::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!GeometricObjectSurface::DoLoad(element))
				return false;

			tinyxml2::XMLElement const * text_element = element->FirstChildElement("text");

			static std::vector<std::pair<HorizontalTextAlignment, char const*>> const halign_map = {
				{ HorizontalTextAlignment::LEFT, "left" },
				{ HorizontalTextAlignment::CENTER, "center" },
				{ HorizontalTextAlignment::RIGHT, "right" },
				{ HorizontalTextAlignment::JUSTIFY, "justify" },
				{ HorizontalTextAlignment::LEFT, nullptr }
			};
			XMLTools::ReadEnumAttribute(text_element, "halign", halign_map, halign);

			static std::vector<std::pair<VerticalTextAlignment, char const*>> const valign_map = {
				{ VerticalTextAlignment::TOP, "top" },
				{ VerticalTextAlignment::CENTER, "center" },
				{ VerticalTextAlignment::BOTTOM, "bottom" },
				{ VerticalTextAlignment::TOP, nullptr  }
			};
			XMLTools::ReadEnumAttribute(text_element, "valign", valign_map, valign);

			XMLTools::ReadAttribute(text_element, "pixelsize", pixelsize);
			XMLTools::ReadAttribute(text_element, "wrap", wrap);
			XMLTools::ReadAttribute(text_element, "fontfamily", fontfamily);
			ReadXMLColor(text_element, "color", color);

			char const * txt = text_element->GetText();
			if (txt != nullptr)
				text = txt;

			return true;
		}

		bool GeometricObjectTile::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!GeometricObjectSurface::DoLoad(element))
				return false;

			int pseudo_gid = 0;// this is a pseudo_gid, because the Vertical & Horizontal flipping is encoded inside this value
			XMLTools::ReadAttribute(element, "gid", pseudo_gid);

			gid = TiledMapTools::DecodeTileGID(pseudo_gid, &horizontal_flip, &vertical_flip, &diagonal_flip);

			return true;
		}

		// XXX : SIZE's signification may be different according to each object
		//
		//	GeometricObjectSurface : abstract
		//
		//	GeometricObjectPoint : position = OBVIOUS
		//
		//	GeometricObjectRectangle : SIZE.Y is going DOWN for positive Y (according to the system)
		//	GeometricObjectEllipse
		//	GeometricObjectText
		//
		//	GeometricObjectTile : SIZE.Y is going UP for positive Y !!!! (the opposite of other objects)
		//
		//	GeometricObjectPolygon   : position = coordinates of very first point
		//	GeometricObjectPolyline  : other points are given relative to the position

		box2 GeometricObjectSurface::GetBoundingBox(bool world_system) const
		{
			//CHAOS_REVERSE_Y_AXIS
			glm::vec2 p1 = position;
			glm::vec2 p2 = glm::vec2(position.x + size.x, position.y - size.y);

			box2 result = box2(std::make_pair(p1, p2));
			if (world_system)
			{
				LayerBase const * parent_layer = auto_cast(owner);
				if (parent_layer != nullptr)
					result.position += parent_layer->offset;
			}
			return result;
		}

		box2 GeometricObjectTile::GetBoundingBox(bool world_system) const
		{
			//CHAOS_REVERSE_Y_AXIS
			glm::vec2 p1 = position;
			glm::vec2 p2 = glm::vec2(position.x + size.x, position.y + size.y);

			//glm::vec2 p1 = position;
			//glm::vec2 p2 = position;
			//p2.x += size.x;
			//p2.y -= size.y; // axis Y is DOWN !!!

			box2 result = box2(std::make_pair(p1, p2));
			if (world_system)
			{
				LayerBase const * parent_layer = auto_cast(owner);
				if (parent_layer != nullptr)
					result.position += parent_layer->offset;
			}
			return result;
		}

		TileInfo GeometricObjectTile::FindTileInfo() const
		{
			Map const * tiled_map = GetMap();
			if (tiled_map != nullptr)
				return tiled_map->FindTileInfo(gid);
			return TileInfo();
		}

		bool GeometricObjectTile::IsObjectOfType(char const* in_type) const
		{
			if (GeometricObjectSurface::IsObjectOfType(in_type))
				return true;

			TileInfo tile_info = FindTileInfo();
			if (tile_info.tiledata != nullptr)
				if (tile_info.tiledata->IsObjectOfType(in_type))
					return true;
			return false;
		}

		// XXX : for a ObjectTile, a property may be localized in 4 places
		//
		//       -the PROPERTY_MAP of the object
		//       -the TYPE1 of the object
		//       -the TILE_DATA 
		//           -PROPERTY_MAP 
		//           -TYPE2 (that may differ from TYPE1)
		//
		//       the PROPERTY_MAP of the object is obviously the very first place where to look at for a property (while it is the finer customization we can do)
		//
		//       we can say that TILE_DATA.PROPERTY_MAP is to be searched before TILE_DATA.TYPE2 (obvious)
		//

		Property* GeometricObjectTile::FindProperty(char const* name, PropertyType type_id)
		{
			// 1 - Own properties
			Property* result = PropertyOwner::FindProperty(name, type_id);
			if (result != nullptr)
				return result;

			// 2 - See Tile properties
			TileInfo tile_info = FindTileInfo();
			if (tile_info.tiledata != nullptr)
			{
				result = tile_info.tiledata->FindProperty(name, type_id);
				if (result != nullptr)
					return result;
			}

			// 3 - See our own type
			if (!StringTools::IsEmpty(type))
			{
				if (tile_info.tiledata == nullptr || StringTools::Stricmp(type, tile_info.tiledata->type) != 0) // if TYPE1 == TYPE2, do not search twice
				{
					Manager* manager = GetManager();
					if (manager != nullptr)
					{
						result = manager->FindObjectProperty(type.c_str(), name, type_id);
						if (result != nullptr)
							return result;
					}
				}
			}
			return result;
		}

		Property const* GeometricObjectTile::FindProperty(char const* name, PropertyType type_id) const
		{
			// 1 - Own properties
			Property const* result = PropertyOwner::FindProperty(name, type_id);
			if (result != nullptr)
				return result;

			// 2 - See Tile properties
			TileInfo tile_info = FindTileInfo();
			if (tile_info.tiledata != nullptr)
			{
				result = tile_info.tiledata->FindProperty(name, type_id);
				if (result != nullptr)
					return result;
			}

			// 3 - See our own type
			if (!StringTools::IsEmpty(type))
			{
				if (tile_info.tiledata == nullptr || StringTools::Stricmp(type, tile_info.tiledata->type) != 0) // if TYPE1 == TYPE2, do not search twice
				{
					Manager const * manager = GetManager();
					if (manager != nullptr)
					{
						result = manager->FindObjectProperty(type.c_str(), name, type_id);
						if (result != nullptr)
							return result;
					}
				}
			}
			return result;
		}

		// ==========================================
		// GroundData methods
		// ==========================================

		bool GroundData::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!PropertyOwner::DoLoad(element))
				return false;
			XMLTools::ReadAttribute(element, "tile", tile_index);
			XMLTools::ReadAttribute(element, "name", name);
			return true;
		}

		// ==========================================
		// TileData methods
		// ==========================================

		bool TileData::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!TypedObject::DoLoad(element))
				return false;

			XMLTools::ReadAttribute(element, "id", id);
			XMLTools::ReadAttribute(element, "probability", probability);

			std::string terrain;
			if (XMLTools::ReadAttribute(element, "terrain", terrain))
				if (!ComputeTerrainIndices(terrain.c_str()))
					return false;

			// some tilesets have a single image (representing an atlas with elements in a grid cells)
			// some tilesets have individual images per tiles
			tinyxml2::XMLElement const * image_element = element->FirstChildElement("image");
			if (image_element != nullptr)
			{
				if (!XMLTools::ReadAttribute(image_element, "source", image_path))
					return false;
				if (!XMLTools::ReadAttribute(image_element, "width", image_size.x))
					return false;
				if (!XMLTools::ReadAttribute(image_element, "height", image_size.y))
					return false;

				image_path = BoostTools::FindAbsolutePath(GetOwnerPath(), image_path);
				atlas_key = chaos::BoostTools::PathToName(image_path);
			}

			DoLoadObjectListHelper(element, object_layers, "objectgroup", nullptr, this);

			return true;
		}

		// "0,,," => top left
		// ",0,," => top right
		// ",,0," => bottom left
		// ",,,0" => bottom right
		bool TileData::ComputeTerrainIndices(char const * str)
		{
			assert(str != nullptr);

			// by default initialization
			for (size_t index = 0; index < 4; ++index)
				terrain_indices[index] = -1;

			// parse the string
			size_t index = 0;

			size_t i = 0;
			while (str[i] != 0)
			{
				if (std::isdigit(str[i]))
				{
					terrain_indices[index] = atoi(&str[i]);
					i = StringTools::SkipNumber(&str[i]) - str;
					if (index == 4)
						break;
				}
				else
				{
					if (str[i] == ',')
						++index;
					++i;
				}
			}
			return true;
		}

		// ==========================================
		// ManagerObject methods
		// ==========================================

		ManagerObject::ManagerObject(BaseObject * in_owner, boost::filesystem::path in_path) :
			PropertyOwner(in_owner),
			path(std::move(in_path))
		{
		}

		bool ManagerObject::IsMatchingName(boost::filesystem::path const & in_path) const
		{
			return (path.compare(in_path) == 0);
		}

		bool ManagerObject::DoLoadDocument(tinyxml2::XMLDocument const * doc)
		{
			assert(doc != nullptr);

			tinyxml2::XMLElement const * element = doc->RootElement();
			if (element == nullptr)
				return false;

			if (StringTools::Stricmp(element->Name(), GetXMLMarkupName()) != 0)
				return false;

			return DoLoad(element);
		}

		bool ManagerObject::DoLoad(tinyxml2::XMLElement const * element)
		{
			assert(element != nullptr);
			if (!PropertyOwner::DoLoad(element))
				return false;
			if (!DoLoadMembers(element))
				return false;
			return true;
		}

		// ==========================================
		// LayerBase methods
		// ==========================================

		bool LayerBase::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!PropertyOwner::DoLoad(element))
				return false;
			XMLTools::ReadAttribute(element, "id", id);
			XMLTools::ReadAttribute(element, "name", name);
			XMLTools::ReadAttribute(element, "visible", visible);
			XMLTools::ReadAttribute(element, "locked", locked);
			XMLTools::ReadAttribute(element, "opacity", opacity);
			XMLTools::ReadAttribute(element, "offsetx", offset.x);
			XMLTools::ReadAttribute(element, "offsety", offset.y);

			// reverse the Y axis
#if CHAOS_REVERSE_Y_AXIS
			offset.y = -offset.y;
#endif
			// compute the order of the layer in the map
			// XXX : the layers in the JSON are given in reversed order than in TileMap Editor
			//       we have to fix that later in the loading code
			Map * map = GetOwner<Map>();
			if (map != nullptr)
				zorder = map->GetLayerCount();

			return true;
		}

		// ==========================================
		// ImageLayer methods
		// ==========================================

		bool ImageLayer::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!LayerBase::DoLoad(element))
				return false;

			tinyxml2::XMLElement const * image_source = element->FirstChildElement("image");
			if (image_source != nullptr)
			{
				ReadXMLColor(image_source, "trans", transparent_color);
				XMLTools::ReadAttribute(image_source, "width", size.x);
				XMLTools::ReadAttribute(image_source, "height", size.y);

				XMLTools::ReadAttribute(image_source, "source", image_path);
				image_path = BoostTools::FindAbsolutePath(GetOwnerPath(), image_path);
			}
			return true;
		}

		// ==========================================
		// LayerBase methods
		// ==========================================

		bool ObjectLayer::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!LayerBase::DoLoad(element))
				return false;

			ReadXMLColor(element, "color", color);

			static std::vector<std::pair<DrawOrder, char const*>> const draw_order_map = {
				{ DrawOrder::MANUAL, "index"  },
				{ DrawOrder::TOPDOWN, nullptr }
			};
			XMLTools::ReadEnumAttribute(element, "draworder", draw_order_map, draw_order);

			if (!DoLoadObjects(element))
				return false;

			return true;
		}

		GeometricObject * ObjectLayer::DoLoadOneObject(tinyxml2::XMLElement const * element)
		{
			// tile ?
			int pseudo_gid = 0;
			if (XMLTools::ReadAttribute(element, "gid", pseudo_gid)) // this is a pseudo_gid, because the Vertical & Horizontal flipping is encoded inside this value
				return new GeometricObjectTile(this);

			// ellipse ?
			tinyxml2::XMLElement const * ellipse_element = element->FirstChildElement("ellipse");
			if (ellipse_element != nullptr)
				return new GeometricObjectEllipse(this);

			// text ?
			tinyxml2::XMLElement const * text_element = element->FirstChildElement("text");
			if (text_element != nullptr)
				return new GeometricObjectText(this);

			// polygon ?
			tinyxml2::XMLElement const * polygon_element = element->FirstChildElement("polygon");
			if (polygon_element != nullptr)
				return new GeometricObjectPolygon(this);

			// polyline ?
			tinyxml2::XMLElement const * polyline_element = element->FirstChildElement("polyline");
			if (polyline_element != nullptr)
				return new GeometricObjectPolyline(this);

			// point ?
			tinyxml2::XMLElement const * point_element = element->FirstChildElement("point");
			if (point_element != nullptr)
				return new GeometricObjectPoint(this);

			// rectangle ?
			return new GeometricObjectRectangle(this);
		}

		bool ObjectLayer::DoLoadObjects(tinyxml2::XMLElement const * element)
		{
			tinyxml2::XMLElement const * e = element->FirstChildElement("object");
			for (; e != nullptr; e = e->NextSiblingElement("object"))
			{
				GeometricObject * object = DoLoadOneObject(e);
				if (object == nullptr)
					continue;
				if (!object->DoLoad(e))
				{
					delete(object);
					continue;
				}
				geometric_objects.push_back(object);
			}
			return true;
		}

		// ==========================================
		// TileLayer methods
		// ==========================================

		box2 TileLayer::GetTileBoundingBox(glm::ivec2 const tile_coord, glm::vec2 const & image_size, bool world_system) const
		{
			//CHAOS_REVERSE_Y_AXIS
			glm::vec2 p1 = glm::vec2(
				(float)(tile_coord.x * tile_size.x),
				(float)(-tile_coord.y * tile_size.y - tile_size.y));
			glm::vec2 p2 = p1;
			p2.x += image_size.x;
			p2.y += image_size.y;
			box2 result = box2(std::make_pair(p1, p2));
			if (world_system)
				result.position += offset;
			return result;
		}

		bool TileLayer::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!LayerBase::DoLoad(element))
				return false;

			XMLTools::ReadAttribute(element, "width", size.x);
			XMLTools::ReadAttribute(element, "height", size.y);

			if (!DoLoadTileBuffer(element))
				return false;

			return true;
		}

		void TileLayer::DoLoadTileBufferFromBase64(Buffer<char> const & buffer)
		{
			size_t count = size.x * size.y;

			if (buffer.bufsize != count * sizeof(uint32_t))  // array width * height * sizeof(uint32)
				return;

			// transform the char buffer into tiles
			for (size_t i = 0; i < count; ++i)
			{
				unsigned int a = (unsigned int)buffer[i * 4 + 0];
				unsigned int b = (unsigned int)buffer[i * 4 + 1];
				unsigned int c = (unsigned int)buffer[i * 4 + 2];
				unsigned int d = (unsigned int)buffer[i * 4 + 3];

				int tile = (a << 0) | (b << 8) | (c << 16) | (d << 24);
				tile_indices.push_back(tile);
			}
		}

		bool TileLayer::DoLoadTileBuffer(tinyxml2::XMLElement const * element)
		{
			tinyxml2::XMLElement const * data = element->FirstChildElement("data");
			if (data == nullptr)
				return false;


			// shuzzz
			// beware : for infinite map, there is a junk level of indirection
#if 0
			<data encoding = "base64" compression = "zlib" >
				<chunk x = "0" y = "48" width = "16" height = "16">
				eAFjYBgFoyEwGgLDIQQYyfAEAAQ0AAI =
				</chunk>
			</ data>
#endif







			std::string encoding;
			XMLTools::ReadAttribute(data, "encoding", encoding);

			// read the data
			size_t count = size.x * size.y;
			tile_indices.reserve(count);

			if (encoding == "base64")
			{
				std::string compression;
				XMLTools::ReadAttribute(data, "compression", compression);

				if (compression == "gzip")
				{

					// TODO

				}
				else if (compression == "zlib")
				{
					std::string content = StringTools::TrimBase64String(data->GetText());

					Buffer<char> base64 = MyBase64().Decode(content.c_str());
					Buffer<char> decoded = MyZLib().Decode(base64);

					DoLoadTileBufferFromBase64(decoded);
				}
				else // no encoding
				{
					std::string content = StringTools::TrimBase64String(data->GetText());

					Buffer<char> base64 = MyBase64().Decode(content.c_str());

					DoLoadTileBufferFromBase64(base64);
				}
			}
			else if (encoding == "csv")
			{
				char const * txt = data->GetText();

				int i = 0;
				while (txt[i] != 0 && tile_indices.size() != count)
				{
					while (txt[i] != 0 && !isdigit(txt[i])) // search first figure
						++i;
					if (txt[i] == 0)
						break;

					int value = atoi(&txt[i]);
					tile_indices.push_back(value); // get and push the tile

					while (txt[i] != 0 && isdigit(txt[i])) // skip all figures
						++i;
				}
			}
			else // else XML
			{
				tinyxml2::XMLElement const * child = data->FirstChildElement("tile");
				while (child != nullptr && tile_indices.size() != count)
				{
					int value = 0;
					XMLTools::ReadAttribute(child, "gid", value);
					tile_indices.push_back(value);

					child = child->NextSiblingElement("tile");
				}
			}
			return (tile_indices.size() == count);
		}

		size_t TileLayer::GetNonEmptyTileCount() const
		{
			size_t result = 0;
			for (size_t i = 0; i < tile_indices.size(); ++i)
				if (tile_indices[i] > 0)
					++result;
			return result;
		}

		glm::ivec2 TileLayer::GetTileCoordinate(size_t index) const
		{
			int tmp = (int)index;
			return glm::ivec2(tmp % size.x, tmp / size.x);
		}

		// ==========================================
		// ObjectTypeDefinition methods
		// ==========================================

		bool ObjectTypeDefinition::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!PropertyOwner::DoLoad(element))
				return false;
			XMLTools::ReadAttribute(element, "name", name);
			ReadXMLColor(element, "color", color);
			return true;
		}

		// ==========================================
		// ObjectTypeSet methods
		// ==========================================

		bool ObjectTypeSet::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!ManagerObject::DoLoad(element))
				return false;
			if (!DoLoadObjectTypes(element))
				return false;
			return true;
		}

		bool ObjectTypeSet::DoLoadObjectTypes(tinyxml2::XMLElement const * element)
		{
			if (!DoLoadObjectListHelper(element, object_types, "objecttype", nullptr, this))
				return false;
			return true;
		}

		bool ObjectTypeSet::DoLoadMembers(tinyxml2::XMLElement const * element)
		{
			return true;
		}

		ObjectTypeDefinition * ObjectTypeSet::FindObjectType(char const * name)
		{
			size_t count = object_types.size();
			for (size_t i = 0; i < count; ++i)
			{
				ObjectTypeDefinition * definition = object_types[i].get();
				if (definition == nullptr)
					continue;
				if (StringTools::Stricmp(definition->name, name) == 0)
					return definition;
			}
			return nullptr;
		}

		ObjectTypeDefinition const * ObjectTypeSet::FindObjectType(char const * name) const
		{
			size_t count = object_types.size();
			for (size_t i = 0; i < count; ++i)
			{
				ObjectTypeDefinition const * definition = object_types[i].get();
				if (definition == nullptr)
					continue;
				if (StringTools::Stricmp(definition->name, name) == 0)
					return definition;
			}
			return nullptr;
		}

		Property * ObjectTypeSet::FindObjectProperty(char const * type, char const * name, PropertyType type_id)
		{
			if (StringTools::IsEmpty(type))
				return nullptr;
			ObjectTypeDefinition * definition = FindObjectType(type);
			if (definition == nullptr)
				return nullptr;
			return definition->FindProperty(name, type_id);
		}

		Property const * ObjectTypeSet::FindObjectProperty(char const * type, char const * name, PropertyType type_id) const
		{
			if (StringTools::IsEmpty(type))
				return nullptr;
			ObjectTypeDefinition const * definition = FindObjectType(type);
			if (definition == nullptr)
				return nullptr;
			return definition->FindProperty(name, type_id);
		}

		// ==========================================
		// TileSet methods
		// ==========================================

#define CHAOS_IMPL_FIND_FILE_DATA(func_name, arg_type, member_name, constess)\
		TileData constess * TileSet::func_name(arg_type arg_name) constess\
		{\
			size_t count = tiles.size();\
			for (size_t i = 0; i < count; ++i)\
			{\
				TileData constess * tile_data = tiles[i].get();\
				if (tile_data == nullptr)\
					continue;\
				if (tile_data->member_name == arg_name)\
					return tile_data;\
			}\
			return nullptr;\
		}

		CHAOS_IMPL_FIND_FILE_DATA(FindTileData, int, id, BOOST_PP_EMPTY())
			CHAOS_IMPL_FIND_FILE_DATA(FindTileData, int, id, const)
			CHAOS_IMPL_FIND_FILE_DATA(FindTileData, char const *, type, BOOST_PP_EMPTY())
			CHAOS_IMPL_FIND_FILE_DATA(FindTileData, char const *, type, const)
			CHAOS_IMPL_FIND_FILE_DATA(FindTileDataFromAtlasKey, char const *, atlas_key, BOOST_PP_EMPTY())
			CHAOS_IMPL_FIND_FILE_DATA(FindTileDataFromAtlasKey, char const *, atlas_key, const)

#undef CHAOS_IMPL_FIND_FILE_DATA


			bool TileSet::DoLoadGrounds(tinyxml2::XMLElement const * element)
		{
			return DoLoadObjectListHelper(element, grounds, "terrain", "terraintypes", this);
		}

		bool TileSet::DoLoadTiles(tinyxml2::XMLElement const * element)
		{
			// load the tiles
			if (!DoLoadObjectListHelper(element, tiles, "tile", nullptr, this))
				return false;
			return true;
		}

		bool TileSet::DoLoadMembers(tinyxml2::XMLElement const * element)
		{
			XMLTools::ReadAttribute(element, "name", name);
			XMLTools::ReadAttribute(element, "tilewidth", tile_size.x);
			XMLTools::ReadAttribute(element, "tileheight", tile_size.y);
			XMLTools::ReadAttribute(element, "tilecount", tile_count);
			XMLTools::ReadAttribute(element, "columns", columns);
			XMLTools::ReadAttribute(element, "margin", image_margin);
			XMLTools::ReadAttribute(element, "spacing", image_spacing);

			ReadXMLColor(element, "backgroundcolor", background_color);

			tinyxml2::XMLElement const * grid_element = element->FirstChildElement("grid");
			if (grid_element != nullptr)
			{
				static std::vector<std::pair<TileSetOrientation, char const*>> const orientation_map = {
					{ TileSetOrientation::ISOMETRIC, "isometric" },
					{ TileSetOrientation::ORTHOGONAL, nullptr }
				};
				XMLTools::ReadEnumAttribute(grid_element, "orientation", orientation_map, orientation);

				XMLTools::ReadAttribute(grid_element, "width", size.x);
				XMLTools::ReadAttribute(grid_element, "height", size.y);
			}

			tinyxml2::XMLElement const * image_element = element->FirstChildElement("image");
			if (image_element != nullptr)
			{
				XMLTools::ReadAttribute(image_element, "source", image_path);
				image_path = BoostTools::FindAbsolutePath(GetOwnerPath(), image_path);

				ReadXMLColor(image_element, "trans", transparent_color);

				XMLTools::ReadAttribute(image_element, "width", image_size.x);
				XMLTools::ReadAttribute(image_element, "height", image_size.y);
			}

			return true;
		}

		bool TileSet::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!ManagerObject::DoLoad(element))
				return false;
			if (!DoLoadTiles(element))
				return false;
			if (!DoLoadGrounds(element))
				return false;
			return true;
		}

		// ==========================================
		// Map methods
		// ==========================================

		bool Map::DoLoadMembers(tinyxml2::XMLElement const * element)
		{
			static std::vector<std::pair<MapOrientation, char const*>> const orient_map = {
				{ MapOrientation::ORTHOGONAL, "orthogonal" },
				{ MapOrientation::ISOMETRIC, "isometric" },
				{ MapOrientation::STAGGERED, "staggered" },
				{ MapOrientation::HEXAGONAL, "hexagonal" },
				{ MapOrientation::ORTHOGONAL, nullptr }
			};
			XMLTools::ReadEnumAttribute(element, "orientation", orient_map, orientation);

			static std::vector < std::pair<StaggerAxis, char const*>> const stagger_axis_map = {
				{ StaggerAxis::AXIS_X, "X",  },
				{ StaggerAxis::AXIS_Y, "Y",  },
				{ StaggerAxis::AXIS_X, nullptr }
			};
			XMLTools::ReadEnumAttribute(element, "staggeraxis", stagger_axis_map, stagger_axis);

			static std::vector<std::pair<StaggerIndex, char const*>> const stagger_index_map = {
				{ StaggerIndex::ODD, "odd" },
				{ StaggerIndex::EVEN, "even" },				
				{ StaggerIndex::ODD, nullptr }
			};
			XMLTools::ReadEnumAttribute(element, "staggerindex", stagger_index_map, stagger_index);

			static std::vector<std::pair<RenderOrder, char const*>> const render_order_map = {
				{ RenderOrder::RIGHT_UP, "right-up" },
				{ RenderOrder::RIGHT_DOWN, "right-down" },
				{ RenderOrder::LEFT_UP, "left-up" },
				{ RenderOrder::LEFT_DOWN, "left-down" },
				{ RenderOrder::RIGHT_UP, nullptr }
			};
			XMLTools::ReadEnumAttribute(element, "renderorder", render_order_map, render_order);

            XMLTools::ReadAttribute(element, "compressionlevel", compressionlevel);
			XMLTools::ReadAttribute(element, "version", version);
			XMLTools::ReadAttribute(element, "width", size.x);
			XMLTools::ReadAttribute(element, "height", size.y);
			XMLTools::ReadAttribute(element, "tilewidth", tile_size.x);
			XMLTools::ReadAttribute(element, "tileheight", tile_size.y);
			XMLTools::ReadAttribute(element, "infinite", infinite);
			XMLTools::ReadAttribute(element, "hexsidelength", hex_side_length);

			ReadXMLColor(element, "backgroundcolor", background_color);

			return true;
		}

		bool Map::DoLoad(tinyxml2::XMLElement const * element)
		{
			if (!ManagerObject::DoLoad(element))
				return false;
			if (!DoLoadTileSet(element))
				return false;
			if (!DoLoadLayers(element))
				return false;
			return true;
		}

		bool Map::DoLoadTileSet(tinyxml2::XMLElement const * element)
		{
			assert(element != nullptr);

			// get the manager
			Manager * manager = GetOwner<Manager>();
			if (manager == nullptr)
				return false;

			tinyxml2::XMLElement const * tileset_element = element->FirstChildElement("tileset");
			for (; tileset_element != nullptr; tileset_element = tileset_element->NextSiblingElement("tileset"))
			{
				int first_gid = 0;
				std::string source;

				if (!XMLTools::ReadAttribute(tileset_element, "firstgid", first_gid)) // firstgid is mandatory (map would be incomplete)
					return false;

				// external tileset
				TileSet * tileset = nullptr;

				if (XMLTools::ReadAttribute(tileset_element, "source", source)) // source is mandatory (map would be incomplete)
				{
					boost::filesystem::path tileset_path = BoostTools::FindAbsolutePath(path, source); // compute the path of the tileset relative to this

					tileset = manager->LoadTileSet(tileset_path);
					if (tileset == nullptr)
						return false;
				}
				// embedded titleset
				else
				{
					tileset = new TileSet(this, boost::filesystem::path());
					if (tileset == nullptr)
						return false;

					if (!tileset->DoLoad(tileset_element))
					{
						delete(tileset);
						return false;
					}
					manager->tile_sets.push_back(tileset);
				}

				TileSetData data;
				data.first_gid = first_gid;
				data.tileset = tileset;
				tilesets.push_back(data);
			}
			return true;
		}

		int Map::GetLayerCount() const
		{
			return (int)(image_layers.size() + tile_layers.size() + object_layers.size());
		}

		bool Map::DoLoadLayers(tinyxml2::XMLElement const * element)
		{
			// get all layers
			// XXX : the very first encoutered layer, is the one that should be rendered last.
			//       that why we proceed in reverse order
			tinyxml2::XMLElement const * e = element->LastChildElement();
			for (; e != nullptr; e = e->PreviousSiblingElement())
			{
				LayerBase * new_layer = nullptr;

				char const * child_name = e->Name();
				if (StringTools::Stricmp(child_name, "imagelayer") == 0)
					new_layer = DoLoadObjectAndInserInList(e, image_layers, this);
				else if (StringTools::Stricmp(child_name, "objectgroup") == 0)
					new_layer = DoLoadObjectAndInserInList(e, object_layers, this);
				else if (StringTools::Stricmp(child_name, "layer") == 0)
					new_layer = DoLoadObjectAndInserInList(e, tile_layers, this, tile_size);
			}
			// now fix the zorders
			if (!DoFixLayersZOrder())
				return false;
			return true;
		}

		bool Map::DoFixLayersZOrder()
		{
			int layer_count = GetLayerCount();

			for (auto layer : image_layers)
				if (layer != nullptr)
					layer->zorder = (layer_count - 1) - layer->zorder;
			for (auto layer : tile_layers)
				if (layer != nullptr)
					layer->zorder = (layer_count - 1) - layer->zorder;
			for (auto layer : object_layers)
				if (layer != nullptr)
					layer->zorder = (layer_count - 1) - layer->zorder;

			return true;
		}

		TileInfo Map::FindTileInfo(int gid)
		{
			if (gid > 0)
			{
				size_t count = tilesets.size();
				for (size_t i = count; i > 0; --i)
				{
					size_t index = i - 1;

					TileSetData & data = tilesets[index];
					if (gid >= data.first_gid)
					{
						TileData * tiledata = data.tileset->FindTileData(gid - data.first_gid);
						if (tiledata != nullptr)
							return TileInfo((gid - data.first_gid), data.tileset.get(), tiledata);
					}
				}
			}
			return TileInfo();
		}

		TileInfo const Map::FindTileInfo(int gid) const
		{
			if (gid >= 0)
			{
				size_t count = tilesets.size();
				for (size_t i = count; i > 0; --i)
				{
					size_t index = i - 1;

					TileSetData const & data = tilesets[index];
					if (gid >= data.first_gid)
					{
						TileData * tiledata = data.tileset->FindTileData(gid - data.first_gid);
						if (tiledata != nullptr)
							return TileInfo((gid - data.first_gid), data.tileset.get(), tiledata);
					}
				}
			}
			return TileInfo();
		}

#define CHAOS_IMPL_FIND_LAYER(func_name, member_name, arg_type, constess)\
	LayerBase constess * Map::func_name(arg_type arg_name) constess\
	{\
		for (auto & it : image_layers)\
			if (it->member_name == arg_name)\
				return it.get();\
		for (auto & it : tile_layers)\
			if (it->member_name == arg_name)\
				return it.get();\
		for (auto & it : object_layers)\
			if (it->member_name == arg_name)\
				return it.get();\
		return nullptr;\
	}

		CHAOS_IMPL_FIND_LAYER(FindLayerByName, name, char const *, BOOST_PP_EMPTY())
			CHAOS_IMPL_FIND_LAYER(FindLayerByName, name, char const *, const)
			CHAOS_IMPL_FIND_LAYER(FindLayerByZOrder, zorder, size_t, BOOST_PP_EMPTY())
			CHAOS_IMPL_FIND_LAYER(FindLayerByZOrder, zorder, size_t, const)
#undef CHAOS_IMPL_FIND_LAYER

#define CHAOS_IMPL_FIND_FILE_INFO(func_name, sub_funcname, arg_type, constess)\
		TileInfo constess Map::func_name(arg_type arg_name) constess\
		{\
			size_t count = tilesets.size();\
			for (size_t i = 0 ; i < count; ++i)\
			{\
				TileSetData const & data = tilesets[i];\
				if (data.tileset != nullptr)\
				{\
					TileData * tiledata = data.tileset->sub_funcname(arg_name);\
					if (tiledata != nullptr)\
						return TileInfo(-1, data.tileset.get(), tiledata);\
				}\
			}\
			return TileInfo();\
		}

			CHAOS_IMPL_FIND_FILE_INFO(FindTileInfo, FindTileData, char const *, BOOST_PP_EMPTY())
			CHAOS_IMPL_FIND_FILE_INFO(FindTileInfo, FindTileData, char const *, const)
			CHAOS_IMPL_FIND_FILE_INFO(FindTileInfoFromAtlasKey, FindTileDataFromAtlasKey, char const *, BOOST_PP_EMPTY())
			CHAOS_IMPL_FIND_FILE_INFO(FindTileInfoFromAtlasKey, FindTileDataFromAtlasKey, char const *, const)
#undef CHAOS_IMPL_FIND_FILE_INFO


			// ==========================================
			// Manager methods
			// ==========================================

#define CHAOS_IMPL_MANAGER_LOAD(function_name, find_function_name, return_type, func_params, call_args)\
return_type * Manager::function_name(func_params, bool store_object)\
{\
	return_type * result = find_function_name(path);\
	if (result != nullptr)\
		return result;\
	return Do##function_name(call_args, store_object);\
}

#define CHAOS_IMPL_MANAGER_LOAD_ALL(function_name, find_function_name, return_type)\
	CHAOS_IMPL_MANAGER_LOAD(function_name, find_function_name, return_type, FilePathParam const & path, path)\
	CHAOS_IMPL_MANAGER_LOAD(function_name, find_function_name, return_type, FilePathParam const & path BOOST_PP_COMMA() Buffer<char> buffer, path BOOST_PP_COMMA() buffer)\
	CHAOS_IMPL_MANAGER_LOAD(function_name, find_function_name, return_type, FilePathParam const & path BOOST_PP_COMMA() tinyxml2::XMLDocument const * doc, path BOOST_PP_COMMA() doc)\

			CHAOS_IMPL_MANAGER_LOAD_ALL(LoadMap, FindMap, Map)
			CHAOS_IMPL_MANAGER_LOAD_ALL(LoadTileSet, FindTileSet, TileSet)
			CHAOS_IMPL_MANAGER_LOAD_ALL(LoadObjectTypeSet, FindObjectTypeSet, ObjectTypeSet)

#undef CHAOS_IMPL_MANAGER_LOAD_ALL
#undef CHAOS_IMPL_MANAGER_LOAD

#define CHAOS_IMPL_MANAGER_FIND(funcname, return_type, member_name, constness)\
return_type constness * Manager::funcname(FilePathParam const & path) constness\
{\
	size_t count = member_name.size();\
	for (size_t i = 0; i < count; ++i)\
		if (member_name[i]->IsMatchingName(path.GetResolvedPath()))\
			return member_name[i].get();\
	return nullptr;\
}

			CHAOS_IMPL_MANAGER_FIND(FindMap, Map, maps, BOOST_PP_EMPTY())
			CHAOS_IMPL_MANAGER_FIND(FindMap, Map, maps, const)
			CHAOS_IMPL_MANAGER_FIND(FindTileSet, TileSet, tile_sets, BOOST_PP_EMPTY())
			CHAOS_IMPL_MANAGER_FIND(FindTileSet, TileSet, tile_sets, const)
			CHAOS_IMPL_MANAGER_FIND(FindObjectTypeSet, ObjectTypeSet, object_type_sets, BOOST_PP_EMPTY())
			CHAOS_IMPL_MANAGER_FIND(FindObjectTypeSet, ObjectTypeSet, object_type_sets, const)
#undef CHAOS_IMPL_MANAGER_FIND

#define CHAOS_IMPL_MANAGER_DOLOAD(funcname, return_type, member_name)\
return_type * Manager::funcname(FilePathParam const & path, bool store_object)\
{\
	Buffer<char> buffer = FileTools::LoadFile(path, true);\
	if (buffer != nullptr)\
		return funcname(path, buffer, store_object);\
	return nullptr;\
}\
return_type * Manager::funcname(FilePathParam const & path, Buffer<char> buffer, bool store_object)\
{\
	return_type * result = nullptr;\
	tinyxml2::XMLDocument * doc = new tinyxml2::XMLDocument();\
	if (doc != nullptr)\
	{\
		tinyxml2::XMLError error = doc->Parse(buffer.data, buffer.bufsize);\
		if (error == tinyxml2::XML_SUCCESS)\
			result = funcname(path, doc, store_object);\
		delete(doc);\
	}\
	return result;\
}\
return_type * Manager::funcname(FilePathParam const & path, tinyxml2::XMLDocument const * doc, bool store_object)\
{\
	assert(doc != nullptr);\
	return_type * result = new return_type(this, path.GetResolvedPath());\
	if (result != nullptr)\
	{\
		if (result->DoLoadDocument(doc))\
		{\
			if (store_object)\
				member_name.push_back(result);\
		}\
		else\
		{\
			delete(result);\
			result = nullptr;\
		}\
	}\
	return result;\
}

			CHAOS_IMPL_MANAGER_DOLOAD(DoLoadTileSet, TileSet, tile_sets)
			CHAOS_IMPL_MANAGER_DOLOAD(DoLoadObjectTypeSet, ObjectTypeSet, object_type_sets)
			CHAOS_IMPL_MANAGER_DOLOAD(DoLoadMap, Map, maps)

#undef CHAOS_IMPL_MANAGER_DOLOAD

		Property * Manager::FindObjectProperty(char const * type, char const * name, PropertyType type_id)
		{	
			if (StringTools::IsEmpty(type))
				return nullptr;
			size_t count = object_type_sets.size();
			for (size_t i = 0 ; i < count ; ++i)
			{
				ObjectTypeSet * ots = object_type_sets[i].get();
				if (ots == nullptr)
					return false;
				Property * result = ots->FindObjectProperty(type, name, type_id);
				if (result != nullptr)
					return result;		
			}
			return nullptr;
		}

		Property const * Manager::FindObjectProperty(char const * type, char const * name, PropertyType type_id) const
		{
			if (StringTools::IsEmpty(type))
				return nullptr;
			size_t count = object_type_sets.size();
			for (size_t i = 0 ; i < count ; ++i)
			{
				ObjectTypeSet * ots = object_type_sets[i].get();
				if (ots == nullptr)
					return false;
				Property * result = ots->FindObjectProperty(type, name, type_id);
				if (result != nullptr)
					return result;		
			}
			return nullptr;
		}

	};  // namespace TiledMap

}; // namespace chaos

#undef CHAOS_REVERSE_Y_AXIS