#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vr
{
	class Particle
	{
	private:
		bool movable; // can the particle move or not ? used to pin parts of the cloth

		float mass; // the mass of the particle (is always 1 in this example)
		glm::vec3 pos; // the current position of the particle in 3D space
		glm::vec3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
		glm::vec3 acceleration; // a vector representing the current acceleration of the particle
		glm::vec3 accumulated_normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading
		glm::vec2 UV;

	public:
		inline static float TIME_STEP_2 = 0.5f * 0.5f;
		inline static float CONSTRAINT_ITERATIONS = 15;
		inline static float DAMPING = 0.01f;

	public:
		Particle(glm::vec3 pos, glm::vec2 uv) : pos(pos), old_pos(pos), acceleration(glm::vec3(0, 0, 0)), mass(1), movable(true), accumulated_normal(glm::vec3(0, 0, 0)), UV(uv) {}
		Particle() {}

		void addForce(glm::vec3 f)
		{
			acceleration += f / mass;
		}

		/* This is one of the important methods, where the time is progressed a single step size (TIME_STEPSIZE)
		   The method is called by Cloth.time_step()
		   Given the equation "force = mass * acceleration" the next position is found through verlet integration*/
		void timeStep()
		{
			if (movable)
			{
				glm::vec3 temp = pos;
				pos = pos + (pos - old_pos) * (1.0f - DAMPING) + acceleration * TIME_STEP_2;
				old_pos = temp;
				acceleration = glm::vec3(0.0, 0.0, 0.0); // acceleration is reset since it HAS been translated into a change in position (and implicitely into velocity)
			}
		}

		glm::vec3& getPos() { return pos; }

		glm::vec2& getUV() { return UV; }

		void resetAcceleration() { acceleration = glm::vec3(0.0, 0.0, 0.0); }

		void offsetPos(const glm::vec3 v) { if (movable) pos += v; }

		void makeUnmovable() { movable = false; }

		void addToNormal(glm::vec3 normal)
		{
			glm::normalize(normal);
			accumulated_normal += normal;
		}

		glm::vec3& getNormal() { return accumulated_normal; } // notice, the normal is not unit length

		void resetNormal() { accumulated_normal = glm::vec3(0.0, 0.0, 0.0); }
	};
}
