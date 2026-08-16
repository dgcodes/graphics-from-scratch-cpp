#ifndef gambetta_graphics_helpers_h
#define gambetta_graphics_helpers_h

#define FLOAT_MAX std::numeric_limits<float>::max()

//#error "YES, THIS HEADER IS BEING INCLUDED"

#include "raylib.h"
#include <utility>
#include <iostream>
#include <limits>
#include <vector>
#include <array>
#include <cassert>

void RayTracePractice();
void RasterizationPractice();

struct CanvasPoint
{
	int x;
	int y;
};

//struct FloatPoint
//{
//	float x;
//	float y;
//};

// interpolate "d = f(i)"
// simple linear 2-variable interpolation
// note that we return floats because we interpolate along 
// the integral pixel values (inputs)
// since slope is calculated as change in y's over x's here
// it "works better" for a more horizontal line (where change in
// y per x is more gradual)
constexpr std::vector<float> LinearInterpolate_2d(CanvasPoint P0, CanvasPoint P1)
{
	assert(P0.x <= P1.x && "P1 must come before P2 in i");

	std::vector<float> interp_vals;
	if (P0.x == P1.x) // Edge case of P1 = P2
	{
		interp_vals.push_back(P0.y);
		return interp_vals;
	}

	float a{ static_cast<float>(P1.y - P0.y) / static_cast<float>(P1.x - P0.x) };
	float d = static_cast<float>(P0.y);
	for (int i = P0.x; i <= P1.x; ++i)
	{
		interp_vals.push_back(d);
		d += a;
	}
	return interp_vals;
}


struct Sphere
{
	Vector3 center{};
	float radius{};
	Color color{};
	float specular_exponent{ -1 }; // -1 means not shiny
	float reflective{ 0 };
};

enum class LightType
{
	ambient,
	directional,
	point,
	unknown
};

struct Light
{
	LightType type{ LightType::unknown };
	float intensity{ 0.0f };
	Vector3 location{};  // only valid for point-type
	Vector3 direction{}; // only valid for directional-type
};

constexpr float dot(Vector3 a, Vector3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr unsigned char ModulateColorByIntensity(unsigned char base_color_val, float intensity_amplification)
{
	float raw_amplified_val{ static_cast<float>(base_color_val) * intensity_amplification };
	return raw_amplified_val > 255 ? 255 : static_cast<unsigned char>(raw_amplified_val);
}

constexpr Vector3 RotateCameraCCW_xzPlane(Vector3 v, float alpha) // angle alpha in degrees
{
	float radians = alpha * PI / 180.0f;

	float c = cos(radians);
	float s = sin(radians);

	return {
		c * v.x - s * v.z,
		v.y,
		s * v.x + c * v.z
	};
}

#endif
