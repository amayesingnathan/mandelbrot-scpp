#include "MandelbrotLayer.h"

#include "ImGui/Widgets.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderer2D.h"
#include "Types/Timer.h"

using namespace slc;

SCONSTEXPR float ScaleFactor = 0.05f;

void MandelbrotLayer::OnAttach()
{
	const auto& appSpec = Application::GetSpec<MandelbrotAppSpec>();

	mRenderData.width = appSpec.resolution.width;
	mRenderData.height = appSpec.resolution.height;

	FramebufferSpec fbSpec;
	fbSpec.width = mRenderData.width;
	fbSpec.height = mRenderData.height;
	fbSpec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.samples = 1;

	mRenderData.zoomLevel = 1.0f;

	mRenderData.viewportBounds[0] = Vector2{ -3.0f, -2.0f };
	mRenderData.viewportBounds[1] = Vector2{ 2.0f, 2.0f };

	mRenderData.fbo = Ref<Framebuffer>::Create(fbSpec);
	mRenderData.texture = Ref<Texture2D>::Create(mRenderData.width, mRenderData.height);

	mRenderData.pixelData = Grid<Pixel>(mRenderData.width, mRenderData.height);
	mRenderData.jobResults.reserve(mRenderData.width * mRenderData.height);

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
	auto offset = 3 * ScaleFactor * mRenderData.zoomLevel;

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
	float offset = e.yOffset * ScaleFactor;

	mRenderData.zoomLevel -= offset;
	mRenderData.zoomLevel = std::max(mRenderData.zoomLevel, ScaleFactor);

	mRenderData.viewportBounds[0] += offset; 
	mRenderData.viewportBounds[1] -= offset; 

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
	for (int j = 0; j < mRenderData.height; j++)
	{
		for (int i = 0; i < mRenderData.width; i++)
		{
			mRenderData.jobResults.emplace_back(mRenderData.workers.Queue(GetMandelbrotColour, i, j, mRenderData.width, mRenderData.height));
		}
	}

	for (auto&& [future, pixel] : std::views::zip(mRenderData.jobResults, mRenderData.pixelData))
	{
		pixel = future.get();
	}

	mRenderData.texture->SetData(mRenderData.pixelData.Data(), mRenderData.width * mRenderData.height * sizeof(Pixel));
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

	Vector2 resolution(mRenderData.width, mRenderData.height);

	for (auto&& [vertexData, position] : std::views::zip(mRenderData.vertexData, QuadVertexPositions))
	{
		vertexData.position = position;
		vertexData.resolution = resolution;
		vertexData.viewportMin = mRenderData.viewportBounds[0];
		vertexData.viewportMax = mRenderData.viewportBounds[1];
	}

	mRenderData.vertexBuffer->SetData(mRenderData.vertexData.data(), BufferSize);
	mRenderData.shader->Bind();
	Renderer::DrawIndexed(mRenderData.vertexArray);
}
