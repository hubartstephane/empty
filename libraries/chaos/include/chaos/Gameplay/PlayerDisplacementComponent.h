namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class PlayerDisplacementComponent;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	class CHAOS_API PlayerDisplacementComponent : public Tickable, public JSONSerializableInterface
	{
		CHAOS_GAMEPLAY_ALLFRIENDS;

		CHAOS_DECLARE_OBJECT_CLASS(PlayerDisplacementComponent, Tickable);

	public:

		CHAOS_DECLARE_GAMEPLAY_GETTERS();

		/** Initialization method */
		bool Initialize(Player* in_player);

		/** returns the player the component belongs to */
		AutoCastable<Player> GetPlayer() { return player; }
		/** returns the player the component belongs to */
		AutoConstCastable<Player> GetPlayer() const { return player; }

	protected:

		/** the player that owns this component */
		Player* player = nullptr;

	};

#endif

}; // namespace chaos