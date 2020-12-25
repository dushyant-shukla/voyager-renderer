#pragma once

#include "Particle.h"

namespace vr
{
	class Constraint
	{
	private:
		float restDistance; // the length between particle p1 and p2 in rest configuration

	public:
		Particle* p1, * p2; // the two particles that are connected through this constraint

		Constraint(Particle* p1, Particle* p2) : p1(p1), p2(p2)
		{
			restDistance = glm::distance(p1->GetPosition(), p2->GetPosition());
		}

		/* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
		the method is called by Cloth.time_step() many times per frame*/
		void SatisfyConstraint()
		{
			glm::vec3 p1_to_p2 = p2->GetPosition() - p1->GetPosition();
			float current_distance = glm::distance(p1->GetPosition(), p2->GetPosition());
			glm::vec3 correctionVector = p1_to_p2 * (1 - restDistance / current_distance);
			glm::vec3 correctionVectorHalf = correctionVector * 0.5f;
			p1->OffsetPosition(correctionVectorHalf);
			p2->OffsetPosition(-correctionVectorHalf);
		}
	};
}
