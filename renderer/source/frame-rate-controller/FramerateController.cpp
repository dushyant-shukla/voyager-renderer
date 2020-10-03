#include "FramerateController.h"
#include "logging/LoggingCore.h"

namespace vr
{
	/*
	FramerateController will always have only one instance throughout the execution of the application.
	*/
	FramerateController* FramerateController::GetInstance(unsigned int maxFrameRate)
	{
		static FramerateController frameRateController(maxFrameRate); // initialized the first time GetInstance() is called.
		return &frameRateController;
	}

	FramerateController::FramerateController(unsigned int maxFrameRate) : mMaxFramerate(maxFrameRate), mFrameTime(0.0)
	{
		if (0 >= mMaxFramerate) { mMaxFramerate = 60; }
		RENDERER_DEBUG("MAX FRAME RATE SET TO: {0} FPS", mMaxFramerate);

		mTicksPerFrame = 1000.0 / mMaxFramerate;

		mClock.Reset();
	}

	FramerateController::~FramerateController()
	{
	}

	void FramerateController::FrameStart()
	{
		mClock.Reset();
	}

	void FramerateController::FrameEnd()
	{
		double elapsedTime = 0.0;
		do
		{
			elapsedTime = mClock.Mark();
		} while (elapsedTime < mTicksPerFrame);

		mFrameTime = elapsedTime / 1000.0; // converting milliseconds to seconds

		//mClock.Reset();
	}

	const double& FramerateController::GetFrameTime() const
	{
		return mFrameTime;
	}
}