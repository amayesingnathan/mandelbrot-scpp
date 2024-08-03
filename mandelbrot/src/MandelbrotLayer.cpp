#include "MandelbrotLayer.h"

#include "ImGui/Widgets.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderer2D.h"

using namespace slc;

void MandelbrotLayer::OnAttach()
{
	const auto& appSpec = Application::GetSpec();

	FramebufferSpec fbSpec;
	fbSpec.width = appSpec.resolution.width;
	fbSpec.height = appSpec.resolution.height;
	fbSpec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.samples = 1;

	mFrame.fbo = Ref<Framebuffer>::Create(fbSpec);

	mFrame.viewportSize = { fbSpec.width, fbSpec.height };

	float aspectRatio = static_cast<float>(fbSpec.width) / static_cast<float>(fbSpec.height);
	mFrame.camera = Ref<Camera2D>::Create(aspectRatio);
}

void MandelbrotLayer::OnDetach()
{
}

void MandelbrotLayer::OnUpdate(Timestep ts)
{
	if (FramebufferSpec spec = mFrame.fbo->GetSpecification();
		mFrame.viewportSize.x > 0.0f && mFrame.viewportSize.y > 0.0f && // zero sized framebuffer is invalid
		(spec.width != mFrame.viewportSize.x || spec.height != mFrame.viewportSize.y))
	{
		mFrame.fbo->Resize(static_cast<uint32_t>(mFrame.viewportSize.x), static_cast<uint32_t>(mFrame.viewportSize.y));

		mFrame.camera->SetViewportSize(mFrame.viewportSize);
	}

	mFrame.camera->OnUpdate(ts);
}

void MandelbrotLayer::OnRender()
{
	mFrame.fbo->Bind();

	Renderer::SetClearColor({ 1, 0, 1, 1 });
	Renderer::Clear();

	Renderer2D::BeginState(mFrame.camera->GetViewProjection());
	Renderer2D::DrawQuad(Vector2{ 0, 0 }, Vector2{ 5, 5 }, Vector4{ 1, 0, 0, 1 });
	Renderer2D::EndState();

	mFrame.fbo->Unbind();
}

void MandelbrotLayer::OnOverlayRender()
{
	Widgets::BeginWindow("Test");

	mFrame.viewportSize = Utils::AvailableRegion<glm::vec2>();
	ImGui::Image((ImTextureID)(uintptr_t)mFrame.fbo->GetTextureID(), Utils::ToImVec<ImVec2>(mFrame.viewportSize));

	Widgets::EndWindow();
}
