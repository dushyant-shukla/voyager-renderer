#pragma once

#include "Particle.h"

namespace vr
{
	class Constraint
	{
	private:
		float rest_distance; // the length between particle p1 and p2 in rest configuration

	public:
		Particle* p1, * p2; // the two particles that are connected through this constraint

		Constraint(Particle* p1, Particle* p2) : p1(p1), p2(p2)
		{
			//glm::vec3 vec = p1->getPos() - p2->getPos();
			rest_distance = glm::distance(p1->getPos(), p2->getPos());
		}

		/* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
		the method is called by Cloth.time_step() many times per frame*/
		void satisfyConstraint()
		{
			glm::vec3 p1_to_p2 = p2->getPos() - p1->getPos();
			float current_distance = glm::distance(p1->getPos(), p2->getPos());
			glm::vec3 correctionVector = p1_to_p2 * (1 - rest_distance / current_distance);
			glm::vec3 correctionVectorHalf = correctionVector * 0.5f;
			p1->offsetPos(correctionVectorHalf);
			p2->offsetPos(-correctionVectorHalf);
		}
	};
}
