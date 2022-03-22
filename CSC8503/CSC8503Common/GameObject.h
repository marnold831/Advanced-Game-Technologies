#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {
		class NetworkObject;

		class GameObject	{
		public:
			GameObject(string name = "", uint32_t layer = UINT32_MAX);
			virtual ~GameObject();

			virtual void Update(float dt);

			void SetBoundingVolume(CollisionVolume* vol) 
			{
				m_BoundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const 
			{
				return m_BoundingVolume;
			}

			bool IsActive() const 
			{
				return m_IsActive;
			}

			const Transform& GetTransform() const 
			{
				return m_Transform;
			}

			Transform& GetTransform() 
			{
				return m_Transform;
			}

			RenderObject* GetRenderObject() const 
			{
				return m_RenderObject;
			}

			PhysicsObject* GetPhysicsObject() const 
			{
				return m_PhysicsObject;
			}

			NetworkObject* GetNetworkObject() const 
			{
				return m_NetworkObject;
			}

			void SetRenderObject(RenderObject* newObject) 
			{
				m_RenderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) 
			{
				m_PhysicsObject = newObject;
			}

			const string& GetName() const 
			{
				return m_Name;
			}

			uint32_t GetLayer() const 
			{
				return m_Layer;
			}

			void SetDebug(bool mode) { m_DebugInfo = mode; }

			void SetLayer(uint32_t _layer) { m_Layer = _layer; }

			virtual void OnCollisionBegin(GameObject* otherObject) 
			{
				//std::cout << "OnCollisionBegin event occured!\n";
			}

			virtual void OnCollisionEnd(GameObject* otherObject) 
			{
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

		protected:
			Transform			m_Transform;

			CollisionVolume*	m_BoundingVolume;
			PhysicsObject*		m_PhysicsObject;
			RenderObject*		m_RenderObject;
			NetworkObject*		m_NetworkObject;

			bool	m_IsActive;
			bool	m_DebugInfo;
			string	m_Name;
			uint32_t m_Layer;

			Vector3 m_BroadphaseAABB;
		};
	}
}

