#include "KeeperObject.h"
#include "StateTransition.h"
#include "State.h"
#include "Debug.h"
#include "..//..//Common/Maths.h"


using namespace NCL;
using namespace CSC8503;


KeeperObject::KeeperObject(string name, uint32_t layer, GooseObject* goose) : GameObject(name, layer),totalTime(0.0f), path(nullptr), goose(goose){
	
	stateMachine = new StateMachine();
	navigation = new NavigationGrid("TestGrid1.txt", 100.0f);
	path = new NavigationPath();

	StateFunc StateAFunc = [](void* keeper, void* goose) {
		KeeperObject* keeperData = (KeeperObject*)keeper;
		GooseObject* gooseData = (GooseObject*)goose;
		
		NavigationGrid* grid = keeperData->GetNavigationGrid();
		NavigationPath* path = keeperData->GetNavigationPath();

		bool found = false;
		if (keeperData->GetTime() > 2.0f) {
			found = grid->FindPath(keeperData->GetTransform().GetWorldPosition(), gooseData->GetTransform().GetWorldPosition(), *path);
			if(keeperData->GetTime() != 0.0f)
				keeperData->SetTime(0.0f);
			
		}
		if (found)
			 path->PopWaypoint(keeperData->GetTargetPos());
	};

	StateFunc StateBFunc = [](void* keeper, void* goose) {
	
		KeeperObject* keeperData = (KeeperObject*)keeper;
		GooseObject* gooseData = (GooseObject*)goose;

		NavigationGrid* grid = keeperData->GetNavigationGrid();
		NavigationPath* path = keeperData->GetNavigationPath();


		if (path->IsEmpty()) {
			bool found = false;
			while (!found) {
				int x = rand() % 100 + 1;
				int z = rand() % 100 + 1;
				Vector3 targetLocation = Vector3(x, 0, z);
				found = grid->FindPath(keeperData->GetTransform().GetWorldPosition(), targetLocation, *path);
			}
			path->PopWaypoint(keeperData->GetTargetPos());
		}
	};

	GenericState* stateA = new GenericState(StateAFunc, this, goose);
	GenericState* stateB = new GenericState(StateBFunc, this, goose);

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	typedef bool(*transitionfunc)(void*, void*);
	transitionfunc transitionAFunc = [](void* keeper, void* goose) {
		KeeperObject* keeperData = (KeeperObject*)keeper;
		GooseObject* GooseData = (GooseObject*)goose;
		Vector3 keeperPos = keeperData->GetTransform().GetWorldPosition();
		Vector3 goosePos = GooseData->GetTransform().GetWorldPosition();

		if ((keeperPos - goosePos).Length() < 30.0f) {
			keeperData->GetNavigationPath()->Clear();
			return true;
		}
		return false;
	};
	
	transitionfunc transitionBFunc = [](void* keeper, void* goose) {
		KeeperObject* keeperData = (KeeperObject*)keeper;
		GooseObject* GooseData = (GooseObject*)goose;
		Vector3 keeperPos = keeperData->GetTransform().GetWorldPosition();
		Vector3 goosePos = GooseData->GetTransform().GetWorldPosition();
		

		if ((keeperPos - goosePos).Length() > 40.0f) {
			keeperData->GetNavigationPath()->Clear();
			return true;
			
		}

		return false;
	};
	
	GenericTransition<void*, void*>* transitionA = new GenericTransition<void*, void*>(transitionAFunc, this, goose, stateB, stateA);
	GenericTransition<void*, void*>* transitionB = new GenericTransition<void*, void*>(transitionBFunc, this, goose, stateA, stateB);

	stateMachine->AddTransition(transitionA);
	stateMachine->AddTransition(transitionB);

}

KeeperObject::~KeeperObject(){
	delete navigation;
	delete path;
}

void KeeperObject::Update(float dt) {

	totalTime += dt;
	
	stateMachine->Update();
	UpdatePosition();
	TestCollidedWithGoose();

}
void KeeperObject::UpdatePosition() {
	Vector3 currentPos = GetTransform().GetWorldPosition();
	currentPos.y = 0;

	if ((currentPos - targetPos).Length() < 2) {
		path->PopWaypoint(targetPos);
	}
	
	Vector3 direction = (targetPos - currentPos).Normalised();
	direction.y = 0;
	GetPhysicsObject()->AddForce(direction * 100);

	GetTransform().SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), RadiansToDegrees(atan2(direction.x, direction.z))));

	if (debugInfo) {
		Debug::DrawLine(currentPos, targetPos, Vector4(1, 0.55, 0, 1));
		Debug::DrawLine(currentPos, currentPos + Vector3(0, 40, 0), Vector4(1, 0, 0, 1));
		Debug::DrawLine(targetPos, targetPos + Vector3(0, 40, 0), Vector4(1, 1, 0, 1));
	}
	
}

void KeeperObject::TestCollidedWithGoose() {
	Vector3 goosePos = goose->GetTransform().GetWorldPosition();
	Vector3 pos = GetTransform().GetWorldPosition();
	pos.y = 0.0f;
	goosePos.y = 0.0f;
	float distanceToGoose = (goosePos - pos).Length();

	if (distanceToGoose < 2) {
		goose->SetScareMode(true);
		path->Clear();

	}


}

