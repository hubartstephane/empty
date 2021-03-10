#ifdef CHAOS_FORWARD_DECLARATION


#else

namespace chaos
{
	namespace TMTools
	{
		/** serialize layers into JSON */
		template<typename T>
		void SerializeLayersFromJSON(T* object, nlohmann::json const& json)
		{
			nlohmann::json const* layers_json = JSONTools::GetStructure(json, "LAYERS");
			if (layers_json != nullptr && layers_json->is_array())
			{
				for (size_t i = 0; i < layers_json->size(); ++i)
				{
					nlohmann::json const* layer_json = JSONTools::GetStructureByIndex(*layers_json, i);
					if (layer_json != nullptr && layer_json->is_object())
					{
						int layer_id = 0;
						if (JSONTools::GetAttribute(*layer_json, "LAYER_ID", layer_id))
						{
							TMLayerInstance* layer_instance = object->FindLayerInstanceByID(layer_id);
							if (layer_instance != nullptr)
								LoadFromJSON(*layer_json, *layer_instance); // XXX : the indirection is important to avoid the creation of a new layer_instance
						}
					}
				}
			}
		}

		/** search a layer inside an object by ID */
		template<typename T, typename U>
		auto FindLayerInstanceByID(T* object, U& layer_instances, int in_id, bool recursive) -> decltype(layer_instances[0].get())
		{
			for (auto& layer : layer_instances)
			{
				if (layer != nullptr)
				{
					if (layer->GetLayerID() == in_id)
						return layer.get();
					if (recursive)
						if (auto result = layer->FindLayerInstanceByID(in_id, recursive))
							return result;
				}
			}
			return nullptr;
		}

		/** search a layer inside an object by request */
		template<typename T, typename U>
		auto FindLayerInstance(T* object, U& layer_instances, ObjectRequest request, bool recursive) -> decltype(layer_instances[0].get())
		{
			for (auto& layer : layer_instances)
			{
				if (layer != nullptr)
				{
					if (request.Match(*layer.get()))
						return layer.get();
					if (recursive)
						if (TMLayerInstance* result = layer->FindLayerInstance(request, recursive))
							return result;
				}
			}
			return nullptr;
		}

		/** fill BitmapAtlasInput from a TiledMap manager */
		bool AddIntoAtlasInput(TiledMap::Manager const * manager, BitmapAtlas::AtlasInput & input);
		/** fill BitmapAtlasInput from a TileSet */
		bool AddIntoAtlasInput(TiledMap::TileSet const * tile_set, BitmapAtlas::AtlasInput & input);
		/** fill BitmapAtlasInput from a Map */
		bool AddIntoAtlasInput(TiledMap::Map const * map, BitmapAtlas::AtlasInput & input);

		/** fill BitmapAtlasInput from a TiledMap manager */
		bool AddIntoAtlasInput(TiledMap::Manager const * manager, BitmapAtlas::FolderInfoInput * folder_input);
		/** fill BitmapAtlasInput from a TileSet */
		bool AddIntoAtlasInput(TiledMap::TileSet const * tile_set, BitmapAtlas::FolderInfoInput * folder_input);
		/** fill BitmapAtlasInput from a Map */
		bool AddIntoAtlasInput(TiledMap::Map const * map, BitmapAtlas::FolderInfoInput * folder_input);
		/** fill BitmapAtlasInput from a Layer */
		bool AddIntoAtlasInput(TiledMap::LayerBase const* layer, BitmapAtlas::FolderInfoInput* folder_input);

		/** returns true whether the object defines an explicit world bounding */
		bool IsWorldBoundingBox(TiledMap::TypedObject const* typed_object);
		/** returns true whether the object defines an explicit layer bounding */
		bool IsLayerBoundingBox(TiledMap::TypedObject const* typed_object);
		/** returns true whether the object is a player start */
		bool IsPlayerStart(TiledMap::TypedObject const* typed_object);
		/** returns true whether the object is a camera template */
		bool IsCameraTemplate(TiledMap::TypedObject const* typed_object);
		/** returns true whether the object is a finish trigger */
		bool IsFinishTrigger(TiledMap::TypedObject const* typed_object);
		/** returns true whether the object is a checpoint trigger */
		bool IsCheckpointTrigger(TiledMap::TypedObject const* typed_object);
		/** returns true whether the object is a notification trigger */
		bool IsNotificationTrigger(TiledMap::TypedObject const* typed_object);
		/** returns true whether the object is a sound */
		bool IsSoundTrigger(TiledMap::TypedObject const* typed_object);

	}; // namespace TMTools

}; // namespace chaos

#endif // CHAOS_FORWARD_DECLARATION