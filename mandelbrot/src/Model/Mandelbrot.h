#pragma once

#include <complex>

#include "Types/Math.h"
#include "Graphics/Pixel.h"

using Complex = std::complex<double>;

slc::Pixel GetMandelbrotColour(double x, double y, int width, int height);