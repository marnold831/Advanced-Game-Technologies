#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class BoundingCapsule : public CollisionVolume {
	public:
		BoundingCapsule();
		virtual ~BoundingCapsule();


		void SetHeight(float _height) { height = _height; }
		float GetHeight() const { return height; }

		void SetRadius(float _radius) { radius = _radius; }
		float GetRadius() const { return radius; }

	protected:
		float height;
		float radius;

	};
}