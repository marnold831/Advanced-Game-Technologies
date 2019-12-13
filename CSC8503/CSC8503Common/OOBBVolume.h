#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector3.h"
namespace NCL {
	class OOBBVolume : CollisionVolume
	{
	public:
		OOBBVolume(const Maths::Vector3& halfDims) {
			type		= VolumeType::OOBB;
			halfSizes	= halfDims;
		}
		~OOBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}
	protected:
		Maths::Vector3 halfSizes;
	};
}

