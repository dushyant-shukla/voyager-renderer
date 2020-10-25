#pragma once

#include <glm/glm.hpp>

#include "graphics/vulkan/CommandBuffers.h"
#include "graphics/vulkan/Buffer.h"
#include "CurveVertex.h"

namespace vr
{
	struct TableValue
	{
		float distance;
		float pointOnCurve;
		int curveIndex;

		TableValue(float d, float p, int i) : distance(d), pointOnCurve(p), curveIndex(i)
		{}
	};

	class Splines
	{
	public:

		Splines();
		~Splines();

		// draws the curve and control points
		void DrawSpline(VkCommandBuffer commandBuffer);
		CurveVertex CalculateBSpline(glm::mat4 matrix, float t);
		TableValue FindInTable(float distance, float& timer);
		void CalculateAdaptiveTable(float& t1, float& t2, float& t3);

	private:

		void CalculateCurve();

	private:

		//static inline CurveVertex P1 = CurveVertex(-45.0f, 10.0f, 20.0f);
		//static inline CurveVertex P2 = CurveVertex(-30.0f, 10.0f, -20.0f);
		//static inline CurveVertex P3 = CurveVertex(15.0f, 10.0f, 50.0f);
		//static inline CurveVertex P4 = CurveVertex(00.0f, 10.0f, 00.0f);
		//static inline CurveVertex P5 = CurveVertex(-15.0f, 10.0f, 40.0f);

		static inline CurveVertex P1 = CurveVertex(-0.5f, 0.5f, 0.0f);
		static inline CurveVertex P2 = CurveVertex(-0.3f, 0.5f, 0.0f);
		static inline CurveVertex P3 = CurveVertex(0.0f, 0.0f, 0.0f);
		static inline CurveVertex P4 = CurveVertex(0.0f, 0.5f, 00.0f);
		static inline CurveVertex P5 = CurveVertex(-0.7f, 0.5f, 0.0f);

		std::vector<CurveVertex> mControlPoints;
		std::vector<CurveVertex> mInterpolatedPoints;
		std::vector<glm::mat4> mControlPointsMatrices;
		std::vector<CurveVertex> points;

		struct {
			Buffer<CurveVertex> controPoints;
			Buffer<CurveVertex> interpolatedPoints;
		} mBuffers;

		std::vector<TableValue> mArcTable;
	};
}
