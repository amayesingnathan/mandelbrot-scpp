#include "Mandelbrot.h"

#include "Common/Base.h"

SCONSTEXPR int ITERATIONS = 1000;
SCONSTEXPR double LIMIT = 16;

SCONSTEXPR double X_MIN = -2.0;
SCONSTEXPR double X_MAX = 2.0;
SCONSTEXPR double Y_MIN = -2.0;
SCONSTEXPR double Y_MAX = 2.0;

static int IsInMandelbrot(Complex c)
{
	Complex z{};

	for (int i = 0; i < ITERATIONS; i++)
	{
		z = std::pow(z, 2) + c;

		if (std::norm(z) > LIMIT)
			return i;
	}

	return 0;
}

Pixel GetMandelbrotColour(double x, double y, int width, int height)
{
	double x_ratio = x / width;
	double y_ratio = y / height;

	double mandelbrot_x = std::lerp(X_MIN, X_MAX, x_ratio);
	double mandelbrot_y = std::lerp(Y_MIN, X_MAX, y_ratio);

	Complex c{ mandelbrot_x, mandelbrot_y };

	int iters = IsInMandelbrot(c);
	if (iters == 0)
		return Pixel{ 0, 0, 0, 255 };

	uint8_t colour = ((iters * 3) % 255);

	return Pixel{ colour, colour, colour, 255 };
}
