#pragma once

#include "Common/Ref.h"
#include "Events/IEventListener.h"
#include "Types/Math.h"
#include "Types/Timestep.h"

class Camera2D : public slc::IEventListener, public slc::RefCounted
{
public:
	Camera2D() = default;
	Camera2D(float aspectRation);

	void OnUpdate(slc::Timestep ts);

	void OnEvent(slc::Event& e) override;
	LISTENING_EVENTS(slc::EventType::KeyPressed, slc::EventType::MouseScrolled);

	slc::Matrix4 GetViewProjection() const { return mProjection * mView; }

	void SetViewportSize(const slc::Vector2 size);

private:
	bool OnKeyPressed(slc::KeyPressedEvent& e);
	bool OnMouseScroll(slc::MouseScrolledEvent& e);

	void RecalculateViewMatrix();

private:
	// Control Data
	float mAspectRatio;
	float mZoomLevel = 1.0f;

	slc::Timestep mCurrentTS;

	float mTranslationSpeed = 5.0f;

	// Camera Data
	slc::Matrix4 mProjection = slc::Matrix4(1.0f);
	slc::Matrix4 mView;

	slc::Vector3 mPosition = { 0.0f, 0.0f, 0.0f };
	float mRotation = 0.0f;
};