#pragma once

#include <string>

namespace vr
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(double ts) {}
		virtual void OnImGuiRender() {}

		inline const std::string GetDebugName() { return mDebugName; }

	protected:
		std::string mDebugName;
	};
}
