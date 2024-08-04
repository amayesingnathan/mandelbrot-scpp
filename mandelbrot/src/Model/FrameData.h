#pragma once

#include <array>

#include "Collections/Grid.h"
#include "IO/Filesystem.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Texture.h"
#include "Types/Math.h"

#include "Threading/JobPool.h"

#include "Camera2D.h"
#include "Mandelbrot.h"

struct FrameData
{
	slc::Ref<slc::Framebuffer> fbo;
	slc::Ref<slc::Texture2D> texture;

	slc::Grid<Pixel> pixelData;

	slc::Vector2 viewportSize = { 0.0f, 0.0f };

	slc::Ref<Camera2D> camera = nullptr;

	JobPool workers;
	std::vector<std::future<Pixel>> jobResults;
};