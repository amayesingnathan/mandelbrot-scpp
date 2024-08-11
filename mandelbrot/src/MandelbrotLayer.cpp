#include "MandelbrotLayer.h"

#include "IO/Input.h"
#include "ImGui/Widgets.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderer2D.h"
#include "Types/Timer.h"

using namespace slc;

void MandelbrotLayer::OnAttach()
{
	const auto& appSpec = Application::GetSpec<MandelbrotAppSpec>();

	int width = appSpec.resolution.width;
	int height = appSpec.resolution.height;

	mRenderData.resolution = { width, height };

	FramebufferSpec fbSpec;
	fbSpec.width = width;
	fbSpec.height = height;
	fbSpec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.samples = 1;

	mRenderData.zoomFactor = 1.0f;

	mRenderData.viewportBounds[0] = Vector2{ -2.0f, -2.0f };
	mRenderData.viewportBounds[1] = Vector2{ 2.0f, 2.0f };

	mRenderData.fbo = Ref<Framebuffer>::Create(fbSpec);
	mRenderData.texture = Ref<Texture2D>::Create(width, height);

	mRenderData.pixelData = Grid<Pixel>(width, height);
	mRenderData.jobResults.reserve(width * height);

	mRenderData.vertexArray = Ref<VertexArray>::Create();
	mRenderData.vertexBuffer = Ref<VertexBuffer>::Create(4 * static_cast<uint32_t>(sizeof(Vertex)));
	mRenderData.vertexBuffer->SetLayout({
		{ ShaderDataType::Float3, "iPosition" },
		{ ShaderDataType::Float2, "iResolution" },
		{ ShaderDataType::Float2, "iViewportMin" },
		{ ShaderDataType::Float2, "iViewportMax" },
	});

	mRenderData.vertexArray->AddVertexBuffer(mRenderData.vertexBuffer);

	std::array<uint32_t, INDEX_COUNT> indices = {
		0, 1, 2, 2, 3, 0
	};

	auto indexBuffer = Ref<IndexBuffer>::Create(indices.data(), INDEX_COUNT);
	mRenderData.vertexArray->SetIndexBuffer(indexBuffer);

	mRenderData.shader = Ref<Shader>::Create("resources/shaders/Mandelbrot.glsl");

	if (appSpec.renderMode == RenderMode::CPU)
	{
		RenderMandelbrot();
	}
}

void MandelbrotLayer::OnDetach()
{
}

void MandelbrotLayer::OnUpdate(Timestep ts)
{
}

void MandelbrotLayer::OnRender()
{
	mRenderData.fbo->Bind();

	Renderer::SetClearColor({ 0, 0, 0, 1 });
	Renderer::Clear();

	Renderer2D::BeginState();

	RenderMandelbrot();

	Renderer2D::EndState();

	mRenderData.fbo->Unbind();
}

void MandelbrotLayer::OnOverlayRender()
{
	Widgets::BeginWindow("Test");

	auto viewportSize = Utils::AvailableRegion<Vector2>();
	ImGui::Image((ImTextureID)(uintptr_t)mRenderData.fbo->GetTextureID(), Utils::ToImVec<ImVec2>(viewportSize));

	Widgets::EndWindow();
}

void MandelbrotLayer::OnEvent(Event& e)
{
	e.Dispatch<KeyPressedEvent>(SLC_BIND_EVENT_FUNC(OnKeyPressed));
	e.Dispatch<MouseScrolledEvent>(SLC_BIND_EVENT_FUNC(OnMouseScrolled));
}

bool MandelbrotLayer::OnKeyPressed(KeyPressedEvent& e)
{
	SCONSTEXPR float ScaleFactor = 0.08f;

	auto offset = ScaleFactor / mRenderData.zoomFactor;

	switch (e.keyCode)
	{
	case Key::W:
		mRenderData.viewportBounds[0].y -= offset;
		mRenderData.viewportBounds[1].y -= offset;
		break;
	case Key::A:
		mRenderData.viewportBounds[0].x -= offset;
		mRenderData.viewportBounds[1].x -= offset;
		break;
	case Key::S:
		mRenderData.viewportBounds[0].y += offset;
		mRenderData.viewportBounds[1].y += offset;
		break;
	case Key::D:
		mRenderData.viewportBounds[0].x += offset;
		mRenderData.viewportBounds[1].x += offset;
		break;
	default:
		break;
	}

	return false;
}

bool MandelbrotLayer::OnMouseScrolled(slc::MouseScrolledEvent& e)
{
	SCONSTEXPR Vector2 ViewportBase[2] = {
		{ -2.0f, -2.0f },
		{ 2.0f, 2.0f },
	};

	mRenderData.zoomFactor += e.yOffset * 0.1f * mRenderData.zoomFactor;
	mRenderData.zoomFactor = std::max(mRenderData.zoomFactor, 0.0001f);

	float scale = 1.0f / mRenderData.zoomFactor;
	Vector2 centre = (mRenderData.viewportBounds[1] + mRenderData.viewportBounds[0]) / 2.0f;

	mRenderData.viewportBounds[0] = centre + (ViewportBase[0] * scale);
	mRenderData.viewportBounds[1] = centre + (ViewportBase[1] * scale);

	return false;
}

void MandelbrotLayer::RenderMandelbrot()
{
	auto const& spec = Application::GetSpec<MandelbrotAppSpec>();

	switch (spec.renderMode)
	{
	case RenderMode::CPU:
		RenderMandelbrotCPU();
		Renderer2D::DrawQuad(Vector2{ 0, 0 }, Vector2{ 2, 2 }, mRenderData.texture);
		break;

	case RenderMode::GPU:
		RenderMandelbrotGPU();
		break;
	}
}

void MandelbrotLayer::RenderMandelbrotCPU()
{
	int width = static_cast<int>(mRenderData.resolution.x);
	int height = static_cast<int>(mRenderData.resolution.y);

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			mRenderData.jobResults.emplace_back(mRenderData.workers.Queue(GetMandelbrotColour, i, j, width, height));
		}
	}

	for (auto&& [future, pixel] : std::views::zip(mRenderData.jobResults, mRenderData.pixelData))
	{
		pixel = future.get();
	}

	mRenderData.texture->SetData(mRenderData.pixelData.Data(), width * height * sizeof(Pixel));
	mRenderData.jobResults.clear();

}

void MandelbrotLayer::RenderMandelbrotGPU()
{
	SCONSTEXPR Vector4 QuadVertexPositions[4] =
	{
		{ -1.0f, -1.0f, 0.0f, 1.0f },
		{  1.0f, -1.0f, 0.0f, 1.0f },
		{  1.0f,  1.0f, 0.0f, 1.0f },
		{ -1.0f,  1.0f, 0.0f, 1.0f }
	};

	SCONSTEXPR Matrix4 Centre = Matrix4(1.0f);

	SCONSTEXPR uint32_t BufferSize = sizeof(Vertex) * VERTEX_COUNT;

	for (auto&& [vertexData, position] : std::views::zip(mRenderData.vertexData, QuadVertexPositions))
	{
		vertexData.position = position;
		vertexData.resolution = mRenderData.resolution;
		vertexData.viewportMin = mRenderData.viewportBounds[0];
		vertexData.viewportMax = mRenderData.viewportBounds[1];
	}

	mRenderData.vertexBuffer->SetData(mRenderData.vertexData.data(), BufferSize);
	mRenderData.shader->Bind();
	Renderer::DrawIndexed(mRenderData.vertexArray);
}
