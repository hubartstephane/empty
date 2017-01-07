#include <chaos/ClockManager.h>
#include <chaos/MathTools.h>

namespace chaos
{
	// ============================================================
	// ClockEventInfo functions
	// ============================================================

	double ClockEventInfo::GetMaxEventTime() const
	{
		if (IsForeverEvent())
			return std::numeric_limits<double>::max();
		if (IsRepeatedInfinitly())
			return std::numeric_limits<double>::max();
		return start_time + duration + (duration + repetition_delay) * (double)repetition_count;
	}

	ClockEventTickData ClockEventInfo::GetExecutionInfo(double t1, double t2) const
	{
		ClockEventTickData result;
		result.time_slice.first = t1;
		result.time_slice.second = t2;
		result.execution_range.first = std::numeric_limits<double>::max();
		result.execution_range.second = std::numeric_limits<double>::max();
		result.tick_range.first = std::numeric_limits<double>::max();
		result.tick_range.second = std::numeric_limits<double>::max();

		// event to come later
		if (start_time > t2)
			return result;

		// event with unknown duration
		if (IsForeverEvent())
		{
			result.execution_range.first = start_time;
			result.execution_range.second = std::numeric_limits<double>::max();
			result.tick_range.first = max(start_time, t1);
			result.tick_range.second = t2;
		}
		// event without REPETITION
		else if (!IsRepeated())
		{
			result.execution_range.first = start_time;
			result.execution_range.second = start_time + duration;
			result.tick_range.first = max(start_time, t1);
			result.tick_range.second = min(start_time + duration, t2);
		}
		// event with limited duration (0 or finite) + REPETITION
		else
		{
			// we want to test start_time + K.repetition against 't1'
			//
			// start_time + K.(duration + repetition_delay) >= t1

			double tmp = (t1 - start_time) / (duration + repetition_delay);

			double k1 = MathTools::Floor(tmp);
			double s1 = start_time + k1 * (duration + repetition_delay);

			if (s1 <= t2 && s1 + duration >= t1)
			{
				result.execution_range.first = s1;
				result.execution_range.second = s1 + duration;
				result.tick_range.first = max(s1, t1);
				result.tick_range.second = min(s1 + duration, t2);
			}
			else
			{
				double k2 = MathTools::Ceil(tmp);
				double s2 = start_time + k2 * (duration + repetition_delay);
				if (s2 <= t2 && s2 + duration >= t1)
				{
					result.execution_range.first = s2;
					result.execution_range.second = s2 + duration;
					result.tick_range.first = max(s2, t1);
					result.tick_range.second = min(s2 + duration, t2);
				}
			}
		}
		return result;
	}

	bool ClockEventInfo::IsTooLateFor(double current_time) const
	{
		double max_event_time = GetMaxEventTime();
		if (max_event_time < current_time)
			return true;
		return false;
	}

	// ============================================================
	// Clock functions
	// ============================================================

	Clock::Clock(ClockCreateParams const & params) :
		time_scale(params.time_scale),
		paused(params.paused)
	{

	}

	Clock::~Clock()
	{
		// after destructor, the children vector will be cleared
		// the problem is that children may survive to their parent death due to reference count
		// so orphan all children to ensure they cannot access parent's anymore
		size_t child_count = children_clocks.size();
		for (size_t i = 0; i < child_count; ++i)
			children_clocks[i]->parent_clock = nullptr;
		// same thing with events
		size_t event_count = pending_events.size();
		for (size_t i = 0; i < event_count; ++i)
			pending_events[i]->clock = nullptr;
	}

	bool Clock::IsDescendantClock(Clock const * child_clock) const
	{
		assert(child_clock != nullptr);

		// consider that a clock is it own child !!
		if (child_clock == this)
			return true;
		// recurse
		size_t count = children_clocks.size();
		for (size_t i = 0; i < count; ++i)
			if (children_clocks[i]->IsDescendantClock(child_clock))
				return true;
		return false;
	}

	Clock * Clock::GetChildClock(int id, bool recursive)
	{
		Clock * result = nullptr;
		if (id > 0)  // no clock with negative ID !
		{
			if (clock_id == id)
				result = this;
			else if (recursive)
			{
				size_t count = children_clocks.size();
				for (size_t i = 0; (i < count) && (result == nullptr); ++i)
					result = children_clocks[i]->GetChildClock(id, recursive);
			}
		}
		return result;
	}

	Clock const * Clock::GetChildClock(int id, bool recursive) const
	{
		Clock const * result = nullptr;
		if (id > 0)  // no clock with negative ID !
		{
			if (clock_id == id)
				result = this;
			else if (recursive)
			{
				size_t count = children_clocks.size();
				for (size_t i = 0; (i < count) && (result == nullptr); ++i)
					result = children_clocks[i]->GetChildClock(id, recursive);
			}
		}
		return result;
	}

	Clock * Clock::GetTopLevelParent()
	{
		Clock * result = this;
		while (result->parent_clock != nullptr)
			result = result->parent_clock;
		return result;
	}

	Clock const * Clock::GetTopLevelParent() const
	{
		Clock const * result = this;
		while (result->parent_clock != nullptr)
			result = result->parent_clock;
		return result;
	}

	void Clock::TriggerClockEvent(ClockEventTickRegistration & registered_event)
	{
		// degenerated use case :
		//   the processing of a previous event remove another event from execution
		boost::intrusive_ptr<ClockEvent> clock_event = registered_event.clock_event;
		if (clock_event == nullptr)
			return;

		// ensure the event is still in range


		// compute the tick information & tick
		ClockEventTickData tick_data;
		tick_data.time_slice = registered_event.time_slice;
		tick_data.execution_range = registered_event.execution_range;
		tick_data.tick_range = registered_event.tick_range;

		ClockEventTickResult tick_result = clock_event->Tick(tick_data);
		// handle the result
		ClockEventInfo & event_info = clock_event->GetEventInfo();

		// current execution is completed if :
		//   - explicitely required
		//   - time is over
		bool execution_completed = tick_result.IsExecutionCompleted(); // explicit request
		if (!execution_completed)
			if (tick_data.time_slice.second >= tick_data.execution_range.second) // time is over
				execution_completed = true;

		if (execution_completed) // current execution is beeing ended : can start a repetition ?
		{
			if (!tick_result.CanRepeatExecution() || !event_info.IsRepeated()) // want to stop all executions
			{
				clock_event->RemoveFromClock();
			}
			else
			{
				// still repetition possible ?
				if (!event_info.IsRepeatedInfinitly())
				{
					if (event_info.repetition_count == 0) // no more repetition 
					{
						clock_event->RemoveFromClock();
						return;
					}
					else
						event_info.repetition_count--;
				}
				// do the repetition
				event_info.start_time =
					min(tick_data.tick_range.second, tick_data.time_slice.second) +
					event_info.repetition_delay;

				clock_event->tick_count = 0;
				clock_event->execution_count++;
			}
		}
		else
		{
			clock_event->tick_count++; // increment internal counter
		}
	}

	bool Clock::TickClock(double delta_time) // should only be called on TopLevel node (public interface)
	{
		assert(parent_clock == nullptr);

		ClockEventTickSet event_tick_set;

		// updates the clocks and collect the events (sorted)
		bool result = TickClockImpl(delta_time, 1.0, event_tick_set);
		// tick the events
		while (event_tick_set.size() > 0)
		{
			auto it = event_tick_set.begin();

			ClockEventTickRegistration registered_event = *it;
			event_tick_set.erase(it);
			TriggerClockEvent(registered_event);
		}
		return result;
	}

	bool Clock::TickClockImpl(double delta_time, double cumulated_factor, ClockEventTickSet & event_tick_set) // protected interface
	{
		// internal tick
		if (paused || time_scale == 0.0)
			return false;
		// register all events in the list
		double time1 = clock_time;
		double time2 = clock_time + time_scale * delta_time;

		clock_time = time2;

		if (tick_events)
		{
			for (size_t i = 0; i < pending_events.size(); ++i)
			{
				ClockEventInfo const & event_info = pending_events[i]->GetEventInfo();

				if (event_info.IsTooLateFor(time1))
				{
					boost::intrusive_ptr<ClockEvent> clock_event = pending_events[i]; // XXX : important to keep a reference after RemoveFromClock(...)
					clock_event->RemoveFromClock(); // XXX : we know RemoveFromClock = "RemoveReplace" so --i
					--i;
				}
				else
				{
					ClockEventTickData execution_info = event_info.GetExecutionInfo(time1, time2);
					if (execution_info.IsValid())
					{
						ClockEventTickRegistration registration;
						registration.clock_event = pending_events[i];
						registration.time_slice = execution_info.time_slice;
						registration.execution_range = execution_info.execution_range;
						registration.tick_range = execution_info.tick_range;

						if (registration.tick_range.first <= time1)
							registration.abs_time_to_start = 0.0;
						else
							registration.abs_time_to_start = (registration.tick_range.first - time1) * cumulated_factor;

						event_tick_set.insert(registration);
					}
				}
			}
		}
		// recursive tick 
		size_t child_count = children_clocks.size();
		for (size_t i = 0; i < child_count; ++i)
			children_clocks[i]->TickClockImpl(time_scale * delta_time, cumulated_factor * time_scale, event_tick_set);

		return true;
	}

	Clock * Clock::CreateChildClock(int id, ClockCreateParams const & params)
	{
		// want an ID : automatic or user-defined
		if (id != 0)
		{
			Clock * top_level_clock = GetTopLevelParent();
			// user-defined
			if (id > 0 && top_level_clock->GetChildClock(id, false) != nullptr) // cannot add a clock whose ID is already in use
				return nullptr;
			// generate a own ID if necessary
			else
			{
				assert(id < 0);
				id = top_level_clock->FindUnusedID(true);
				if (id < 0)
					return nullptr;
			}
		}

		// allocate the clock
		boost::intrusive_ptr<Clock> child_clock = boost::intrusive_ptr<Clock>(new Clock(params));
		if (clock != nullptr)
		{
			child_clock->clock_id = id;
			child_clock->parent_clock = this;
			children_clocks.push_back(child_clock);
		}
		return child_clock.get();
	}

	bool Clock::RemoveFromParent()
	{
		Clock * tmp = parent_clock;
		if (tmp != nullptr)
		{
			size_t count = tmp->children_clocks.size();
			for (size_t i = 0; i < count; ++i)
			{
				if (tmp->children_clocks[i].get() == this) // remove swap
				{
					if (i != count - 1) // nothing to do if already last element
						std::swap(tmp->children_clocks[i], tmp->children_clocks.back());

					parent_clock = nullptr;          // XXX : we cannot invert these 2 lines because, because 'this' could be deleted and then
					tmp->children_clocks.pop_back(); //       and then we would access 'parent_clock' member after destructor
					return true;                     //       that's why we are using 'tmp'					
				}
			}
		}
		return false;
	}

	void Clock::FindUnusedIDStep1(int & smaller_id, int & bigger_id, bool recursive) const // iteration over MIN & MAX ID's
	{
		size_t count = children_clocks.size();
		for (size_t i = 0; i < count; ++i)
		{
			if (smaller_id == 1 && bigger_id == std::numeric_limits<int>::max()) // no need to continue STEP1 recursion, we can say that there
				return;                                                            // will not be a fast answer, have to go STEP 2

			Clock const * clock = children_clocks[i].get(); // ignore clock with "NO" id
			if (clock->clock_id > 0)
			{
				smaller_id = min(smaller_id, clock->clock_id);
				bigger_id = max(bigger_id, clock->clock_id);
			}
			if (recursive)
			{
				clock->FindUnusedIDStep1(smaller_id, bigger_id, recursive);
			}
		}
	}

	void Clock::FindUnusedIDStep2(std::vector<int> & IDs, bool recursive) const
	{
		size_t count = children_clocks.size();
		for (size_t i = 0; i < count; ++i)
		{
			Clock const * clock = children_clocks[i].get();
			if (clock->clock_id > 0)
				IDs.push_back(clock->clock_id); // store only 'REAL' id's
			if (recursive)
				clock->FindUnusedIDStep2(IDs, recursive);
		}
	}

	int Clock::FindUnusedID(bool recursive) const
	{
		// STEP 1 : fast answer, with no vector usage
		//          search bigger and smaller ids (no memory allocation)
		//          if max and min values found are not MAX/MIN INTEGER values, just use increment/decrement values
		int smaller_id = (clock_id > 0) ? clock_id : std::numeric_limits<int>::max();
		int bigger_id = (clock_id > 0) ? clock_id : 0;

		FindUnusedIDStep1(smaller_id, bigger_id, recursive);
		if (bigger_id < std::numeric_limits<int>::max())   // XXX : try BIGGER first because ID will increment : 1, 2, 3 ..
			return bigger_id + 1;                            // if you try SMALLER first , you'll get MAX - 1 (that will later block BIGGER)
		if (smaller_id > 1)
			return smaller_id - 1;

		// STEP 2 : store all ID's in a vector
		//          sort the vector
		//          detect a 'hole' inside the vector an use it
		//          => require memory allocation
		std::vector<int> IDs;
		if (clock_id > 0)
			IDs.push_back(clock_id);
		FindUnusedIDStep2(IDs, recursive); // store all ID's
		std::sort(IDs.begin(), IDs.end());

		size_t count = IDs.size();
		for (size_t i = 0; i < count - 1; ++i)
			if (IDs[i] + 1 < IDs[i + 1]) // search for holes in the ID list
				return IDs[i] + 1;

		return -1;
	}


	bool Clock::AddPendingEvent(ClockEvent * clock_event, ClockEventInfo event_info, bool relative_time)
	{
		assert(clock_event != nullptr);

		// event already registered ?
		if (clock_event->clock != nullptr)
			return false;
		// make the event_info, relative to current_time if necessary
		if (relative_time)
			event_info.start_time += clock_time;
		// is it too late for that event ?
		if (event_info.IsTooLateFor(clock_time))
			return false;
		// do the registration
		clock_event->clock = this;
		clock_event->event_info = event_info;
		clock_event->tick_count = 0;
		clock_event->execution_count = 0;
		pending_events.push_back(clock_event);

		return true;
	}

	void Clock::RemoveAllChildClocks()
	{
		while (children_clocks.size() > 0)
		{
			boost::intrusive_ptr<Clock> child_clock = children_clocks[children_clocks.size() - 1];
			child_clock->RemoveFromParent();
		}
	}

	void Clock::RemoveAllPendingEvents()
	{
		while (pending_events.size() > 0)
		{
			boost::intrusive_ptr<ClockEvent> clock_event = pending_events[pending_events.size() - 1];
			clock_event->RemoveFromClock();
		}
	}

	bool ClockEvent::RemoveFromClock()
	{
		Clock * tmp = clock; // keep a trace of parent
		if (tmp != nullptr)
		{
			size_t count = tmp->pending_events.size();
			for (size_t i = 0; i < count; ++i)
			{
				if (tmp->pending_events[i].get() == this)
				{
					AddReference(); // because, we want to pop back the event, then call OnEventRemovedFromClock(...)

					if (i != count - 1)
						std::swap(tmp->pending_events[i], tmp->pending_events.back());
					clock = nullptr;
					tmp->pending_events.pop_back();

					OnEventRemovedFromClock();
					SubReference();
					return true;
				}
			}
		}
		return false;
	}
	



}; // namespace chaos
