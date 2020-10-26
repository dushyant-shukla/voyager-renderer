#include "Splines.h"

#include <stack>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace vr
{
	Splines::Splines()
	{
		mControlPoints.push_back(P0);
		mControlPoints.push_back(P1);
		mControlPoints.push_back(P2);
		mControlPoints.push_back(P3);
		mControlPoints.push_back(P4);
		mControlPoints.push_back(P5);
		mControlPoints.push_back(P6);
		mControlPoints.push_back(P7);
		mControlPoints.push_back(P8);
		mControlPoints.push_back(P9);
		mControlPoints.push_back(P10);
		mControlPoints.push_back(P11);
		mControlPoints.push_back(P12);
		mControlPoints.push_back(P13);
		mControlPoints.push_back(P14);
		mControlPoints.push_back(P15);
		mControlPoints.push_back(P16);

		mControlPoints.push_back(P17);
		mControlPoints.push_back(P18);
		mControlPoints.push_back(P19);
		mControlPoints.push_back(P20);
		mControlPoints.push_back(P21);
		mControlPoints.push_back(P22);
		mControlPoints.push_back(P23);
		mControlPoints.push_back(P24);
		mControlPoints.push_back(P25);
		mControlPoints.push_back(P26);
		mControlPoints.push_back(P27);
		mControlPoints.push_back(P28);
		mControlPoints.push_back(P29);
		mControlPoints.push_back(P30);
		mControlPoints.push_back(P31);
		mControlPoints.push_back(P32);
		mControlPoints.push_back(P33);

		CalculateCurve();

		// create buffers
		mBuffers.controPoints.Create(mControlPoints, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		mBuffers.interpolatedPoints.Create(mInterpolatedPoints, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}

	Splines::~Splines()
	{
	}

	void Splines::CalculateCurve()
	{
		for (unsigned int i = 0; i < mControlPoints.size() - 3; ++i)
		{
			glm::mat4 temp;
			temp[0] = mControlPoints[i].position;
			temp[1] = mControlPoints[i + 1].position;
			temp[2] = mControlPoints[i + 2].position;
			temp[3] = mControlPoints[i + 3].position;
			mControlPointsMatrices.push_back(glm::transpose(temp));
		}

		for (unsigned int i = 0; i < mControlPointsMatrices.size(); ++i)
		{
			for (float j = 0.0f; j <= 1.0f; j += 0.0001f)
			{
				mInterpolatedPoints.push_back(CalculateBSpline(mControlPointsMatrices[i], j));
			}
		}
	}

	void Splines::DrawSpline(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { mBuffers.interpolatedPoints.mBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdDraw(commandBuffer, mInterpolatedPoints.size(), 1, 0, 0);
	}

	void Splines::DrawControlPoints(VkCommandBuffer commandBuffer)
	{
		VkBuffer controlPointsBuffers[] = { mBuffers.controPoints.mBuffer };
		VkDeviceSize controlPointsOffsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, controlPointsBuffers, controlPointsOffsets);
		vkCmdDraw(commandBuffer, mControlPoints.size(), 1, 0, 0);
	}

	CurveVertex Splines::CalculateBSpline(glm::mat4 matrix, float t)
	{
		glm::mat4 bsplineMatrix;
		bsplineMatrix[0] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
		bsplineMatrix[1] = glm::vec4(3.0f, -6.0f, 3.0f, 0.0f);
		bsplineMatrix[2] = glm::vec4(-3.0f, 0.0f, 3.0f, 0.0f);
		bsplineMatrix[3] = glm::vec4(1.0f, 4.0f, 1.0f, 0.0f);

		bsplineMatrix /= someFactor;

		bsplineMatrix = glm::transpose(bsplineMatrix);

		glm::vec4 position = glm::vec4(t * t * t, t * t, t, 1.0f) * bsplineMatrix * matrix;
		return CurveVertex(position);
	}

	CurveVertex Splines::CalculateBSplineDerivative(glm::mat4 matrix, float t)
	{
		glm::mat4 bsplineMatrix;
		bsplineMatrix[0] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
		bsplineMatrix[1] = glm::vec4(3.0f, -6.0f, 3.0f, 0.0f);
		bsplineMatrix[2] = glm::vec4(-3.0f, 0.0f, 3.0f, 0.0f);
		bsplineMatrix[3] = glm::vec4(1.0f, 4.0f, 1.0f, 0.0f);

		bsplineMatrix /= someFactor;

		bsplineMatrix = glm::transpose(bsplineMatrix);

		glm::vec4 position = glm::vec4(3 * t * t, 2 * t, 1.0, 0.0f) * bsplineMatrix * matrix;
		return CurveVertex(position);
	}

	TableValue Splines::FindInTable(float distance)
	{
		for (unsigned int i = 0; i < mArcTable.size() - 1; ++i)
		{
			float s, alpha;
			float d1, d2;
			int curveIndex;
			if ((mArcTable[i].distance < distance) && (mArcTable[i + 1].distance > distance))
			{
				// lerp between points
				alpha = (distance - mArcTable[i].distance) / (mArcTable[i + 1].distance - mArcTable[i].distance);
				if (mArcTable[i].curveIndex != mArcTable[i + 1].curveIndex)
				{
					d1 = 0.0f;
					d2 = mArcTable[i + 1].pointOnCurve;
					s = glm::lerp(d1, d2, alpha);
					curveIndex = mArcTable[i + 1].curveIndex;
				}
				else
				{
					d1 = mArcTable[i + 1].pointOnCurve;
					d2 = mArcTable[i].pointOnCurve;
					s = glm::lerp(d2, d1, alpha);
					curveIndex = mArcTable[i].curveIndex;
				}

				return TableValue(0.0f, s, curveIndex);
			}
		}
		return TableValue(0.0f, 0.0f, 0);
	}

	void Splines::CalculateAdaptiveTable(float& t1, float& t2, float& t3)
	{
		float tolerance = 0.1;
		mArcTable.emplace_back(TableValue(0.0f, 0.0f, 0));

		for (unsigned int i = 0; i < mControlPointsMatrices.size(); ++i)
		{
			std::stack<TableValue> tempValueStack;
			tempValueStack.push(TableValue(0.0f, 1.0f, i));

			while (tempValueStack.size() > 0)
			{
				TableValue stackTop = tempValueStack.top();
				tempValueStack.pop();

				int curveIndex = stackTop.curveIndex;
				float s_a = stackTop.distance;
				float s_b = stackTop.pointOnCurve;
				float s_m = (s_a + s_b) / 2.0f;

				CurveVertex P_sa = CalculateBSpline(mControlPointsMatrices[curveIndex], s_a);
				CurveVertex P_sb = CalculateBSpline(mControlPointsMatrices[curveIndex], s_b);
				CurveVertex P_sm = CalculateBSpline(mControlPointsMatrices[curveIndex], s_m);

				float A = glm::length(P_sm.position - P_sa.position);
				float B = glm::length(P_sb.position - P_sm.position);
				float C = glm::length(P_sb.position - P_sa.position);

				float d = A + B - C;
				// tolerance check
				if (d < tolerance)
				{
					int previousLength = mArcTable[(mArcTable.size()) - 1].distance;

					// add previous_lenght + A , and previous_length + A + B to arc Table
					mArcTable.emplace_back(TableValue(previousLength + A, s_m, curveIndex));
					mArcTable.emplace_back(TableValue(previousLength + A + B, s_b, curveIndex));
				}
				// if d is greater then tolerance then we continue
				else
				{
					tempValueStack.push(TableValue(s_m, s_b, curveIndex));
					tempValueStack.push(TableValue(s_a, s_m, curveIndex));
				}
			}
		}

		for (unsigned int i = 0; i < mArcTable.size(); ++i)
		{
			std::cout << "distance: " << mArcTable[i].distance << "\t"
				<< "point on curve: " << mArcTable[i].pointOnCurve << "\t" <<
				"curveIndex: " << mArcTable[i].curveIndex << std::endl;
		}

		t3 = mArcTable[mArcTable.size() - 1].distance / someFactor;
		t1 = 0.3f * t3;
		t2 = 0.9f * t3;
		t3 += (t1 + (t3 - t2));
	}
}