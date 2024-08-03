#pragma once

#include "Common/Application.h"

#include "Model/FrameData.h"

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
	FrameData mFrame;
};