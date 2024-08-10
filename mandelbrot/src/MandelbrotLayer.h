#pragma once

#include "Common/Application.h"

#include "Model/Mandelbrot.h"
#include "Rendering/RenderData.h"

enum class RenderMode
{
	CPU,
	GPU
};

struct MandelbrotAppSpec : public slc::ApplicationSpecification
{
	RenderMode renderMode = RenderMode::GPU;
};

class MandelbrotLayer : public slc::ApplicationLayer
{
public:
	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(slc::Timestep ts) override;
	void OnRender() override;
	void OnOverlayRender() override;

	void OnEvent(slc::Event& e) override {}
	LISTENING_EVENTS()

private:
	void RenderMandelbrot();
	void RenderMandelbrotCPU();
	void RenderMandelbrotGPU();

private:
	RenderData mRenderData;
};