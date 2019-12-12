#include "CharacterObject.h"
#include "Debug.h"
#include "..//..//Common/Maths.h"


using namespace NCL;
using namespace CSC8503;

CharacterObject::CharacterObject(string name, uint32_t layer, GooseObject* goose) : GameObject(name, layer), goose(goose), chaseGoose(false), firstChase(true), timer(0) {
	grid = new NavigationGrid("TestGrid1.txt", 100.0f);
	path = new NavigationPath();
}

CharacterObject::~CharacterObject() {
	delete grid;
	delete path;
	goose = nullptr;
}

void CharacterObject::Update(float dt) {
	
	TestToChaseGoose();

	if(chaseGoose)
		timer += dt;

	if (chaseGoose == true && timer > 2 || (firstChase && chaseGoose)) {
		BuildPathToGoose();
		timer = 0.0f;
		firstChase = false;
		
	}

	
	if (chaseGoose) {
		
		UpdatePosition();
		TestCollideWithGoose();
	}
}



void NCL::CSC8503::CharacterObject::TestToChaseGoose() {
	Vector3 goosePos = goose->GetTransform().GetWorldPosition();

	float distanceToGoose = (goosePos - GetTransform().GetWorldPosition()).Length();

	if (distanceToGoose < 30 && goose->IsHoldingBonus()) {
		chaseGoose = true;
	}
	else {
		chaseGoose = false;
	}
}

void CharacterObject::BuildPathToGoose() {
	bool found = false;
	path->Clear();
	while (!found) {
		Vector3 targetLocation = goose->GetTransform().GetWorldPosition();
		found = grid->FindPath(GetTransform().GetWorldPosition(), targetLocation, *path);
	}

	path->PopWaypoint(targetPos);
}

void CharacterObject::UpdatePosition() {
	

	Vector3 currentPos = GetTransform().GetWorldPosition();
	currentPos.y = 0;

	if ((currentPos - targetPos).Length() < 1) {
		if (path->IsEmpty())
			return;

		path->PopWaypoint(targetPos);
	}

	Vector3 direction = (targetPos - currentPos).Normalised();
	direction.y = 0;
	GetPhysicsObject()->AddForce(direction * 100);

	GetTransform().SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), RadiansToDegrees(atan2(direction.x, direction.z))));


	if(debugInfo)
		path->DrawLines();
}

void CharacterObject::TestCollideWithGoose() {
	Vector3 goosePos = goose->GetTransform().GetWorldPosition();
	Vector3 pos = GetTransform().GetWorldPosition();
	pos.y = 0.0f;
	goosePos.y = 0.0f;
	float distanceToGoose = (goosePos - pos).Length();

	if (distanceToGoose < 2) {
		goose->SetCaught(true);
		chaseGoose = false;
		GetTransform().SetWorldPosition(spawnPos);
		path->Clear();
		
		firstChase = true;
	}
	

}
