#pragma once

#include "clock/Clock.h"

namespace vr
{
	/*
	*	Basic usage:

		FramerateController::FrameStart();

		// do work { ... }

		FramerateController::FrameEnd();

		Then use FramerateController::GetFrameTime() to retrieve frame time.
	*/
	class FramerateController
	{
	public:

		static FramerateController* GetInstance(unsigned int maxFrameRate);
		~FramerateController();

		void FrameStart();
		void FrameEnd();

		/*
			Returns the frame-time in seconds
		*/
		const double& GetFrameTime() const;

		static inline bool mControlFramerate = true;

	private:

		FramerateController(unsigned int maxFrameRate);

	private:

		unsigned int mMaxFramerate;
		double mTicksPerFrame;
		double mFrameTime;
		Clock<std::milli> mClock;
	};
}
