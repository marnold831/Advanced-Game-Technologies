#pragma once

#include "GooseObject.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"

namespace NCL {
	namespace CSC8503 {
		class CharacterObject : public GameObject {
			
		public:
			CharacterObject(string name = "", uint32_t layer = UINT32_MAX, GooseObject* goose = nullptr);
			virtual ~CharacterObject();

			void Update(float dt) override;
			
			void SetSpawnPosition(Vector3 position) { spawnPos = position; }

			

		private:

			void TestToChaseGoose();
			void BuildPathToGoose();
			void UpdatePosition();
			void TestCollideWithGoose();

			GooseObject* goose;
			NavigationGrid* grid;
			NavigationPath* path;

			Vector3 targetPos;
			Vector3 spawnPos;
			bool firstChase;
			bool chaseGoose;
			float timer;
		};
	}
}