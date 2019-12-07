#pragma once
#include "GameWorld.h"

namespace NCL {
	namespace CSC8503 {
		class GooseObject : public GameObject {

		public:
			GooseObject(string objectName = "", uint32_t layer = UINT32_MAX, GameWorld* world = nullptr);
			~GooseObject();

			void Update();

			
			void UpdateCameraPos();

		private:
			void GooseKeyboardListener(GameWorld* world);
			void GooseMouseListener(GameWorld* world);

			Vector3 currentPos;
			Vector3 prevPos;
			Vector3 cameraPos;
			Vector3 camClampToGoose;
			GameWorld* world;

		};
	}
}