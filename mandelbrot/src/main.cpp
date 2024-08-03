#include "Common/EntryPoint.h"

#include "MandelbrotLayer.h"

using namespace slc;

class MandelbrotApplication : public Application
{
public:
	MandelbrotApplication(Impl<ApplicationSpecification> spec)
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
	Impl<ApplicationSpecification> spec = MakeImpl<ApplicationSpecification>();
	spec->name = "ExampleApp";

	return new MandelbrotApplication(std::move(spec));
}