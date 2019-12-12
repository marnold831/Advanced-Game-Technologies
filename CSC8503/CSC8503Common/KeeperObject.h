#pragma once

#include "GameObject.h"
#include "GooseObject.h"
#include "NavigationGrid.h"
#include "StateMachine.h"

namespace NCL {
	namespace CSC8503 {
		class KeeperObject : public GameObject {
		public:

			KeeperObject(string name = "", uint32_t layer = UINT32_MAX, GooseObject* goose = nullptr);
			virtual ~KeeperObject();
			
			void Update(float dt);

			NavigationGrid* GetNavigationGrid() const { return navigation;}
			NavigationPath* GetNavigationPath() const { return path; }

			void SetGoose(GooseObject* _goose) { goose = _goose; }
			Vector3& GetTargetPos()  { return targetPos; }

			float GetTime() const { return totalTime; }
			void SetTime(float dt) { totalTime = dt; }

		private:
			void UpdatePosition();
			void TestCollidedWithGoose();

			Vector3 targetPos;
			float totalTime;

			NavigationGrid* navigation;
			NavigationPath* path;
			StateMachine* stateMachine;
			GooseObject* goose;



		};
	}
}