#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"

#include "..\\CSC8503Common\GooseObject.h"
#include "..\\CSC8503Common\KeeperObject.h"
#include "..\\CSC8503Common\CharacterObject.h"

#include "..\\CSC8503Common\HighscorePacketReceiver.h"

#include "../CSC8503Common/GameServer.h"


namespace NCL {
	namespace CSC8503 {
		
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual bool UpdateGame(float dt);

			void DeleteObject(GameObject* object);
			void ServerReceived(bool mode) { serverReceived = mode; }


		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();
			void InitServer();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitWorldFromFile(const std::string& filename);
			void InitMainMenu();
			void BridgeConstraintTest();
			void SimpleGJKTest();

			void SetCameraBehindGoose();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();

			void EndGameListener();
			void EndGameSetup();
			void ReadAndUpdateHighscores();
			void UpdateHighScoreFile(vector<int> indexes, vector<string> names, vector<int> scores);
			void PrintHighScores();
			void gen_random(string& s, const int len);
			
			
			void ServerTick();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, OGLTexture* texture, float inverseMass = 10.0f, string name = "", uint32_t layer = UINT32_MAX);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, OGLTexture* texture, float inverseMass = 10.0f, string name = "", uint32_t layer = UINT32_MAX);
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddParkKeeperToWorld(const Vector3& position, GooseObject* goose);
			GameObject* AddCharacterToWorld(const Vector3& position, GooseObject* goose);
			GameObject* AddAppleToWorld(const Vector3& position);

			void AddOOBBToWorld();

			void AddTreeToWorld(const Vector3& position);


			bool ButtonAAction();
			bool ButtonBAction();
			bool ButtonCAction();
			bool ButtonBackAction(GameObject* button);

			void ShowObjectInfo(GameObject* object);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;
			bool inMainMenuMode;
			bool highScoresRead;
			bool highscoreMode;
			bool objectInfo;
			bool endGame;
			bool debugInfo;
			bool serverReceived;

			float		forceMagnitude;
			float gametimer;
			int colourChangeTimer;
			float displayObjectInfoTimer;
			float servertick;

			GameObject* selectionObject = nullptr;
			GooseObject* goose = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLTexture* startButtonTex = nullptr;
			OGLTexture* fenceTex	= nullptr;
			OGLTexture* quitButtonTex = nullptr;
			OGLTexture* highscoresButtonTex = nullptr;
			OGLTexture* backButtonTex = nullptr;

			OGLShader*	basicShader = nullptr;
			OGLShader* buttonShader = nullptr;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			bool ConvertFromNavSpaceToWorldSpace(Vector3& pos);

			vector<GameObject*> menuObjects;
			int numberofCollectables;
			int score;
			vector<string> names;
			vector<int> scores;
			vector<int> sortedScoreIndex;

			string objectDebugInfo;


			GameServer* server;
			GameClient* client;
			HighscorePacketReceiver* serverReceiver;
			HighscorePacketReceiver* clientReceiver;
		};
	}
}

