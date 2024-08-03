#pragma once

#include <array>

#include "IO/Filesystem.h"
#include "Graphics/Framebuffer.h"
#include "Types/Math.h"

#include "Camera2D.h"

struct FrameData
{
	slc::Ref<slc::Framebuffer> fbo;

	bool viewportFocused = false, viewportHovered = false;
	slc::Vector2 viewportSize = { 0.0f, 0.0f };
	std::array<slc::Vector2, 2> viewportBounds = {};

	slc::Ref<Camera2D> camera = nullptr;
};