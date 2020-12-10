//#include "Cloth.h"
//#include "graphics/vulkan/utility/MemoryUtility.h"
//#include "RendererState.h"
//
//#include <vulkan/vulkan.h>
//
//vr::Cloth::Cloth()
//{
//	//mVertices.resize(14406);
//}
//
//vr::Cloth::Cloth(const float width, const float height, const unsigned int particleCountWidth, const unsigned int particleCountHeight)
//	: mParticleCountWidth(particleCountWidth), mParticleCountHeight(particleCountHeight)
//{
//	mParticles.resize(mParticleCountWidth * mParticleCountHeight); //I am essentially using this vector as an array with room for num_particles_width*num_particles_height particles
//	mUV.resize(mParticleCountWidth * mParticleCountHeight);
//	//mVertices.resize(14406);
//
//	// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
//	for (int x = 0; x < mParticleCountWidth; x++)
//	{
//		for (int y = 0; y < mParticleCountHeight; y++)
//		{
//			glm::vec3 pos = glm::vec3(width * (x / (float)mParticleCountWidth),
//				-height * (y / (float)mParticleCountHeight),
//				0.0f);
//
//			mParticles[y * mParticleCountWidth + x] = Particle(pos, glm::vec2(pos.x, -pos.y)); // insert particle in column x at y'th row
//		}
//	}
//
//	// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
//	for (int x = 0; x < mParticleCountWidth; x++)
//	{
//		for (int y = 0; y < mParticleCountHeight; y++)
//		{
//			if (x < mParticleCountWidth - 1) MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y));
//			if (y < mParticleCountHeight - 1) MakeConstraint(GetParticle(x, y), GetParticle(x, y + 1));
//			if (x < mParticleCountWidth - 1 && y < mParticleCountHeight - 1) MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y + 1));
//			if (x < mParticleCountWidth - 1 && y < mParticleCountHeight - 1) MakeConstraint(GetParticle(x + 1, y), GetParticle(x, y + 1));
//		}
//	}
//
//	// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
//	for (int x = 0; x < mParticleCountWidth; x++)
//	{
//		for (int y = 0; y < mParticleCountHeight; y++)
//		{
//			if (x < mParticleCountWidth - 2) MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y));
//			if (y < mParticleCountHeight - 2) MakeConstraint(GetParticle(x, y), GetParticle(x, y + 2));
//			if (x < mParticleCountWidth - 2 && y < mParticleCountHeight - 2) MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y + 2));
//			if (x < mParticleCountWidth - 2 && y < mParticleCountHeight - 2) MakeConstraint(GetParticle(x + 2, y), GetParticle(x, y + 2));
//		}
//	}
//
//	// making the upper left most three and right most three particles unmovable
//	for (int i = 0; i < 3; i++)
//	{
//		GetParticle(0 + i, 0)->OffsetPosition(glm::vec3(0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
//		GetParticle(0 + i, 0)->MakeUnmovable();
//
//		GetParticle(0 + i, 0)->OffsetPosition(glm::vec3(-0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
//		GetParticle(mParticleCountWidth - 1 - i, 0)->MakeUnmovable();
//	}
//
//	//VkDeviceSize size = 86436 * sizeof(float);
//	//MemoryUtility::CreateBuffer(size,
//	//	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//	//	ALLOCATION_CALLBACK,
//	//	&mClothBuffer.mBuffer,
//	//	&mClothBuffer.mMemory);
//	//mClothBuffer.Map();
//
//	/////////////////////////////////////// MY WORK ////////////////////////////////////////
//
//	//mParticles.resize(mParticleCountWidth * mParticleCountHeight);
//	//mUV.resize(mParticleCountWidth * mParticleCountHeight);
//
//	//// creating particles in a grid of particles from (0, 0, 0) to (width, -height, 0)
//	//for (unsigned int x = 0; x < mParticleCountWidth; ++x)
//	//{
//	//	for (unsigned int y = 0; y < mParticleCountHeight; ++y)
//	//	{
//	//		glm::vec3 position = glm::vec3(width * (x / (float)mParticleCountWidth),
//	//			-height * (y / (float)mParticleCountHeight),
//	//			0.0f);
//
//	//		mParticles[y * mParticleCountWidth + x] = Particle(position, glm::vec2(position.x, -position.y)); // TODO: don't know the error here if I remove Particle::operator==()
//	//	}
//	//}
//
//	//// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
//	//for (unsigned int x = 0; x < mParticleCountWidth; ++x)
//	//{
//	//	for (unsigned int y = 0; y < mParticleCountHeight; ++y)
//	//	{
//	//		if (x < mParticleCountWidth - 1)
//	//		{
//	//			MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y));
//	//		}
//	//		if (y < mParticleCountHeight - 1)
//	//		{
//	//			MakeConstraint(GetParticle(x, y), GetParticle(x, y + 1));
//	//		}
//	//		if ((x < mParticleCountWidth - 1) && (y < mParticleCountHeight - 1))
//	//		{
//	//			MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y + 1));
//	//		}
//	//		if ((x < mParticleCountWidth - 1) && (y < mParticleCountHeight - 1))
//	//		{
//	//			MakeConstraint(GetParticle(x + 1, y), GetParticle(x, y + 1));
//	//		}
//	//	}
//	//}
//
//	//// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
//	//for (unsigned int x = 0; x < mParticleCountWidth; ++x)
//	//{
//	//	for (unsigned int y = 0; y < mParticleCountHeight; ++y)
//	//	{
//	//		if (x < mParticleCountWidth - 2)
//	//		{
//	//			MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y));
//	//		}
//	//		if (y < mParticleCountHeight - 2)
//	//		{
//	//			MakeConstraint(GetParticle(x, y), GetParticle(x, y + 2));
//	//		}
//	//		if ((x < mParticleCountWidth - 2) && (y < mParticleCountHeight - 2))
//	//		{
//	//			MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y + 2));
//	//		}
//	//		if ((x < mParticleCountWidth - 2) && (y < mParticleCountHeight - 2))
//	//		{
//	//			MakeConstraint(GetParticle(x + 2, y), GetParticle(x, y + 2));
//	//		}
//	//	}
//	//}
//
//	//// making the upper left most three and right most three particles unmovable
//	//for (int i = 0; i < 3; i++)
//	//{
//	//	GetParticle(0 + i, 0)->OffsetPosition(glm::vec3(0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
//	//	GetParticle(0 + i, 0)->MakeUnmovable();
//
//	//	GetParticle(0 + i, 0)->OffsetPosition(glm::vec3(-0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
//	//	GetParticle(mParticleCountWidth - 1 - i, 0)->MakeUnmovable();
//	//}
//}
//
//vr::Cloth::~Cloth()
//{
//}
//
////void vr::Cloth::Initialize(const float width, const float height, const unsigned int particleCountWidth, const unsigned int particleCountHeight)
////{
////	mParticleCountWidth = particleCountWidth;
////	mParticleCountHeight = particleCountHeight;
////	mParticles.resize(mParticleCountWidth * mParticleCountHeight); //I am essentially using this vector as an array with room for num_particles_width*num_particles_height particles
////	mUV.resize(mParticleCountWidth * mParticleCountHeight);
////	mVertices.resize(14406);
////
////	// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
////	for (int x = 0; x < mParticleCountWidth; x++)
////	{
////		for (int y = 0; y < mParticleCountHeight; y++)
////		{
////			glm::vec3 pos = glm::vec3(width * (x / (float)mParticleCountWidth),
////				-height * (y / (float)mParticleCountHeight),
////				0.0f);
////
////			mParticles[y * mParticleCountWidth + x] = Particle(pos, glm::vec2(pos.x, -pos.y)); // insert particle in column x at y'th row
////		}
////	}
////
////	// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
////	for (int x = 0; x < mParticleCountWidth; x++)
////	{
////		for (int y = 0; y < mParticleCountHeight; y++)
////		{
////			if (x < mParticleCountWidth - 1) MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y));
////			if (y < mParticleCountHeight - 1) MakeConstraint(GetParticle(x, y), GetParticle(x, y + 1));
////			if (x < mParticleCountWidth - 1 && y < mParticleCountHeight - 1) MakeConstraint(GetParticle(x, y), GetParticle(x + 1, y + 1));
////			if (x < mParticleCountWidth - 1 && y < mParticleCountHeight - 1) MakeConstraint(GetParticle(x + 1, y), GetParticle(x, y + 1));
////		}
////	}
////
////	// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
////	for (int x = 0; x < mParticleCountWidth; x++)
////	{
////		for (int y = 0; y < mParticleCountHeight; y++)
////		{
////			if (x < mParticleCountWidth - 2) MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y));
////			if (y < mParticleCountHeight - 2) MakeConstraint(GetParticle(x, y), GetParticle(x, y + 2));
////			if (x < mParticleCountWidth - 2 && y < mParticleCountHeight - 2) MakeConstraint(GetParticle(x, y), GetParticle(x + 2, y + 2));
////			if (x < mParticleCountWidth - 2 && y < mParticleCountHeight - 2) MakeConstraint(GetParticle(x + 2, y), GetParticle(x, y + 2));
////		}
////	}
////
////	// making the upper left most three and right most three particles unmovable
////	for (int i = 0; i < 3; i++)
////	{
////		GetParticle(0 + i, 0)->OffsetPosition(glm::vec3(0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
////		GetParticle(0 + i, 0)->MakeUnmovable();
////
////		GetParticle(0 + i, 0)->OffsetPosition(glm::vec3(-0.5, 0.0, 0.0)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
////		GetParticle(mParticleCountWidth - 1 - i, 0)->MakeUnmovable();
////	}
////
////	//VkDeviceSize size = 86436 * sizeof(float);
////	//MemoryUtility::CreateBuffer(size,
////	//	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
////	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
////	//	ALLOCATION_CALLBACK,
////	//	&mClothBuffer.mBuffer,
////	//	&mClothBuffer.mMemory);
////	//mClothBuffer.Map();
////}
//
//vr::Cloth& vr::Cloth::operator=(const Cloth& other)
//{
//	mParticleCountWidth = other.mParticleCountWidth;
//	mParticleCountHeight = other.mParticleCountHeight;
//	mParticles = other.mParticles;
//	mConstraints = other.mConstraints;
//	mUV = other.mUV;
//	//mVertices = other.mVertices;
//	//mClothBuffer = other.mClothBuffer;
//	return *this;
//}
//
//vr::Particle* vr::Cloth::GetParticle(const int x, const int y)
//{
//	return &mParticles[y * mParticleCountWidth + x];
//}
//
//void vr::Cloth::MakeConstraint(Particle* p1, Particle* p2)
//{
//	mConstraints.push_back(Constraint(p1, p2));
//}
//
//void vr::Cloth::TimeStep()
//{
//	std::vector<Constraint>::iterator constraint;
//	for (int i = 0; i < CONSTRAINT_ITERATIONS; ++i) // iterate over all constraints several times
//	{
//		for (constraint = mConstraints.begin(); constraint != mConstraints.end(); ++constraint)
//		{
//			constraint->SatisfyConstraint(); // satisfy constraint.
//		}
//	}
//
//	std::vector<Particle>::iterator particle;
//	for (particle = mParticles.begin(); particle != mParticles.end(); ++particle)
//	{
//		particle->TimeStep(); // calculate the position of each particle at the next time step.
//	}
//}
//
//void vr::Cloth::AddForce(const glm::vec3 direction)
//{
//	std::vector<Particle>::iterator particle;
//	for (particle = mParticles.begin(); particle != mParticles.end(); ++particle)
//	{
//		particle->AddForce(direction);
//	}
//}
//
//void vr::Cloth::WindForce(const glm::vec3 direction)
//{
//	for (int y = 0; y < mParticleCountHeight - 1; y++)
//	{
//		for (int x = 0; x < mParticleCountWidth - 1; x++)
//		{
//			AddWindForcesForTriangle(GetParticle(x + 1, y), GetParticle(x, y), GetParticle(x, y + 1), direction);
//			AddWindForcesForTriangle(GetParticle(x + 1, y + 1), GetParticle(x + 1, y), GetParticle(x, y + 1), direction);
//		}
//	}
//}
//
//void vr::Cloth::BallCollision(const glm::vec3 center, const float radius)
//{
//	std::vector<Particle>::iterator particle;
//	for (particle = mParticles.begin(); particle != mParticles.end(); ++particle)
//	{
//		glm::vec3 v = particle->GetPosition() - center;
//		float l = glm::distance(particle->GetPosition(), center);
//		if (1 < radius) // if the particle is inside the ball
//		{
//			v = glm::normalize(v);
//			particle->OffsetPosition(v * (radius - 1)); // project the particle to the surface of the ball
//		}
//	}
//}
//
//glm::vec3 vr::Cloth::CalculateTriangleNormal(Particle* p1, Particle* p2, Particle* p3)
//{
//	glm::vec3 pos1(p1->GetPosition());
//	glm::vec3 pos2(p2->GetPosition());
//	glm::vec3 pos3(p3->GetPosition());
//
//	glm::vec3 v1 = pos2 - pos1;
//	glm::vec3 v2 = pos3 - pos1;
//
//	return glm::cross(v1, v2);
//}
//
//void vr::Cloth::AddWindForcesForTriangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3 direction)
//{
//	glm::vec3 normal = CalculateTriangleNormal(p1, p2, p3);
//	normal = glm::normalize(normal);
//	glm::vec3 d(normal);
//	glm::vec3 force = normal * glm::dot(d, direction);
//	p1->AddForce(force);
//	p2->AddForce(force);
//	p3->AddForce(force);
//}
//
////void vr::Cloth::UpdateCloth(const glm::vec3 center, const float radius)
////{
//	//AddForce(glm::vec3(0.0, -0.02, 0.02) * Particle::TIME_STEP);
//	//WindForce(glm::vec3(0.0, 0, -1.0) * Particle::TIME_STEP);
//	//TimeStep();
//	////mCloth.BallCollision(ball->mTransform.position, ball->mTransform.scale.x); // position, radius
//	//BallCollision(center, radius); // position, radius
//	//std::vector<Particle>::iterator particle;
//	//for (particle = mParticles.begin(); particle != mParticles.end(); particle++)
//	//{
//	//	particle->ResetNormal();
//	//}
//
//	////create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
//	////if (mVertices.size() == 0)
//	////{
//	////	mVertices.reserve();
//	////}
//	////std::vector<vrassimp::Vertex> mVertices;
//	//int count = 0;
//	//for (int x = 0; x < mParticleCountWidth - 1; ++x)
//	//{
//	//	for (int y = 0; y < mParticleCountHeight - 1; ++y)
//	//	{
//	//		glm::vec3 normal = CalculateTriangleNormal(GetParticle(x + 1, y), GetParticle(x, y), GetParticle(x, y + 1));
//	//		GetParticle(x + 1, y)->AddToNormal(normal);
//	//		GetParticle(x, y)->AddToNormal(normal);
//	//		GetParticle(x, y + 1)->AddToNormal(normal);
//
//	//		Particle* part = GetParticle(x + 1, y);
//	//		//mVertices.emplace_back(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//		//	part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//	//		mVertices[count++] = vrassimp::Vertex(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//			part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//
//	//		////////////////////////////////////////
//
//	//		part = GetParticle(x, y);
//	//		//mVertices.emplace_back(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//		//	part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//	//		mVertices[count++] = vrassimp::Vertex(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//			part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//
//	//		/////////////////////////////////////
//
//	//		part = GetParticle(x, y + 1);
//	//		//mVertices.emplace_back(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//		//	part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//	//		mVertices[count++] = vrassimp::Vertex(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//			part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//
//	//		//////////////////////////////////////////////////////////////////////
//
//	//		normal = CalculateTriangleNormal(GetParticle(x + 1, y + 1), GetParticle(x + 1, y), GetParticle(x, y + 1));
//	//		GetParticle(x + 1, y + 1)->AddToNormal(normal);
//	//		GetParticle(x + 1, y)->AddToNormal(normal);
//	//		GetParticle(x, y + 1)->AddToNormal(normal);
//
//	//		part = GetParticle(x + 1, y + 1);
//	//		//mVertices.emplace_back(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//		//	part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//	//		mVertices[count++] = vrassimp::Vertex(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//			part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//
//	//		////////////////////////////////////////
//
//	//		part = GetParticle(x + 1, y);
//	//		//mVertices.emplace_back(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//		//	part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//	//		mVertices[count++] = vrassimp::Vertex(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//			part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//
//	//		/////////////////////////////////////
//
//	//		part = GetParticle(x, y + 1);
//	//		//mVertices.emplace_back(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//		//	part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//	//		mVertices[count++] = vrassimp::Vertex(part->GetPosition().x, part->GetPosition().y, part->GetPosition().z,
//	//			part->GetNormal().x, part->GetNormal().y, part->GetNormal().z);
//	//	}
//	//}
//
//	//mClothBuffer.Map();
//	//mClothBuffer.CopyData(&mVertices[0], sizeof(vrassimp::Vertex) * mVertices.size());
////}
//
////void vr::Cloth::Draw(const VkCommandBuffer& cmdBuffer)
////{
////	VkBuffer vertexBuffers[] = { mClothBuffer.GetVulkanBuffer() };	// buffers to bind
////	VkDeviceSize offsets[] = { 0 };									// offsets into buffers being bound
////	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);// command to bind vertex buffers before drawing with them
////
////	vkCmdDraw(cmdBuffer, mVertices.size(), 1, 0, 0);
////}