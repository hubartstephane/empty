#pragma once

namespace chaos
{


	template<typename T>
	class AutoConstCastable
	{
	public:

		/** the constructors */
		AutoConstCastable() = default;
		AutoConstCastable(AutoConstCastable const& src) = default;
		AutoConstCastable(T* in_ptr) : ptr(in_ptr) {}
		AutoConstCastable(T const* in_ptr) : ptr(in_ptr) {}
		/** the conversion operator */
		template<typename U>
		operator U const* () const
		{
			if constexpr (std::is_base_of_v<U, T>)
				return ptr;
			return dynamic_cast<const U*>(ptr);
		}

		/** indirection operator */
		T const* operator -> () const { return ptr; }

	protected:

		/** the pointer */
		T const* ptr = nullptr;
	};

	template<typename T>
	class AutoCastable
	{
	public:

		/** the constructors */
		AutoCastable() = default;
		AutoCastable(AutoCastable const& src) = default;
		AutoCastable(T* in_ptr) : ptr(in_ptr) {}
		/** the const conversion operator */
		operator AutoConstCastable<T>() const
		{
			return AutoConstCastable<T>(ptr);
		}
		/** the conversion operator */
		template<typename U>
		operator U* () const
		{
			if constexpr (std::is_base_of_v<U, T>)
				return ptr;
			return dynamic_cast<U*>(ptr);
		}
		/** indirection operator */
		T * operator -> () const { return ptr; }

	protected:

		/** the pointer */
		T* ptr = nullptr;
	};

	/** create a delayed dynamic_cast<> */
	template<typename T>
	AutoCastable<T> auto_cast(T * ptr) { return AutoCastable<T>(ptr); }
	/** create a delayed dynamic_cast<> */
	template<typename T>
	AutoConstCastable<T> auto_cast(T const * ptr) { return AutoConstCastable<T>(ptr); }

}; // chaos
