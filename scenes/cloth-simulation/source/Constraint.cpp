//#include "Constraint.h"
//#include "Particle.h"
//
//vr::Constraint::Constraint() : mP1(nullptr), mP2(nullptr)
//{
//}
//
//vr::Constraint::Constraint(Particle* p1, Particle* p2) : mP1(p1), mP2(p2), mRestDistance(glm::distance(mP1->GetPosition(), mP2->GetPosition()))
//{
//}
//
//vr::Constraint::~Constraint()
//{
//	//delete mP1;
//	//delete mP2;
//}
//
//void vr::Constraint::SatisfyConstraint()
//{
//	glm::vec3 p2p1 = mP2->GetPosition() = mP1->GetPosition();
//	float currentDistance = glm::distance(mP1->GetPosition(), mP2->GetPosition());
//	glm::vec3 correctionVector = p2p1 * (1 - mRestDistance / currentDistance);
//	glm::vec3 correctionVectorHalf = correctionVector * 0.5f;
//	mP1->OffsetPosition(correctionVectorHalf);
//	mP2->OffsetPosition(correctionVectorHalf);
//}
//
//vr::Particle* vr::Constraint::GetParticle1()
//{
//	return mP1;
//}
//
//vr::Particle* vr::Constraint::GetParticle2()
//{
//	return mP2;
//}