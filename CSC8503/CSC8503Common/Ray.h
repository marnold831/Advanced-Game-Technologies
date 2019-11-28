#pragma once
#include "../../Common/Vector3.h"
#include "../../Common/Plane.h"

namespace NCL {
	namespace Maths {
		struct RayCollision {
			void*		node;			//Node that was hit
			Vector3		collidedAt;		//WORLD SPACE position of the collision!
			float		rayDistance;

			RayCollision(void*node, Vector3 collidedAt) {
				this->node			= node;
				this->collidedAt	= collidedAt;
				this->rayDistance	= 0.0f;
			}

			RayCollision() {
				node			= nullptr;
				rayDistance		= FLT_MAX;
			}
		};

		class Ray {
		public:
			Ray(Vector3 position, Vector3 direction, uint32_t mask = UINT32_MAX) : position(position), direction(direction), mask(mask){ }
			~Ray(void) {}

			Vector3 GetPosition() const {return position;	}

			Vector3 GetDirection() const {return direction;	}

			uint32_t GetMask() const { return mask; }
			void SetMask(uint32_t _mask) { mask = _mask; }

		protected:
			Vector3 position;	//World space position
			Vector3 direction;	//Normalised world space direction
			uint32_t mask;
		};
	}
}