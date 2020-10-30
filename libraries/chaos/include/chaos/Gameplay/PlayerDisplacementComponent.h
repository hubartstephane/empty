#ifdef CHAOS_FORWARD_DECLARATION

namespace chaos
{
	class PlayerDisplacementComponent;

}; // namespace chaos

#else

namespace chaos
{
	class PlayerDisplacementComponent : public Tickable
	{
		CHAOS_GAMEPLAY_ALLFRIENDS;

		CHAOS_DECLARE_OBJECT_CLASS2(PlayerDisplacementComponent, Tickable);

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

}; // namespace chaos

#endif // CHAOS_FORWARD_DECLARATION