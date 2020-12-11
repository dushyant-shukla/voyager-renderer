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

		int num_particles_width; // number of particles in "width" direction
		int num_particles_height; // number of particles in "height" direction
		// total number of particles is num_particles_width*num_particles_height

		std::map<std::string, std::vector<std::pair<int, int>>> pinnedParticles;

		std::vector<Particle> particles; // all particles that are part of this cloth
		std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth
		std::vector<glm::vec2> UV;
		Particle* getParticle(int x, int y)
		{
			return &particles[y * num_particles_width + x];
		}

		void makeConstraint(Particle* p1, Particle* p2) { constraints.push_back(Constraint(p1, p2)); }

		/* A private method used by drawShaded() and addWindForcesForTriangle() to retrieve the
		normal vector of the triangle defined by the position of the particles p1, p2, and p3.
		The magnitude of the normal vector is equal to the area of the parallelogram defined by p1, p2 and p3
		*/
		glm::vec3 calcTriangleNormal(Particle* p1, Particle* p2, Particle* p3)
		{
			glm::vec3 pos1 = p1->getPos();
			glm::vec3 pos2 = p2->getPos();
			glm::vec3 pos3 = p3->getPos();

			glm::vec3 v1 = pos2 - pos1;
			glm::vec3 v2 = pos3 - pos1;

			return  glm::cross(v1, v2);
		}

		/* A private method used by windForce() to calcualte the wind force for a single triangle
		defined by p1,p2,p3*/
		void addWindForcesForTriangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3 direction)
		{
			glm::vec3 normal = calcTriangleNormal(p1, p2, p3);
			glm::normalize(normal);
			glm::vec3 d = normal;
			glm::vec3 force = normal * (glm::dot(d, direction));
			p1->addForce(force);
			p2->addForce(force);
			p3->addForce(force);
		}

	public:

		Cloth() {}
		/* This is a important constructor for the entire system of particles and constraints*/
		Cloth(float width, float height, int num_particles_width, int num_particles_height) : num_particles_width(num_particles_width), num_particles_height(num_particles_height)
		{
			particles.resize(num_particles_width * num_particles_height);
			UV.resize(num_particles_width * num_particles_height);

			// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
			for (int x = 0; x < num_particles_width; x++)
			{
				for (int y = 0; y < num_particles_height; y++)
				{
					glm::vec3 pos = glm::vec3(width * (x / (float)num_particles_width),
						-height * (y / (float)num_particles_height),
						0.0f);

					particles[y * num_particles_width + x] = Particle(pos, glm::vec2(pos.x, -pos.y)); // insert particle in column x at y'th row
				}
			}

			// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
			for (int x = 0; x < num_particles_width; x++)
			{
				for (int y = 0; y < num_particles_height; y++)
				{
					if (x < num_particles_width - 1) makeConstraint(getParticle(x, y), getParticle(x + 1, y));
					if (y < num_particles_height - 1) makeConstraint(getParticle(x, y), getParticle(x, y + 1));
					if (x < num_particles_width - 1 && y < num_particles_height - 1) makeConstraint(getParticle(x, y), getParticle(x + 1, y + 1));
					if (x < num_particles_width - 1 && y < num_particles_height - 1) makeConstraint(getParticle(x + 1, y), getParticle(x, y + 1));
				}
			}

			// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
			for (int x = 0; x < num_particles_width; x++)
			{
				for (int y = 0; y < num_particles_height; y++)
				{
					if (x < num_particles_width - 2) makeConstraint(getParticle(x, y), getParticle(x + 2, y));
					if (y < num_particles_height - 2) makeConstraint(getParticle(x, y), getParticle(x, y + 2));
					if (x < num_particles_width - 2 && y < num_particles_height - 2) makeConstraint(getParticle(x, y), getParticle(x + 2, y + 2));
					if (x < num_particles_width - 2 && y < num_particles_height - 2) makeConstraint(getParticle(x + 2, y), getParticle(x, y + 2));
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
				getParticle(0 + i, 0)->offsetPos(glm::vec3(0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
				getParticle(0 + i, 0)->makeUnmovable(); // upper left
				upperLeft.emplace_back(0 + i, 0);

				// upper right
				getParticle(num_particles_width - 1 - i, 0)->offsetPos(glm::vec3(-0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
				getParticle(num_particles_width - 1 - i, 0)->makeUnmovable();// upper right
				upperRight.emplace_back(num_particles_width - 1 - i, 0);

				// bottom left
				getParticle(0, num_particles_height - 1 - i)->offsetPos(glm::vec3(0.5, 0.0, 0.0));
				getParticle(0, num_particles_height - 1 - i)->makeUnmovable(); // bottom left
				bottomLeft.emplace_back(0, num_particles_height - 1 - i);

				// bottom right
				getParticle(num_particles_width - 1, num_particles_height - 1 - i)->offsetPos(glm::vec3(-0.5, 0.0, 0.0));
				getParticle(num_particles_width - 1, num_particles_height - 1 - i)->makeUnmovable(); // bottom right
				bottomRight.emplace_back(num_particles_width - 1, num_particles_height - 1 - i);
			}

			pinnedParticles["upper-left"] = upperLeft;
			pinnedParticles["upper-right"] = upperRight;
			pinnedParticles["bottom-left"] = bottomLeft;
			pinnedParticles["bottom-right"] = bottomRight;
		}

		/* this is an important methods where the time is progressed one time step for the entire cloth.
		This includes calling satisfyConstraint() for every constraint, and calling timeStep() for all particles
		*/
		void timeStep()
		{
			std::vector<Constraint>::iterator constraint;
			for (int i = 0; i < Particle::CONSTRAINT_ITERATIONS; i++) // iterate over all constraints several times
			{
				for (constraint = constraints.begin(); constraint != constraints.end(); constraint++)
				{
					(*constraint).satisfyConstraint(); // satisfy constraint.
				}
			}

			std::vector<Particle>::iterator particle;
			for (particle = particles.begin(); particle != particles.end(); particle++)
			{
				(*particle).timeStep(); // calculate the position of each particle at the next time step.
			}
		}

		/* used to add gravity (or any other arbitrary vector) to all particles*/
		void addForce(const glm::vec3 direction)
		{
			std::vector<Particle>::iterator particle;
			for (particle = particles.begin(); particle != particles.end(); particle++)
			{
				(*particle).addForce(direction); // add the forces to each particle
			}
		}

		/* used to add wind forces to all particles, is added for each triangle since the final force is proportional to the triangle area as seen from the wind direction*/
		void windForce(const glm::vec3 direction)
		{
			for (int y = 0; y < num_particles_height - 1; y++)
			{
				for (int x = 0; x < num_particles_width - 1; x++)
				{
					addWindForcesForTriangle(getParticle(x + 1, y), getParticle(x, y), getParticle(x, y + 1), direction);
					addWindForcesForTriangle(getParticle(x + 1, y + 1), getParticle(x + 1, y), getParticle(x, y + 1), direction);
				}
			}
		}

		/* used to detect and resolve the collision of the cloth with the ball.
		This is based on a very simples scheme where the position of each particle is simply compared to the sphere and corrected.
		This also means that the sphere can "slip through" if the ball is small enough compared to the distance in the grid bewteen particles
		*/
		void ballCollision(const glm::vec3 center, const float radius)
		{
			std::vector<Particle>::iterator particle;
			for (particle = particles.begin(); particle != particles.end(); particle++)
			{
				glm::vec3 v = (*particle).getPos() - center;
				float l = glm::distance((*particle).getPos(), center);
				if (l < radius) // if the particle is inside the ball
				{
					glm::normalize(v);
					(*particle).offsetPos(v * (radius - l)); // project the particle to the surface of the ball
				}
			}
		}
	};
}