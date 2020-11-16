#ifdef CHAOS_FORWARD_DECLARATION

namespace chaos
{
	class WeakPointerData;
	class SharedPointerPolicy;
	class WeakPointerPolicy;

	template<typename T, typename POLICY>
	class SmartPointerBase;

	template<typename T>
	using weak_ptr = SmartPointerBase<T, WeakPointerPolicy>;
	template<typename T>
	using shared_ptr = SmartPointerBase<T, SharedPointerPolicy>;

	template<typename T>
	struct is_weak_ptr;

}; // namespace chaos

#else 

namespace chaos
{


	// XXX : -shared_ptr<> is more or less a clone of boost::intrusive_ptr<>. It can be used on any class as soon as there are the 2 following functions
	//        	 
	//           intrusive_ptr_add_ref(...) and intrusive_ptr_release(...)
	//
	//       -weak_ptr<> use an intermediate structure that points on the target object
	//
	//        weak_ptr 
	//            |
	//            +-----> WeakPointerData <-----+
	//                          |               |
	//                          +----> target --+
	//
	//         to work the target must have a pointer on the WeakPointerData so it can reset it when it is destroyed
	//
	//         As a side effect, accessing target from a weak_ptr requires an additionnal indirection (slower)
	//                           using weak_ptr cause an allocation (for the very first only)
	//
	// XXX : -shared_ptr<T const> make no sense !!! (while the shared_ptr is responsible for th death of the object)
	//
	//       -weak_ptr<T const>   can be used

	/**
	 * WeakPointerData : an utility structure used to handle weak pointers
	 */

	class WeakPointerData
	{
	public:

		/** constructor */
		WeakPointerData(void const* ptr) : // XXX : const here because a weak_ptr<T const> make sense
			object_ptr(ptr)
		{
			assert(ptr != nullptr);
		}

		/** pointer on the object */
		void const * object_ptr = nullptr;
		/** count weak reference */
		boost::atomic<int> weak_count;
	};

	/**
	 * SharedPointerPolicy : policy used for shared_ptr
	 */

	class SharedPointerPolicy
	{
	public:

		/** the internal data in the smart pointer */
		template<typename T>
		using pointer_type = T;

		/** adding a reference */
		template<typename T>
		static pointer_type<T> * AddReference(T* in_target)
		{
			assert(in_target != nullptr);
			intrusive_ptr_add_ref(in_target);
			return in_target;
		}

		/** removing a reference */
		template<typename T>
		static pointer_type<T> * SubReference(pointer_type<T> * in_target)
		{
			assert(in_target != nullptr);
			intrusive_ptr_release(in_target);
			return nullptr;
		}

		/** get the resource pointed */
		template<typename T>
		static T * Get(T * in_target)
		{
			return in_target;
		}
	};


	/**
	 * SharedPointerPolicy : policy used for weak_ptr
	 */

	class WeakPointerPolicy
	{
	public:

		/** the internal data in the smart pointer */
		template<typename T>
		using pointer_type = WeakPointerData;


		/** adding a reference */
		template<typename T>
		static WeakPointerData * AddReference(T* in_target)
		{
			assert(in_target != nullptr);

			if (in_target->weak_ptr_data == nullptr) // reserve a weak structure if needed
			{
				in_target->weak_ptr_data = new WeakPointerData(in_target);
				if (in_target->weak_ptr_data == nullptr)
					return nullptr;
			}
			++in_target->weak_ptr_data->weak_count;
			return in_target->weak_ptr_data;
		}

		/** removing a reference */
		static WeakPointerData * SubReference(WeakPointerData* in_target)
		{
			--in_target->weak_count;
			return nullptr;
		}

		/** get the resource pointed */
		template<typename T>
		static T * Get(WeakPointerData* in_target)
		{
			if (in_target != nullptr)
				return (T*)in_target->object_ptr; // remove constness here
			return nullptr;
		}

	};

	/**
	 * SmartPointerBase : implementation of both shared_ptr and weak_ptr
	 */

	template<typename T, typename POLICY>
	class SmartPointerBase
	{
		friend class SharedPointerPolicy;
		friend class WeakPointerPolicy;

	public:

		/** the type of object pointed */
		using type = T;
		
		/** default constructor */
		SmartPointerBase() {}
		/** constructor with capturing the object */
		SmartPointerBase(type * in_target)
		{
			if (in_target != nullptr)
				target = POLICY::AddReference(in_target);
		}
		/** copy constructor */
		SmartPointerBase(SmartPointerBase<T, POLICY> const & src) :
			SmartPointerBase(src.get())
		{
		}

		/** move constructor */
		SmartPointerBase(SmartPointerBase<T, POLICY>&& src) noexcept: // XXX : the noexcept is required to have move semantic used during vector resized
			target(src.target)
		{
			src.target = nullptr; // capture the reference
		}

		/** destructor */
		~SmartPointerBase()
		{
			if (target != nullptr)
				target = POLICY::SubReference(target);
		}

		/** copy */
		SmartPointerBase & operator = (type * src)
		{
			if (src != get())
				DoSetTarget(src);
			return *this;
		}
		/** copy */
		SmartPointerBase & operator = (SmartPointerBase<T, POLICY> const & src)
		{
			if (src.target != target) // no need to fully get() => small optimization for weak_ptr
				DoSetTarget(src.get());
			return *this;
		}

		/** move */
		SmartPointerBase & operator = (SmartPointerBase<T, POLICY>&& src) noexcept // shuxxx to test
		{
			if (src.target != target)
			{
				// XXX : add reference before destroying to be sure that by chain reaction we do not destroy the object we want to point first
				auto old_target = target;

				// 1 - capture new the reference
				target = src.target;
				src.target = nullptr;
				// 2 - remove old reference
				if (old_target != nullptr)
					POLICY::SubReference(old_target);
			}
			return *this;
		}

		/** getters */
		type * get() const
		{
			return POLICY::Get<type>(target);
		}

		/** getters */
		type * operator ->() const
		{
			type * result = get();
			assert(result != nullptr);
			return result;
		}

		/** getters */
		type & operator * () const
		{
			type * result = get();
			assert(result != nullptr);
			return *result;
		}

		/** swap pointers */
		friend void swap(SmartPointerBase& src1, SmartPointerBase& src2)
		{
			std::swap(src1.target, src2.target); // this works for shared_ptr AND weak_ptr
		}

	protected:

		/** internal method to change the content of the pointer */
		void DoSetTarget(type * src)
		{
			// XXX : add reference before destroying to be sure that by chain reaction we do not destroy the object we want to point first
			auto old_target = target;

			// 1 - add reference 
			if (src != nullptr)
				target = POLICY::AddReference(src);
			else
				target = nullptr;
			// 2 - remove reference
			if (old_target != nullptr)
				POLICY::SubReference(old_target);
		}

	protected:

		/** the object pointed (or a WeakPointerData structure) */
		typename POLICY::template pointer_type<type> * target = nullptr;
	};

	/**
	* SmartPointerBase / raw pointer comparaisons
	*/

	template<typename T, typename POLICY, typename U>
	bool operator == (SmartPointerBase<T, POLICY> const & src1, U * src2)
	{
		return (src1.get() == src2);
	}

	template<typename T, typename POLICY, typename U>
	bool operator != (SmartPointerBase<T, POLICY> const & src1, U * src2)
	{
		return (src1.get() != src2);
	}

	template<typename T, typename POLICY, typename U>
	bool operator == (U * src1, SmartPointerBase<T, POLICY> const & src2)
	{
		return (src1 == src2.get());
	}

	template<typename T, typename POLICY, typename U>
	bool operator != (U * src1, SmartPointerBase<T, POLICY> const & src2)
	{
		return (src1 != src2.get());
	}

	/**
	* SmartPointerBase / nullptr comparaisons
	*/

	template<typename T, typename POLICY>
	bool operator == (SmartPointerBase<T, POLICY> const & src1, nullptr_t src2)
	{
		return (src1.get() == src2);
	}

	template<typename T, typename POLICY>
	bool operator != (SmartPointerBase<T, POLICY> const & src1, nullptr_t src2)
	{
		return (src1.get() != src2);
	}

	template<typename T, typename POLICY>
	bool operator == (nullptr_t src1, SmartPointerBase<T, POLICY> const & src2)
	{
		return (src1 == src2.get());
	}

	template<typename T, typename POLICY>
	bool operator != (nullptr_t src1, SmartPointerBase<T, POLICY> const & src2)
	{
		return (src1 != src2.get());
	}

	/**
	* SmartPointerBase / SmartPointerBase comparaisons
	*/

	template<typename T, typename POLICY, typename U>
	bool operator == (SmartPointerBase<T, POLICY> const & src1, SmartPointerBase<U, POLICY> const & src2)
	{
		return (src1.get() == src2.get());
	}

	template<typename T, typename POLICY, typename U>
	bool operator != (SmartPointerBase<T, POLICY> const & src1, SmartPointerBase<U, POLICY> const & src2)
	{
		return (src1.get() != src2.get());
	}

	/**
	* swap
	*/

	template<typename T, typename POLICY> 
	void swap(SmartPointerBase<T, POLICY> & src1, SmartPointerBase<T, POLICY> & src2)
	{
		src1.swap(src2);
	}

	/**
	* checking whether a data is some kind of weak pointer
	*/

	template<typename T>
	struct is_weak_ptr
	{
		static constexpr bool value = false;
	};

	template<typename T>
	struct is_weak_ptr<weak_ptr<T>>
	{
		static constexpr bool value = true;
	};

	template<typename T>
	constexpr bool is_weak_ptr_v = is_weak_ptr<T>::value;

	/**
	* checking whether a data is some kind of shared pointer
	*/

	template<typename T>
	struct is_shared_ptr
	{
		static constexpr bool value = false;
	};

	template<typename T>
	struct is_shared_ptr<shared_ptr<T>>
	{
		static constexpr bool value = true;
	};

	template<typename T>
	constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

}; // namespace chaos

#endif // CHAOS_FORWARD_DECLARATION


