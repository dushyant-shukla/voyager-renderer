#pragma once

#include "Constraint.h"
#include "Particle.h"
#include "graphics/vulkan/Model.h"

#include <vector>
#include <map>

namespace vr
{
	class Cloth
	{
	public:

		int horizontalParticleCount; // number of particles in "width" direction
		int verticalParticleCount; // number of particles in "height" direction
		// total number of particles is num_particles_width*num_particles_height

		std::map<std::string, std::vector<std::pair<int, int>>> pinnedParticles;

		std::vector<Particle> particles; // all particles that are part of this cloth
		std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth
		std::vector<glm::vec2> UV;

		Particle* GetParticle(int x, int y)
		{
			return &particles[y * horizontalParticleCount + x];
		}

		void MakeConstraint(Particle* p1, Particle* p2) { constraints.push_back(Constraint(p1, p2)); }

		/* A private method used by drawShaded() and addWindForcesForTriangle() to retrieve the
		normal vector of the triangle defined by the position of the particles p1, p2, and p3.
		The magnitude of the normal vector is equal to the area of the parallelogram defined by p1, p2 and p3
		*/
		glm::vec3 CalculateTriangleNormal(Particle* p1, Particle* p2, Particle* p3)
		{
			glm::vec3 pos1 = p1->GetPosition();
			glm::vec3 pos2 = p2->GetPosition();
			glm::vec3 pos3 = p3->GetPosition();

			glm::vec3 v1 = pos2 - pos1;
			glm::vec3 v2 = pos3 - pos1;

			return  glm::cross(v1, v2);
		}

		/* A private method used by windForce() to calcualte the wind force for a single triangle
		defined by p1,p2,p3*/
		void AddWindForcesForTriangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3 direction)
		{
			glm::vec3 normal = CalculateTriangleNormal(p1, p2, p3);
			glm::normalize(normal);
			glm::vec3 d = normal;
			glm::vec3 force = normal * (glm::dot(d, direction));
			p1->AddForce(force);
			p2->AddForce(force);
			p3->AddForce(force);
		}

	public:

		Cloth() {}
		/* This is a important constructor for the entire system of particles and constraints*/
		Cloth(float width, float height, int particleCountwidth, int particleCountHeight) : horizontalParticleCount(particleCountwidth), verticalParticleCount(particleCountHeight)
		{
			particles.resize(particleCountwidth * particleCountHeight);
			UV.resize(particleCountwidth * particleCountHeight);

			// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
			for (int x = 0; x < particleCountwidth; x++)
			{
				for (int y = 0; y < particleCountHeight; y++)
				{
					glm::vec3 pos = glm::vec3(width * (x / (float)particleCountwidth),
						-height * (y / (float)particleCountHeight),
						0.0f);

					particles[y * particleCountwidth + x] = Particle(pos, glm::vec2(pos.x, -pos.y)); // insert particle in column x at y'th row
				}
			}

			// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
			for (int x = 0; x < particleCountwidth; x++)
			{
				for (int y = 0; y < particleCountHeight; y++)
				{
					if (x < particleCountwidth - 1) MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y));
					if (y < particleCountHeight - 1) MakeConstraint(GetParticle(x, y), GetParticle(x, y + 1));
					if (x < particleCountwidth - 1 && y < particleCountHeight - 1) MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y + 1));
					if (x < particleCountwidth - 1 && y < particleCountHeight - 1) MakeConstraint(GetParticle(x + 1, y), GetParticle(x, y + 1));
				}
			}

			// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
			for (int x = 0; x < particleCountwidth; x++)
			{
				for (int y = 0; y < particleCountHeight; y++)
				{
					if (x < particleCountwidth - 2) MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y));
					if (y < particleCountHeight - 2) MakeConstraint(GetParticle(x, y), GetParticle(x, y + 2));
					if (x < particleCountwidth - 2 && y < particleCountHeight - 2) MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y + 2));
					if (x < particleCountwidth - 2 && y < particleCountHeight - 2) MakeConstraint(GetParticle(x + 2, y), GetParticle(x, y + 2));
				}
			}

			// making the upper left most three and right most three particles unmovable
			std::vector<std::pair<int, int>> upperLeft;  // 0
			std::vector<std::pair<int, int>> upperRight; // 1
			std::vector<std::pair<int, int>> bottomLeft; // 2
			std::vector<std::pair<int, int>> bottomRight;// 3
			for (int i = 0; i < 3; i++)
			{
				// upper left
				GetParticle(0 + i, 0)->OffsetPosition(glm::vec3(0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
				GetParticle(0 + i, 0)->MakeUnmovable(); // upper left
				upperLeft.emplace_back(0 + i, 0);

				// upper right
				GetParticle(particleCountwidth - 1 - i, 0)->OffsetPosition(glm::vec3(-0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
				GetParticle(particleCountwidth - 1 - i, 0)->MakeUnmovable();// upper right
				upperRight.emplace_back(particleCountwidth - 1 - i, 0);

				// bottom left
				GetParticle(0, particleCountHeight - 1 - i)->OffsetPosition(glm::vec3(0.5, 0.0, 0.0));
				GetParticle(0, particleCountHeight - 1 - i)->MakeUnmovable(); // bottom left
				bottomLeft.emplace_back(0, particleCountHeight - 1 - i);

				// bottom right
				GetParticle(particleCountwidth - 1, particleCountHeight - 1 - i)->OffsetPosition(glm::vec3(-0.5, 0.0, 0.0));
				GetParticle(particleCountwidth - 1, particleCountHeight - 1 - i)->MakeUnmovable(); // bottom right
				bottomRight.emplace_back(particleCountwidth - 1, particleCountHeight - 1 - i);
			}

			pinnedParticles["upper-left"] = upperLeft;
			pinnedParticles["upper-right"] = upperRight;
			pinnedParticles["bottom-left"] = bottomLeft;
			pinnedParticles["bottom-right"] = bottomRight;
		}

		/* this is an important methods where the time is progressed one time step for the entire cloth.
		This includes calling satisfyConstraint() for every constraint, and calling timeStep() for all particles
		*/
		void TimeStep()
		{
			std::vector<Constraint>::iterator constraint;
			for (int i = 0; i < Particle::CONSTRAINT_ITERATIONS; i++) // iterate over all constraints several times
			{
				for (constraint = constraints.begin(); constraint != constraints.end(); constraint++)
				{
					(*constraint).SatisfyConstraint(); // satisfy constraint.
				}
			}

			std::vector<Particle>::iterator particle;
			for (particle = particles.begin(); particle != particles.end(); particle++)
			{
				(*particle).TimeStep(); // calculate the position of each particle at the next time step.
			}
		}

		/* used to add gravity (or any other arbitrary vector) to all particles*/
		void AddForce(const glm::vec3 direction)
		{
			std::vector<Particle>::iterator particle;
			for (particle = particles.begin(); particle != particles.end(); particle++)
			{
				(*particle).AddForce(direction); // add the forces to each particle
			}
		}

		/* used to add wind forces to all particles, is added for each triangle since the final force is proportional to the triangle area as seen from the wind direction*/
		void WindForce(const glm::vec3 direction)
		{
			for (int y = 0; y < verticalParticleCount - 1; y++)
			{
				for (int x = 0; x < horizontalParticleCount - 1; x++)
				{
					AddWindForcesForTriangle(GetParticle(x + 1, y), GetParticle(x, y), GetParticle(x, y + 1), direction);
					AddWindForcesForTriangle(GetParticle(x + 1, y + 1), GetParticle(x + 1, y), GetParticle(x, y + 1), direction);
				}
			}
		}

		/* used to detect and resolve the collision of the cloth with the ball.
		This is based on a very simples scheme where the position of each particle is simply compared to the sphere and corrected.
		This also means that the sphere can "slip through" if the ball is small enough compared to the distance in the grid bewteen particles
		*/
		void BallCollision(const glm::vec3 center, const float radius)
		{
			std::vector<Particle>::iterator particle;
			for (particle = particles.begin(); particle != particles.end(); particle++)
			{
				glm::vec3 v = (*particle).GetPosition() - center;
				float l = glm::distance((*particle).GetPosition(), center);
				if (l < radius) // if the particle is inside the ball
				{
					glm::normalize(v);
					(*particle).OffsetPosition(v * (radius - l)); // project the particle to the surface of the ball
				}
			}
		}
	};
}