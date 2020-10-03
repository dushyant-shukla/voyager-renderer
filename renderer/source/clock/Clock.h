#pragma once

#include <chrono>

namespace vr
{
	/*
		A simple timer class.
		Call Reset() followed by Mark() to get elapsed time in T.
		Where T = unit of time, std::milli for example.
	*/
	template <typename T>
	class Clock
	{
	public:

		Clock()
		{
			Reset();
		}

		/*
			Records current point in time.
		*/
		void Reset()
		{
			mLastRecordedTimePoint = std::chrono::high_resolution_clock::now();
		}

		/*
			Returns time elapsed since the last time reset was called in T.
		*/
		double Mark()
		{
			std::chrono::high_resolution_clock::time_point currentTimePoint = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, T> elapsedTime = currentTimePoint - mLastRecordedTimePoint;
			return elapsedTime.count();
		}

	private:

		std::chrono::high_resolution_clock::time_point mLastRecordedTimePoint;
	};
}