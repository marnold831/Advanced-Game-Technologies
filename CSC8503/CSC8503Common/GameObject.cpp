#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName, uint32_t _layer)	{
	m_Name				= objectName;
	m_Layer				= _layer;
	m_IsActive			= true;
	m_BoundingVolume	= nullptr;
	m_PhysicsObject		= nullptr;
	m_RenderObject		= nullptr;
	m_NetworkObject		= nullptr;
}



GameObject::~GameObject()	{
	delete m_BoundingVolume;
	delete m_PhysicsObject;
	delete m_RenderObject;
	delete m_NetworkObject;
}

void GameObject::Update(float dt){
	//std::cout << "Base Object Type" << std::endl;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!m_BoundingVolume) {
		return false;
	}
	outSize = m_BroadphaseAABB;
	return true;
}

//These would be better as a virtual 'ToAABB' type function, really...
void GameObject::UpdateBroadphaseAABB() {
	if (!m_BoundingVolume) {
		return;
	}
	if (m_BoundingVolume->type == VolumeType::AABB) {
		m_BroadphaseAABB = ((AABBVolume&)*m_BoundingVolume).GetHalfDimensions();
	}
	else if (m_BoundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*m_BoundingVolume).GetRadius();
		m_BroadphaseAABB = Vector3(r, r, r);
	}
	else if (m_BoundingVolume->type == VolumeType::OOBB) {
		Matrix3 mat = Matrix3(m_Transform.GetWorldOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OOBBVolume&)*m_BoundingVolume).GetHalfDimensions();
		m_BroadphaseAABB = mat * halfSizes;
	}
}