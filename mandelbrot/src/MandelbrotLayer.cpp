#include "MandelbrotLayer.h"

#include "ImGui/Widgets.h"
#include "Graphics/Renderer.h"
#include "Graphics/Renderer2D.h"
#include "Types/Timer.h"

using namespace slc;

void MandelbrotLayer::OnAttach()
{
	const auto& appSpec = Application::GetSpec();

	mRenderData.width = appSpec.resolution.width;
	mRenderData.height = appSpec.resolution.height;

	FramebufferSpec fbSpec;
	fbSpec.width = mRenderData.width;
	fbSpec.height = mRenderData.height;
	fbSpec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fbSpec.samples = 1;

	mRenderData.fbo = Ref<Framebuffer>::Create(fbSpec);
	mRenderData.texture = Ref<Texture2D>::Create(mRenderData.width, mRenderData.height);

	float aspectRatio = static_cast<float>(mRenderData.width) / static_cast<float>(mRenderData.height);
	mRenderData.camera = Ref<Camera2D>::Create(aspectRatio);

	mRenderData.pixelData = Grid<Pixel>(mRenderData.width, mRenderData.height);
	mRenderData.jobResults.reserve(mRenderData.width * mRenderData.height);

	mRenderData.vertexArray = Ref<VertexArray>::Create();
	mRenderData.vertexBuffer = Ref<VertexBuffer>::Create(4 * static_cast<uint32_t>(sizeof(Vertex)));
	mRenderData.vertexBuffer->SetLayout({
		{ ShaderDataType::Float3, "iPosition" },
		{ ShaderDataType::Float2, "iResolution "},
	});

	mRenderData.vertexArray->AddVertexBuffer(mRenderData.vertexBuffer);

	std::array<uint32_t, INDEX_COUNT> indices = {
		0, 1, 2, 2, 3, 0
	};

	auto indexBuffer = Ref<IndexBuffer>::Create(indices.data(), INDEX_COUNT);
	mRenderData.vertexArray->SetIndexBuffer(indexBuffer);

	mRenderData.shader = Ref<Shader>::Create("resources/shaders/Mandelbrot.glsl");
}

void MandelbrotLayer::OnDetach()
{
}

void MandelbrotLayer::OnUpdate(Timestep ts)
{
	mRenderData.camera->OnUpdate(ts);
}

void MandelbrotLayer::OnRender()
{
	mRenderData.fbo->Bind();

	Renderer::SetClearColor({ 0, 0, 0, 1 });
	Renderer::Clear();

	Renderer2D::BeginState(mRenderData.camera->GetViewProjection());

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

void MandelbrotLayer::RenderMandelbrot()
{
	auto const& spec = Application::GetSpec<MandelbrotAppSpec>();

	switch (spec.renderMode)
	{
	case RenderMode::CPU:
		RenderMandelbrotCPU();
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

	Renderer2D::DrawQuad(Vector2{ 0, 0 }, Vector2{ 3, 2 }, mRenderData.texture);
}

void MandelbrotLayer::RenderMandelbrotGPU()
{
	SCONSTEXPR Vector4 QuadVertexPositions[4] =
	{
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{  0.5f, -0.5f, 0.0f, 1.0f },
		{  0.5f,  0.5f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, 0.0f, 1.0f }
	};

	SCONSTEXPR Matrix4 Centre = Matrix4(1.0f);

	SCONSTEXPR uint32_t BufferSize = sizeof(Vertex) * VERTEX_COUNT;

	Vector2 resolution(mRenderData.width, mRenderData.height);

	for (auto&& [vertexData, position] : std::views::zip(mRenderData.vertexData, QuadVertexPositions))
	{
		vertexData.position = position;
		vertexData.resolution = resolution;
	}

	mRenderData.vertexBuffer->SetData(mRenderData.vertexData.data(), BufferSize);
	mRenderData.shader->Bind();
	Renderer::DrawIndexed(mRenderData.vertexArray);
}
