#pragma once
namespace NCL {
	enum class VolumeType {
		AABB	= 1,
		OOBB		= 2,
		Sphere	= 4,
		Capsule = 8,
		Mesh	= 16,
		Compound= 32,
		Invalid = 256
	};

	class CollisionVolume
	{
	public:
		CollisionVolume() {
			type = VolumeType::Invalid;
		}
		~CollisionVolume() {}

		VolumeType type;
	};
}