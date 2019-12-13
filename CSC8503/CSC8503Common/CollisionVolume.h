#pragma once
namespace NCL {
	enum class VolumeType {
		AABB	= 1,
		OOBB	= 2,
		Sphere	= 4, 
		Mesh	= 8,
		Compound= 16,
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