#include "MandelbrotLayer.h"

#include "ImGui/Widgets.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderer2D.h"

using namespace slc;

void MandelbrotLayer::OnAttach()
{
	const auto& appSpec = Application::GetSpec();

	int width = appSpec.resolution.width;
	int height = appSpec.resolution.height;

	FramebufferSpec fbSpec;
	fbSpec.width = width;
	fbSpec.height = height;
	fbSpec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.samples = 1;

	mFrame.fbo = Ref<Framebuffer>::Create(fbSpec);
	mFrame.texture = Ref<Texture2D>::Create(width, height);

	mFrame.viewportSize = { width, height };

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	mFrame.camera = Ref<Camera2D>::Create(aspectRatio);

	mFrame.pixelData = Grid<Pixel>(width, height);

	mFrame.jobResults.reserve(width * height);

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			mFrame.jobResults.emplace_back(mFrame.workers.Queue(GetMandelbrotColour, i, j, width, height));
		}
	}

	for (auto&& [future, pixel] : std::views::zip(mFrame.jobResults, mFrame.pixelData))
	{
		pixel = future.get();
	}

	mFrame.texture->SetData(mFrame.pixelData.Data(), width * height * sizeof(Pixel));
	mFrame.jobResults.clear();
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

	Renderer::SetClearColor({ 0, 0, 0, 1 });
	Renderer::Clear();

	Renderer2D::BeginState(mFrame.camera->GetViewProjection());

	Renderer2D::DrawQuad(Vector2{ 0, 0 }, Vector2{ 3, 2 }, mFrame.texture);

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
