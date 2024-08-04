#pragma once

#include <complex>

#include "Types/Math.h"

using Complex = std::complex<double>;

struct Pixel
{
	uint8_t r, g, b, a;
};

Pixel GetMandelbrotColour(double x, double y, int width, int height);