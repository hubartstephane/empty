#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/SmartPointers.h>

namespace chaos
{

	/**
	* ReferencedObject is a base class that have a reference count (shared and weak)
	*/

	class ReferencedObject
	{
		friend class SharedPointerPolicy;
		friend class WeakPointerPolicy;

	public:

		/** constructor */
		ReferencedObject();

		/** destructor */
		virtual ~ReferencedObject() = default;

	public:

		/** adding a shared reference */
		virtual void AddReference(SharedPointerPolicy policy);
		/** adding a weak reference */
		virtual void AddReference(WeakPointerPolicy policy);
		/** removing a shared reference */
		virtual void SubReference(SharedPointerPolicy policy);
		/** removing a weak reference */
		virtual void SubReference(WeakPointerPolicy policy);

	protected:

		/** called whenever there are no more reference on the object */
		virtual void OnLastReferenceLost();

	protected:

		/** count shared reference */
		boost::atomic<int> shared_count;
		/** count weak reference */
		boost::atomic<int> weak_count;
		/** whenever the content has been destroyed */
		bool shared_destroyed = false;
	};

	/**
	* DisableReferenceCount : an utility class to help using referenced object on stack
	*/

	template<typename T>
	class DisableReferenceCount : public T
	{
	public:

		/** forwarding constructor */
		using T::T;

	protected:

		/** disable all reference count functions */
		virtual void AddReference(WeakPointerPolicy policy) override { }
		virtual void AddReference(SharedPointerPolicy policy) override { }
		virtual void SubReference(WeakPointerPolicy policy) override { }
		virtual void SubReference(SharedPointerPolicy policy) override { }		
		virtual void OnLastReferenceLost() override { }
	};

	/**
	* ReferencedObjectDataWrapper : a data wrapped into a referenced object => while referenced object may be dynamic_casted we can test for the data inside
	*/

	template<typename T>
	class ReferencedObjectDataWrapper : public DisableReferenceCount<ReferencedObject>
	{
		using type = T;

	public:

		/** copy constructor */
		ReferencedObjectDataWrapper(ReferencedObjectDataWrapper const & src) : data(src.data){}

		/** constructor */
		template<typename ...PARAMS>
		ReferencedObjectDataWrapper(PARAMS... params) : data(params...){}

	public:

		/** the wrapped data */
		T data;
	};

	template<typename T, typename ...PARAMS>
	auto MakeReferencedObjectWrapper(PARAMS... params)
	{
		return ReferencedObjectDataWrapper<T>(params...);
	}

}; // namespace chaos

	 /**
	 * ReferencedObject : reference count external methods
	 *
	 * XXX : theses functions are out of chaos scope, else while shared_ptr is in chaos, it searches for chaos::intrusive_ptr_add function in prioriy
	 *       and if it was finding ReferencedObject reference functions inside chaos scope, it will fail with IrrklangTools functions
	 */

	 /** utility method for shared_ptr */
template<typename POLICY = chaos::SharedPointerPolicy>
void intrusive_ptr_add_ref(chaos::ReferencedObject * obj, POLICY policy = POLICY()) // to work with boost::intrusive_ptr<>
{
	obj->AddReference(policy);
}

/** utility method for shared_ptr */
template<typename POLICY = chaos::SharedPointerPolicy>
void intrusive_ptr_release(chaos::ReferencedObject * obj, POLICY policy = POLICY()) // to work with boost::intrusive_ptr<>
{
	obj->SubReference(policy);
}

