#ifndef gambetta_graphics_helpers_h
#define gambetta_graphics_helpers_h

//#error "YES, THIS HEADER IS BEING INCLUDED"

#include "raylib.h"
#include <utility>
#include <iostream>
#include <limits>
#include <vector>
#include <array>

#define FLOAT_MAX std::numeric_limits<float>::max()

void RayTracePractice();

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
