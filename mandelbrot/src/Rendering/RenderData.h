#pragma once

#include "Graphics/VertexArray.h"
#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Pixel.h"

#include "Collections/Grid.h"

#include "Threading/ThreadPool.h"

struct Vertex
{
	slc::Vector3 position;
	slc::Vector2 resolution;
	slc::Vector2 viewportMin;
	slc::Vector2 viewportMax;
};

SCONSTEXPR uint32_t VERTEX_COUNT = 4;
SCONSTEXPR uint32_t INDEX_COUNT  = 6;

struct RenderData
{
	slc::Ref<slc::VertexArray> vertexArray;
	slc::Ref<slc::VertexBuffer> vertexBuffer;
	slc::Ref<slc::Shader> shader;

	std::array<Vertex, VERTEX_COUNT> vertexData;

	float zoomLevel = 1.0f;
	std::array<slc::Vector2, 2> viewportBounds;

	int width, height;

	slc::Ref<slc::Framebuffer> fbo;
	slc::Ref<slc::Texture2D> texture;

	slc::Grid<slc::Pixel> pixelData;

	slc::ThreadPool workers;
	std::vector<std::future<slc::Pixel>> jobResults;
};