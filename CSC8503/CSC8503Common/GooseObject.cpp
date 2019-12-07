#include "GooseObject.h"
#include "../../Common/Window.h"
#include "../../Common/Maths.h"

using namespace NCL;
using namespace CSC8503;

GooseObject::GooseObject(string objectName, uint32_t layer, GameWorld* world) : GameObject(objectName, layer), prevPos(Vector3(0,0,0)), currentPos(Vector3(0,0,0)), camClampToGoose(Vector3(0,20,50)), world(world) {}


GooseObject::~GooseObject() {
	delete world;
}

void GooseObject::Update() {
	prevPos = currentPos;
	currentPos = GetTransform().GetWorldPosition();
	//UpdateCameraPos();
	GooseKeyboardListener(world);
	GooseMouseListener(world);
}

void GooseObject::UpdateCameraPos() {
	float gooseToCamDis = (cameraPos - currentPos).LengthSquared();
	float gooseToClampDis = camClampToGoose.LengthSquared();

	if (gooseToCamDis > gooseToClampDis)
		world->GetMainCamera()->SetPosition(currentPos + camClampToGoose);
}

void GooseObject::GooseKeyboardListener(GameWorld* world) {

	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); 
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 10.0f, 0), rightAxis);

	Vector3 relativePosition = currentPos - cameraPos;

	GetTransform().SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0, 1.0f, 0.0f), RadiansToDegrees(atan2(relativePosition.x, relativePosition.z))));


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		GetPhysicsObject()->AddForce(-rightAxis * 10.0f);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		GetPhysicsObject()->AddForce(rightAxis * 10.0f);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		GetPhysicsObject()->AddForce(-fwdAxis);
	}
}

void GooseObject::GooseMouseListener(GameWorld* world) {
	
	
	
	Vector2 mousePos = Window::GetMouse()->GetRelativePosition();

	cameraPos = (Matrix4::Rotation(-mousePos.x * 2.5f, Vector3(0, 1, 0)) * (cameraPos - currentPos) + currentPos);

	cameraPos += currentPos - prevPos;

	Matrix4 temp = Matrix4::BuildViewMatrix(cameraPos, GetTransform().GetWorldPosition() , Vector3(0, 1, 0));

	Matrix4 modelMat = temp.Inverse();

	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	world->GetMainCamera()->SetPosition(cameraPos);
	world->GetMainCamera()->SetPitch(angles.x);
	world->GetMainCamera()->SetYaw(angles.y);
}

