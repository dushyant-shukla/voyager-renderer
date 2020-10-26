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
		void DrawControlPoints(VkCommandBuffer commandBuffer);
		CurveVertex CalculateBSpline(glm::mat4 matrix, float t);
		CurveVertex CalculateBSplineDerivative(glm::mat4 matrix, float t);
		TableValue FindInTable(float distance);
		void CalculateAdaptiveTable(float& t1, float& t2, float& t3);

	private:

		void CalculateCurve();

	public:

		//static inline CurveVertex P1 = CurveVertex(-45.0f, 10.0f, 20.0f);
		//static inline CurveVertex P2 = CurveVertex(-30.0f, 10.0f, -20.0f);
		//static inline CurveVertex P3 = CurveVertex(15.0f, 10.0f, 50.0f);
		//static inline CurveVertex P4 = CurveVertex(00.0f, 10.0f, 00.0f);
		//static inline CurveVertex P5 = CurveVertex(-15.0f, 10.0f, 40.0f);

		//static inline CurveVertex P1 = CurveVertex(-0.5f, 0.5f, 0.0f);
		//static inline CurveVertex P2 = CurveVertex(-0.3f, 0.5f, 0.0f);
		//static inline CurveVertex P3 = CurveVertex(0.0f, 0.0f, 0.0f);
		//static inline CurveVertex P4 = CurveVertex(0.0f, 0.5f, 00.0f);
		//static inline CurveVertex P5 = CurveVertex(-0.7f, 0.5f, 0.0f);

		static inline CurveVertex P0 = CurveVertex(0.0f, 0.5f, -3.3f);
		static inline CurveVertex P1 = CurveVertex(0.0f, 0.5f, 0.0f);
		static inline CurveVertex P2 = CurveVertex(0.0f, 0.5f, 3.3f);
		static inline CurveVertex P3 = CurveVertex(0.0f, 0.5f, 6.6f);
		static inline CurveVertex P4 = CurveVertex(0.0f, 0.5f, 10.0f);

		//////////////////////////////////////////////////////////////

		static inline CurveVertex P5 = CurveVertex(0.0f, 0.5f, 10.0f);
		static inline CurveVertex P6 = CurveVertex(1.0f, 0.5f, 17.5f);
		static inline CurveVertex P7 = CurveVertex(7.5f, 0.5f, 17.5f);
		static inline CurveVertex P8 = CurveVertex(15.0f, 0.5f, 10.0f);

		//////////////////////////////////////////////////////////////

		static inline CurveVertex P9 = CurveVertex(15.0f, 0.5f, 10.0f);
		static inline CurveVertex P10 = CurveVertex(19.0f, 0.5f, 5.5f);
		static inline CurveVertex P11 = CurveVertex(22.5f, 0.5f, 5.5f);
		static inline CurveVertex P12 = CurveVertex(26.0f, 0.5f, 7.7f);

		//////////////////////////////////////////////////////////////

		static inline CurveVertex P13 = CurveVertex(26.0f, 0.5f, 7.7f);
		static inline CurveVertex P14 = CurveVertex(30.0f, 0.5f, 10.5f);
		static inline CurveVertex P15 = CurveVertex(35.5f, 0.5f, 13.5f);
		static inline CurveVertex P16 = CurveVertex(40.0f, 0.5f, 10.0f);

		//////////////////////////////////////////////////////////////

		static inline CurveVertex P17 = CurveVertex(40.0f, 0.5f, -10.0f);
		static inline CurveVertex P18 = CurveVertex(35.5f, 0.5f, -13.5f);
		static inline CurveVertex P19 = CurveVertex(30.0f, 0.5f, -10.5f);
		static inline CurveVertex P20 = CurveVertex(26.0f, 0.5f, -7.7f);

		//////////////////////////////////////////////////////////////

		static inline CurveVertex P21 = CurveVertex(26.0f, 0.5f, -7.7f);
		static inline CurveVertex P22 = CurveVertex(22.5f, 0.5f, -5.5f);
		static inline CurveVertex P23 = CurveVertex(19.0f, 0.5f, -5.5f);
		static inline CurveVertex P24 = CurveVertex(15.0f, 0.5f, -10.0f);

		//////////////////////////////////////////////////////////////

		static inline CurveVertex P25 = CurveVertex(15.0f, 0.5f, -10.0f);
		static inline CurveVertex P26 = CurveVertex(7.5f, 0.5f, -17.5f);
		static inline CurveVertex P27 = CurveVertex(1.0f, 0.5f, -17.5f);
		static inline CurveVertex P28 = CurveVertex(0.0f, 0.5f, -10.0f);

		//////////////////////////////////////////////////////////////

		static inline CurveVertex P29 = CurveVertex(0.0f, 0.5f, -10.0f);
		static inline CurveVertex P30 = CurveVertex(0.0f, 0.5f, -6.6f);
		static inline CurveVertex P31 = CurveVertex(0.0f, 0.5f, -3.3f);
		static inline CurveVertex P32 = CurveVertex(0.0f, 0.5f, 0.0f);
		static inline CurveVertex P33 = CurveVertex(0.0f, 0.5f, 3.3f);

		//////////////////////////////////////////////////////////////

		std::vector<CurveVertex> mControlPoints;
		std::vector<CurveVertex> mInterpolatedPoints;
		std::vector<glm::mat4> mControlPointsMatrices;
		std::vector<CurveVertex> points;

		struct {
			Buffer<CurveVertex> controPoints;
			Buffer<CurveVertex> interpolatedPoints;
		} mBuffers;

		std::vector<TableValue> mArcTable;

		float someFactor = 6.0f;
	};
}
