#include "Camera2D.h"

#include <glm/gtc/matrix_transform.hpp>

#include "IO/KeyCodes.h"

using namespace slc;

SCONSTEXPR float ROTATION_SPEED = 360.0f;

Camera2D::Camera2D(float aspectRatio)
	: mAspectRatio(aspectRatio)
	, mProjection(glm::ortho(-mAspectRatio * mZoomLevel, mAspectRatio* mZoomLevel, -mZoomLevel, mZoomLevel, -1.0f, 1.0f))
	, mView(1.0f)
{
}

void Camera2D::OnUpdate(slc::Timestep ts)
{
	mCurrentTS = ts;
	mTranslationSpeed = 5.0f * mZoomLevel;
}

void Camera2D::OnEvent(Event& e)
{
	e.Dispatch<KeyPressedEvent>(SLC_BIND_EVENT_FUNC(OnKeyPressed));
	e.Dispatch<MouseScrolledEvent>(SLC_BIND_EVENT_FUNC(OnMouseScroll));
}

void Camera2D::SetViewportSize(const Vector2 size)
{
	mAspectRatio = size.x / size.y;
	mProjection = glm::ortho(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel);
}

bool Camera2D::OnKeyPressed(KeyPressedEvent& e)
{
	KeyCode k = e.keyCode;
	if (k != Key::W and k != Key::A and k != Key::S and k != Key::D and k != Key::Q and k != Key::R)
		return false;

	switch (e.keyCode)
	{
	case Key::W:
		mPosition.x += -sin(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		mPosition.y += cos(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		break;
	case Key::A:
		mPosition.x -= cos(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		mPosition.y -= sin(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		break;
	case Key::S:
		mPosition.x -= -sin(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		mPosition.y -= cos(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		break;
	case Key::D:
		mPosition.x += cos(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		mPosition.y += sin(glm::radians(mRotation)) * mTranslationSpeed * mCurrentTS;
		break;
	case Key::Q:
		mRotation += ROTATION_SPEED * mCurrentTS;
		break;
	case Key::R:
		mRotation -= ROTATION_SPEED * mCurrentTS;
		break;
	default:
		break;
	}

	RecalculateViewMatrix();
	return false;
}

bool Camera2D::OnMouseScroll(MouseScrolledEvent& e)
{
	mZoomLevel -= e.yOffset * 0.25f;
	mZoomLevel = std::max(mZoomLevel, 0.25f);

	mProjection = glm::ortho(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel);

	return false;
}

void Camera2D::RecalculateViewMatrix()
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), mPosition) *
		glm::rotate(glm::mat4(1.0f), glm::radians(mRotation), glm::vec3(0, 0, 1));

	mView = glm::inverse(transform);
}