/* Author: Michael Arnold
   Last Edited: 10 / 12 / 19
*/
#include "GooseObject.h"
#include "..\\..\\Common\Window.h"
#include "../../Common/Maths.h"
#include "Debug.h"
#include "..\\GameTech\TutorialGame.h"
#include "Layers.h"


using namespace NCL;
using namespace CSC8503;

GooseObject::GooseObject(string name, uint32_t layer) : GameObject(name, layer), camera(nullptr), holdingObject(nullptr), game(nullptr), scaredMode(false), isHoldingBonus(false),
caught(false), timer(0.0), generatedScaredPath(false), collisionWater(false){
	grid = new NavigationGrid("TestGrid1.txt", 100.0f);
	path = new NavigationPath();
}

GooseObject::~GooseObject() {
	delete camera;
}

void GooseObject::Update(float dt) {
	prevFramePos = currentFramePos;
	currentFramePos = GetTransform().GetWorldPosition();
	if (!scaredMode) {
		MouseListener();
		KeyboardListener();
	}
	else {
		if (!generatedScaredPath) {
			Scared();

			this->SetLayer(layer &= ~DISABLE_OBJECT_OBJECT_COL_LAYER);
			uint32_t tempLayer = holdingObject->GetLayer();
			holdingObject->SetLayer(tempLayer &= ~DISABLE_OBJECT_OBJECT_COL_LAYER);
			holdingObject->GetTransform().SetWorldPosition(holdingObject->GetTransform().GetWorldPosition() + Vector3(2,0,2));
			isHoldingBonus = false;
			holdingObject = nullptr;
			generatedScaredPath = true;
		}
		TransverseScaredPath();
		timer += dt;
		
		
		

	}
	if (timer > 2) {
		scaredMode = false;
		generatedScaredPath = false;
		timer = 0.0;
		
	}
	if (caught) {
		GetTransform().SetWorldPosition(spawnPos);
		caught = false;
		this->SetLayer(layer &= ~DISABLE_OBJECT_OBJECT_COL_LAYER);
		uint32_t tempLayer = holdingObject->GetLayer();
		holdingObject->SetLayer(tempLayer &= ~DISABLE_OBJECT_OBJECT_COL_LAYER);
		isHoldingBonus = false;
		holdingObject = nullptr;
	}

	

	if (holdingObject) {
		holdingObject->GetTransform().SetWorldPosition(this->GetTransform().GetWorldPosition());
		if (holdingObject->GetTransform().GetWorldPosition().x < homeBoundary.x && holdingObject->GetTransform().GetWorldPosition().z > homeBoundary.y) {
			game->DeleteObject(holdingObject);
			holdingObject = nullptr;
			this->SetLayer(layer &= ~DISABLE_OBJECT_OBJECT_COL_LAYER);
			if (isHoldingBonus)
				isHoldingBonus = false;
		}

	}
	
	if (debugInfo) {

		Debug::DrawLine(Vector3(homeBoundary.x, 0, homeBoundary.y), Vector3(homeBoundary.x, 30, homeBoundary.y), Vector4(1.0f, 0.0f, 1.0f, 1.0f));
	}

	
}

void NCL::CSC8503::GooseObject::OnCollisionBegin(GameObject* otherObject) {

	if (otherObject->GetName() == "apple" || otherObject->GetName() == "bonus") {
		holdingObject = otherObject;
		otherObject->SetLayer(otherObject->GetLayer() | DISABLE_OBJECT_OBJECT_COL_LAYER);
		this->SetLayer(DISABLE_OBJECT_OBJECT_COL_LAYER);
		if (otherObject->GetName() == "bonus")
			isHoldingBonus = true;
	}
	if (otherObject->GetName() == "water") {
		collisionWater = true;
	}
	
	
		
	
}

void NCL::CSC8503::GooseObject::OnCollisionEnd(GameObject* otherObject)
{
	if (otherObject->GetName() == "water") {
		collisionWater = false;
	}
}

void GooseObject::MouseListener() {
	Vector2 mousePos = Window::GetMouse()->GetRelativePosition();
	Vector3 newCameraPos = (Matrix4::Rotation(-mousePos.x * 2.5f, Vector3(0, 1, 0)) * (camera->GetPosition() - currentFramePos) + currentFramePos);
	newCameraPos += currentFramePos - prevFramePos;

	Matrix4 temp = Matrix4::BuildViewMatrix(newCameraPos, currentFramePos, Vector3(0.0f, 1.0f, 0.0f));
	Matrix4 modelMat = temp.Inverse();



	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	camera->SetPosition(newCameraPos);
	camera->SetPitch(angles.x);
	camera->SetYaw(angles.y);

	
}

void GooseObject::KeyboardListener(){

	Matrix4 view = camera->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); // View is inverse of model!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0.0f, 10.0f, 0.0f), rightAxis);
	Vector3 relativePos = currentFramePos - camera->GetPosition();
	GetTransform().SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), RadiansToDegrees(atan2(relativePos.x, relativePos.z))));

	float forceWater = 2.0f;
	float force = 10.0f;
	if (collisionWater)
		force = forceWater;


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT) || Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		GetPhysicsObject()->AddForce(-rightAxis * force * 3.0f);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT) || Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		GetPhysicsObject()->AddForce(rightAxis * force * 3.0f);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP) || Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN) || Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		GetPhysicsObject()->AddForce(-fwdAxis * force);
	}
	
}

void GooseObject::Scared() {
	bool found = false;
	
	while (!found) {
		int x = rand() % 200 + 1;
		int z = rand() % 200 + 1;
		Vector3 targetLocation = Vector3(x - 100, 0, z - 100);
		found = grid->FindPath(GetTransform().GetWorldPosition(), targetLocation, *path);
	}
	
	path->PopWaypoint(targetPos);


}

void GooseObject::TransverseScaredPath() {
	Vector3 currentPos = GetTransform().GetWorldPosition();
	currentPos.y = 0;

	if ((currentPos - targetPos).Length() < 2) {
		path->PopWaypoint(targetPos);
	}

	Vector3 direction = (targetPos - currentPos).Normalised();
	direction.y = 0;
	GetPhysicsObject()->AddForce(direction * 100);

	if (debugInfo) {
		Debug::DrawLine(currentPos, targetPos, Vector4(1, 0.55, 0, 1));
		Debug::DrawLine(currentPos, currentPos + Vector3(0, 40, 0), Vector4(1, 0, 0, 1));
		Debug::DrawLine(targetPos, targetPos + Vector3(0, 40, 0), Vector4(1, 1, 0, 1));
	}
	
}

