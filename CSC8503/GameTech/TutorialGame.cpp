#include "TutorialGame.h"
#include <fstream>
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../../Common/Assets.h"

#include "../CSC8503Common/PositionConstraint.h"
#include "..\\CSC8503Common\Layers.h"


#include <sstream>
#include <string>

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : menuObjects(0), useGravity(false), inSelectionMode(false), inMainMenuMode(true), numberofCollectables(0), gametimer(180.0), colourChangeTimer(0),
highScoresRead(false), highscoreMode(false), objectInfo(false), displayObjectInfoTimer(0.0), endGame(false), debugInfo(false), serverReceived(false), servertick(0.0) {
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	
	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("goose.msh"	 , &gooseMesh);
	loadFunc("CharacterA.msh", &keeperMesh);
	loadFunc("CharacterM.msh", &charA);
	loadFunc("CharacterF.msh", &charB);
	loadFunc("Apple.msh"	 , &appleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	startButtonTex = (OGLTexture*)TextureLoader::LoadAPITexture("StartButton.png");
	quitButtonTex = (OGLTexture*)TextureLoader::LoadAPITexture("QuitButton.png");
	highscoresButtonTex = (OGLTexture*)TextureLoader::LoadAPITexture("HighScoresButton.png");
	backButtonTex = (OGLTexture*)TextureLoader::LoadAPITexture("BackButton.png");

	fenceTex = (OGLTexture*)TextureLoader::LoadAPITexture("fence.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
	buttonShader = new OGLShader("ButtonVert.glsl", "ButtonFrag.glsl");

	InitCamera();
	InitWorld();
	InitServer();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

bool TutorialGame::UpdateGame(float dt) {
	Vector3 cameraPos = world->GetMainCamera()->GetPosition();
	string text = "position: (" + std::to_string(cameraPos.x) + "," + std::to_string(cameraPos.y) + "," + std::to_string(cameraPos.z) +")";
	if(debugInfo)
		Debug::Print(text, Vector2(10, 55));

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		LockedCameraMovement();
	}

	UpdateKeys();

	if (useGravity &&  debugInfo) {
		Debug::Print("(G)ravity on", Vector2(10, 40));
	}
	else if(debugInfo) {
		Debug::Print("(G)ravity off", Vector2(10, 40));
	}
	if (SelectObject()) {
		string objectName = selectionObject->GetName();
		if (objectName == "buttonA") {
			ButtonAAction();
		}
		if (objectName == "buttonB") {
			ButtonBAction();
		}
		if (objectName == "buttonD") {
			return false;
		}
		if (objectName == "buttonBack") {
			ButtonBackAction(selectionObject);
		}
		if (objectName != "buttonA" && objectName != "buttonB" && objectName != "buttonD" && objectName != "buttonBack") {
			ShowObjectInfo(selectionObject);
		}

	}


	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	Debug::FlushRenderables();
	renderer->Render();

	if (world->GetinGameMode()) {
		gametimer -= dt;
	}
	
	if (gametimer < 15) {
		colourChangeTimer++;
	}
	if(colourChangeTimer%2 == 0)
		renderer->DrawString(std::to_string(gametimer), Vector2(1175, 650), 0.5f, Vector4(1, 1, 1, 1));
	else
		renderer->DrawString(std::to_string(gametimer), Vector2(1175, 650), 0.5f, Vector4(1, 0, 0, 1));
	
	renderer->DrawString("Score: " + std::to_string(score), Vector2(10, 650), 0.5f, Vector4(0, 1, 0, 1));

	if(highscoreMode)
		PrintHighScores();
	EndGameListener();

	if (objectInfo) {
		renderer->DrawString(objectDebugInfo, Vector2(10, 100), 0.25f, Vector4(0, 1, 0, 1));
		displayObjectInfoTimer += dt;
	}
	if (displayObjectInfoTimer > 3) {
		objectInfo = false;
		displayObjectInfoTimer = 0.0;
	}

	servertick += dt;
	if (servertick > 3) {
		ServerTick();
		servertick = 0.0;
	}
	
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) {
		world->SetinGameMode((world->GetinGameMode()) ? false : true);
		world->GetMainCamera()->SetToggleControls(!world->GetMainCamera()->GetToggleControls());

		if (world->GetinGameMode())
			SetCameraBehindGoose();	
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F4)) {
		debugInfo = !debugInfo;
		world->SetDebugMode(debugInfo);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		selectionObject->GetPhysicsObject()->AddForce(-rightAxis);
	}
	
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		selectionObject->GetPhysicsObject()->AddForce(rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}
}

void  TutorialGame::LockedCameraMovement() {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);
	}
}

void NCL::CSC8503::TutorialGame::EndGameListener() {
	if (numberofCollectables <= 0 || gametimer <= 0) {
		endGame = true;
		EndGameSetup();

	}
}

void NCL::CSC8503::TutorialGame::EndGameSetup() {
	Debug::FlushRenderables();
	Debug::Print("GameOver", Vector2(550, 600));
	world->SetinGameMode(false);
	world->GetMainCamera()->SetToggleControls(true);
	inSelectionMode = true;
	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(false);

	if (!highScoresRead) {
		ReadAndUpdateHighscores();
		highScoresRead = true;
	}
	

	PrintHighScores();
}

void NCL::CSC8503::TutorialGame::ReadAndUpdateHighscores() {
	std::ifstream infile;
	infile.open(Assets::DATADIR + "highscores.txt");
	
	string line;

	while (std::getline(infile, line)) {
		std::stringstream ss;
		string temp;
		int number;
		ss << line;
		while (!ss.eof()) {
			ss >> temp;

			if (std::stringstream(temp) >> number)
				scores.push_back(number);
			else
				names.push_back(temp);

			temp = "";
		}
	}
	if (endGame) {
		scores.push_back(score);
		string name;
		gen_random(name, 5);
		names.push_back(name);
	}
	

	vector<int> sortedScores = scores;
	std::sort(sortedScores.begin(), sortedScores.end());
	std::reverse(sortedScores.begin(), sortedScores.end());

	

	for (int i = 0; i < sortedScores.size(); ++i) {
		int  j = 0;
		while (true) {
			if (sortedScores[i] == scores[j]) {
				sortedScoreIndex.push_back(j);
				break;
			}
			j++;
		}
	}
	
	if (endGame) {
		UpdateHighScoreFile(sortedScoreIndex, names, scores);
	}
	
	

}
void NCL::CSC8503::TutorialGame::UpdateHighScoreFile(vector<int> indexes, vector<string> names, vector<int> score) {
	std::ofstream outfile;
	outfile.open(Assets::DATADIR + "highscores.txt");
	for (auto i : indexes) {
		string outputString = names[i] + " " + std::to_string(score[i]) + "\n";
		outfile << outputString;
	}
}
void NCL::CSC8503::TutorialGame::PrintHighScores() {
	renderer->DrawString("Highscores", Vector2(50,520), 0.5f);
	for (int i = 0; i < sortedScoreIndex.size(); ++i) {
		string line = names[sortedScoreIndex[i]] + " " + std::to_string(scores[sortedScoreIndex[i]]);
		renderer->DrawString(line, Vector2(50, (500 - (i * 20))), 0.5f);
	}
}
void TutorialGame::gen_random(string &s, const int len) {
	s.resize(len);
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	s[len] = 0;
}

void NCL::CSC8503::TutorialGame::ServerTick() {
	string message;
	string holdingMessage;
	std::ifstream myFile(Assets::DATADIR + "highscores.txt");
	while (std::getline(myFile, holdingMessage)) {
		message = message + " "+ holdingMessage;
	}
	if(serverReceived)
		server->SendGlobalPacket(StringPacket(message));

	client->SendPacket(StringPacket(message));

	server->UpdateServer();
	client->UpdateClient();

	
}

void TutorialGame::DeleteObject(GameObject* object) {
	if (object->GetName() == "apple")
		score++;
	if (object->GetName() == "bonus")
		score += 3;
	physics->DeleteObject(object);
	world->DeleteObject(object);
	delete object;
	numberofCollectables--;

}


void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		world->SetinGameMode(!world->GetinGameMode());
		
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0), 0.5);

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
			if (inMainMenuMode)
				ray.SetMask(MENU_LAYER);
			else
				ray.SetMask(CLICKABLE_LAYER);
			
			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				std::cout << selectionObject->GetName() << std::endl;
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0), 0.5f);
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {

	renderer->DrawString("Click Force: " + std::to_string(forceMagnitude), Vector2(10, 20), 0.5f);

	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.f;

	if (!selectionObject)
		return;

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(0.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	world->GetMainCamera()->SetPosition(Vector3(0, 23.5, 20));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	
	goose = (GooseObject*)AddGooseToWorld(Vector3(30, 2, 0));
	goose->SetGame(this);

	AddParkKeeperToWorld(Vector3(-60, 2, -50), (GooseObject*)goose);
	AddCharacterToWorld(Vector3(45, 2, 0), goose);

	InitWorldFromFile("TestGrid1.txt");
	AddFloorToWorld(Vector3(0, -2, 0));
	InitMainMenu();
	
	Vector3 gooseSpawn = Vector3(goose->GetHomeBoundary().x, 2.0, goose->GetHomeBoundary().y);
	goose->GetTransform().SetWorldPosition(gooseSpawn);
	goose->SetSpawnPosition(gooseSpawn);

	inSelectionMode = true;
	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(false);
}

void NCL::CSC8503::TutorialGame::InitServer() {
	NetworkBase::Initialise();

	serverReceiver = new HighscorePacketReceiver("Server", this);
	clientReceiver = new HighscorePacketReceiver("client", this);

	int port = NetworkBase::GetDefaultPort();

	server = new GameServer(port, 1);
	client = new GameClient();

	server->RegisterPacketHandler(String_Message, serverReceiver);
	client->RegisterPacketHandler(String_Message, clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);
}

//From here on it's functions to add in objects to the world!

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor", DISABLE_COLLISION_RES_LAYER);

	Vector3 floorSize = Vector3(100, 2, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, nullptr, basicShader));
	floor->GetRenderObject()->SetColour(Vector4(0.14f, 0.55f, 0.14f, 1.0f));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, OGLTexture* texture, float inverseMass, string name, uint32_t layer) {
	GameObject* sphere = new GameObject(name, layer);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, texture, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	if (name == "leaves") {
		sphere->GetRenderObject()->SetColour(Vector4(0.11f, 0.34f, 0.05f, 1.0f));
	}

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, OGLTexture* texture, float inverseMass, string name, uint32_t layer) {
	GameObject* cube = new GameObject(name, layer);

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);
	
	
	
	if(name == "buttonA")
		cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, texture, basicShader));
	else
		cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, texture, basicShader));

	if (name == "water") {
		cube->GetRenderObject()->SetColour(Vector4(0.0f, 0.75f, 1.0f, 1.0f));
	}
	if (name == "trunk") {
		cube->GetRenderObject()->SetColour(Vector4(0.34f, 0.18f, 0.10f, 1.0f));
	}
	if (name == "bonus")
		numberofCollectables++;

	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}


GameObject* TutorialGame::AddGooseToWorld(const Vector3& position)
{

	float size			= 1.0f;
	float inverseMass	= 1.0f;

	GooseObject* goose = new GooseObject("goose", GOOSE_LAYER);


	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size,size,size) );
	goose->GetTransform().SetWorldPosition(position);
	goose->SetSpawnPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();

	goose->SetCamera(world->GetMainCamera());

	world->AddGameObject(goose);

	return goose;
}

GameObject* TutorialGame::AddParkKeeperToWorld(const Vector3& position, GooseObject* goose)
{
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	KeeperObject* keeper = new KeeperObject("keeper", (KEEPER_LAYER | DISABLE_OBJECT_OBJECT_COL_LAYER | CLICKABLE_LAYER), goose);

	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	keeper->GetTransform().SetWorldPosition(position);

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();

	

	world->AddGameObject(keeper);

	return keeper;
}

GameObject* TutorialGame::AddCharacterToWorld(const Vector3& position, GooseObject* goose) {
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	auto pos = keeperMesh->GetPositionData();

	Vector3 minVal = pos[0];
	Vector3 maxVal = pos[0];

	for (auto& i : pos) {
		maxVal.y = max(maxVal.y, i.y);
		minVal.y = min(minVal.y, i.y);
	}

	CharacterObject* character = new CharacterObject("character", (KEEPER_LAYER | DISABLE_OBJECT_OBJECT_COL_LAYER), goose);

	float r = rand() / (float)RAND_MAX;


	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	character->GetTransform().SetWorldPosition(position);
	character->SetSpawnPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? charA : charB, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddAppleToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("apple", (APPLE_LAYER | DISABLE_OBJECT_OBJECT_COL_LAYER | CLICKABLE_LAYER));

	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
	apple->GetTransform().SetWorldPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));
	apple->GetRenderObject()->SetColour(Vector4(1.0, 0.2, 0.2, 1.0));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	numberofCollectables++;
	return apple;
}

bool TutorialGame::ButtonAAction() {
	bool success = false;
	for (auto i : menuObjects) {
		if ((*i).GetName() == selectionObject->GetName()) {
			world->RemoveGameObject(i);
			i = nullptr;
			world->SetinGameMode(true);
			inSelectionMode = false;
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
			world->GetMainCamera()->SetToggleControls(true);
			SetCameraBehindGoose();
			success = true;
			useGravity = true;
			physics->UseGravity(useGravity);
		}
		else
		{
			world->RemoveGameObject(i);
			i = nullptr;
		}
	}
	inMainMenuMode = false;
	return success;
}

bool NCL::CSC8503::TutorialGame::ButtonBAction() {
	for (auto i : menuObjects) {
		i->GetTransform().SetWorldPosition(i->GetTransform().GetWorldPosition() + Vector3(0, 100, 0));
	}
	AddCubeToWorld(Vector3(6, 20, 0), Vector3(5, 3, 0), backButtonTex, 0.0f, "buttonBack", MENU_LAYER);
	highscoreMode = true;
	ReadAndUpdateHighscores();
	
	return true;
}

bool NCL::CSC8503::TutorialGame::ButtonCAction()
{
	return false;
}

bool NCL::CSC8503::TutorialGame::ButtonBackAction(GameObject* button) {
	button->GetTransform().SetWorldPosition(Vector3(200, 2000, 2000));
	for (auto i : menuObjects) {
		i->GetTransform().SetWorldPosition(i->GetTransform().GetWorldPosition() - Vector3(0, 100, 0));
	}
	highscoreMode = false;
	return true;
}

void NCL::CSC8503::TutorialGame::ShowObjectInfo(GameObject* object) {
	Vector3 pos = object->GetTransform().GetWorldPosition();
	objectDebugInfo = object->GetName() + " \n" + "Position: " + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z);
	objectInfo = true;
}

void TutorialGame::AddTreeToWorld(const Vector3& position) {
	Vector3 trunkPosition = position;
	Vector3 leavesPosition = position;
	trunkPosition.y = 5.0f;
	leavesPosition.y = 14.0f;
	AddCubeToWorld(trunkPosition, Vector3(2, 10, 2), nullptr, 0.0f, "trunk", DISABLE_COLLISION_RES_LAYER | CLICKABLE_LAYER);
	AddSphereToWorld(leavesPosition, 8.0f, nullptr, 0.0f, "leaves", DISABLE_COLLISION_RES_LAYER | CLICKABLE_LAYER);
}


void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, basicTex, 1.0f);
		}
	}
	
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims, basicTex);
			}
			else {
				AddSphereToWorld(position, sphereRadius, basicTex );
			}
		}
	}
	
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, basicTex, 1.0f);
		}
	}
	
}

void TutorialGame::InitWorldFromFile(const std::string& filename) {
	int nodeSize;
	int gridWidth;
	int gridHeight;
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			char type = 0;
			infile >> type;

			if (type == 'x') {
				int cubeDimension = 100 / gridWidth;
				Vector3 position = Vector3((x * gridWidth) /2, 0, (y * gridHeight) /2);
				ConvertFromNavSpaceToWorldSpace(position);
				position.y = position.y + 5;
				AddCubeToWorld(position, Vector3(cubeDimension, cubeDimension, cubeDimension), fenceTex, 0.0f, "Wall", DISABLE_COLLISION_RES_LAYER);
			}
			if (type == 'w') {
				int cubeDimension = 100 / gridWidth;
				Vector3 position = Vector3((x * gridWidth) / 2, 0, (y * gridHeight) / 2);
				ConvertFromNavSpaceToWorldSpace(position);
				position.y = -1.0f;
				AddCubeToWorld(position, Vector3(cubeDimension, 1.01, cubeDimension), nullptr, 0.0f, "water", DISABLE_COLLISION_RES_LAYER | CLICKABLE_LAYER);
			}
			if (type == 't') {
				Vector3 position = Vector3((x * gridWidth) / 2, 0, (y * gridHeight) / 2);
				ConvertFromNavSpaceToWorldSpace(position);
				AddTreeToWorld(position);

			}
			if (type == 'a') {
				Vector3 position = Vector3((x * gridWidth) / 2, 0, (y * gridHeight) / 2);
				ConvertFromNavSpaceToWorldSpace(position);
				position.y += 1.0f;
				AddAppleToWorld(position);
			}
			if (type == 'h') {
				int cubeDimension = 100 / gridWidth;
				Vector3 position = Vector3((x * gridWidth) / 2, 0, (y * gridHeight) / 2);
				ConvertFromNavSpaceToWorldSpace(position);
				goose->SetHomeBoundary(Vector2(position.x+ cubeDimension-1, position.z - cubeDimension+1 ));
			}
			if (type == 'b') {
				Vector3 bonusDimension = Vector3(0.5, 0.5, 0.5);
				Vector3 position = Vector3((x * gridWidth) / 2, 0, (y * gridHeight) / 2);
				ConvertFromNavSpaceToWorldSpace(position);
				position.y = 0.25;
				AddCubeToWorld(position, bonusDimension, basicTex, 1.0f, "bonus", APPLE_LAYER | CLICKABLE_LAYER);
			}
		}
	}
}


void TutorialGame::InitMainMenu() {
	menuObjects.push_back(AddCubeToWorld(Vector3(-6, 27, 0), Vector3(5, 3, 0), startButtonTex, 0.0f, "buttonA", MENU_LAYER));
	menuObjects.push_back(AddCubeToWorld(Vector3(6, 27, 0), Vector3(5, 3, 0), highscoresButtonTex, 0.0f, "buttonB", MENU_LAYER));
	menuObjects.push_back(AddCubeToWorld(Vector3(-6, 20, 0), Vector3(5, 3, 0), quitButtonTex, 0.0f, "buttonD", MENU_LAYER));
	
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);

	float	invCubeMass = 5;
	int		numLinks	= 25;
	float	maxDistance	= 30;
	float	cubeDistance = 20;

	Vector3 startPos = Vector3(500, 1000, 500);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, basicTex, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::SimpleGJKTest() {
	Vector3 dimensions		= Vector3(5, 5, 5);
	Vector3 floorDimensions = Vector3(100, 2, 100);

	GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, basicTex, 10.0f);
	GameObject* newFloor	= AddCubeToWorld(Vector3(0, 0, 0), floorDimensions, basicTex, 0.0f);

	delete fallingCube->GetBoundingVolume();
	delete newFloor->GetBoundingVolume();

	fallingCube->SetBoundingVolume((CollisionVolume*)new OBBVolume(dimensions));
	newFloor->SetBoundingVolume((CollisionVolume*)new OBBVolume(floorDimensions));

}

void TutorialGame::SetCameraBehindGoose() {
	Quaternion orientationGoose = goose->GetTransform().GetWorldOrientation();
	Vector3 frwdAxis(0, 0, 1);
	frwdAxis = orientationGoose * frwdAxis;
	world->GetMainCamera()->SetPosition(goose->GetTransform().GetWorldPosition() - (frwdAxis * 25));
	Vector3 camPos = world->GetMainCamera()->GetPosition();
	camPos.y += 10.0f;
	world->GetMainCamera()->SetPosition(camPos);
}

bool NCL::CSC8503::TutorialGame::ConvertFromNavSpaceToWorldSpace(Vector3& pos)
{
	pos = pos - Vector3(95, 0, 95);
	return true;
}
