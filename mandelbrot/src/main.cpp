#include "Common/EntryPoint.h"

#include "MandelbrotLayer.h"

using namespace slc;

class MandelbrotApplication : public Application
{
public:
	MandelbrotApplication(Impl<MandelbrotAppSpec> spec)
		: Application(std::move(spec))
	{
		PushLayer<MandelbrotLayer>();
	}

	virtual ~MandelbrotApplication()
	{
	}
};

Application* CreateApplication(int argc, char** argv)
{
	Impl<MandelbrotAppSpec> spec = MakeImpl<MandelbrotAppSpec>();
	spec->name = "ExampleApp";
	spec->renderMode = RenderMode::GPU;

	return new MandelbrotApplication(std::move(spec));
}