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
		mControlPoints.push_back(P1);
		mControlPoints.push_back(P2);
		mControlPoints.push_back(P3);
		mControlPoints.push_back(P4);
		mControlPoints.push_back(P5);

		CalculateCurve();

		// create buffers
		mBuffers.controPoints.Create(mControlPoints, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

		CurveVertex p1(-0.5f, 0.5f, 0.0f);
		CurveVertex p2(0.25f, 0.5f, 0.0f);
		CurveVertex p3(0.5f, 0.5f, 0.0f);
		points.push_back(p1);
		points.push_back(p2);
		points.push_back(p3);

		//mBuffers.interpolatedPoints.Create(points, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
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

		//VkBuffer vertexBuffers[] = { mBuffers.interpolatedPoints.mBuffer };
		//VkDeviceSize offsets[] = { 0 };
		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		//vkCmdDraw(commandBuffer, points.size(), 1, 0, 0);

		//VkBuffer vertexBuffers[] = { mBuffers.controPoints.mBuffer };
		//VkDeviceSize offsets[] = { 0 };
		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		//vkCmdDraw(commandBuffer, mControlPoints.size(), 1, 0, 0);
	}

	CurveVertex Splines::CalculateBSpline(glm::mat4 matrix, float t)
	{
		glm::mat4 catmul;
		catmul[0] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
		catmul[1] = glm::vec4(3.0f, -6.0f, 3.0f, 0.0f);
		catmul[2] = glm::vec4(-3.0f, 0.0f, 3.0f, 0.0f);
		catmul[3] = glm::vec4(1.0f, 4.0f, 1.0f, 0.0f);

		catmul /= 6.0f;

		catmul = glm::transpose(catmul);

		glm::vec4 position = glm::vec4(t * t * t, t * t, t, 1.0f) * catmul * matrix;
		return CurveVertex(position);
	}

	TableValue Splines::FindInTable(float distance, float& timer)
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

		t3 = mArcTable[mArcTable.size() - 1].distance / 6.0f;
		t1 = 0.3f * t3;
		t2 = 0.9f * t3;
		t3 += (t1 + (t3 - t2));
	}
}