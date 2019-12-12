#pragma once
#include "..\\..\\Common\Camera.h"
#include "GameObject.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"
//#include "..\\GameTech\TutorialGame.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame;

		class GooseObject : public GameObject {
		public:
			GooseObject(string name = "", uint32_t layer = UINT32_MAX);
			virtual ~GooseObject();

			void Update(float dt) override;

			void SetCamera(Camera* _camera) { camera = _camera; }
			void OnCollisionBegin(GameObject* otherObject) override;
			void OnCollisionEnd(GameObject* otherObject) override;

			void SetHomeBoundary(Vector2 boundary) { homeBoundary = boundary; }

			void SetGame(TutorialGame* tutorialGame) { game = tutorialGame; }

			void SetScareMode(bool mode) { scaredMode = mode; }

			bool IsHoldingBonus() { return isHoldingBonus; }

			void SetCaught(bool _caught) { caught = _caught; }

			void SetSpawnPosition(Vector3 position) { 
				prevFramePos = position;
				currentFramePos = position;
				spawnPos = position; 
			}
			
			Vector2 GetHomeBoundary() { return homeBoundary; }
			

		private:
			void MouseListener();
			void KeyboardListener();
			void Scared();
			void TransverseScaredPath();
			

			Camera* camera;
			GameObject* holdingObject;
			Vector3 prevFramePos;
			Vector3 currentFramePos;
			Vector2 homeBoundary;
			TutorialGame* game;

			NavigationGrid* grid;
			NavigationPath* path;
			Vector3 targetPos;
			Vector3 spawnPos;

			bool scaredMode;
			bool isHoldingBonus;
			bool caught;
			bool generatedScaredPath;
			bool collisionWater;
			float timer;
		};
	}
}