﻿#include <chaos/Tickable.h>

namespace chaos
{
	// ========================================================
	// Tickable implementation
	// ========================================================	
	
	void Tickable::SetPause(bool in_pause)
	{
		if (paused == in_pause)
			return;
		paused = in_pause;
		OnPauseStateChanged(in_pause);
	}
		
	bool Tickable::IsPaused() const
	{
		return paused;
	}

	bool Tickable::Tick(double delta_time)
	{
		if (IsPaused())
			return false;
		return DoTick(delta_time);
	}

	bool Tickable::DoTick(double delta_time)
	{
		return true;
	}

	void Tickable::OnPauseStateChanged(bool in_pause)
	{

	}

}; // namespace chaos
