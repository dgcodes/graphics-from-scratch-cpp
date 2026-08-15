/*
// David Giles August 2026
Implementing https://gabrielgambetta.com/computer-graphics-from-scratch/02-basic-raytracing.html
in C++

Began project with ...
	Raylib example file.
	This is an example main file for a simple raylib project.
	Use this as a starting point or replace it with your code.

	by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/
*/

#include "raylib.h"
#include <utility>
#include <iostream>
#include <limits>
#include <vector>
#include <array>

//#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define FLOAT_MAX std::numeric_limits<float>::max()

struct Sphere
{
	Vector3 center{};
	float radius{};
	Color color{};
	float specular_exponent{-1}; // -1 means not shiny
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

namespace Scene
{
	std::vector<Sphere> spheres{
		{ { 0.0f, -1.0f,  3.0f},	1.0f,		RED,	500.0f,		0.2f	},
		{ { 2.0f,  0.0f,  4.0f},	1.0f,		BLUE,	500.0f,		0.3f	},
		{ {-2.0f,  0.0f,  4.0f},	1.0f,		GREEN,	10.0f,		0.4f	},
		{ {0.0f,  -5001.0f,  0.0f}, 5000.0f,	YELLOW,	1000.0f,	0.5f	},
	};

	constexpr std::array<Light, 3> lights{ {
		{ LightType::ambient, 0.2f },
		{ LightType::point, 0.6f, {2.0f, 1.0f, 0.0f} },
		{ LightType::directional, 0.2f, {0.0f, 0.0f, 0.0f}, { 1.0f, 4.0f, 4.0f } },
	} };

	Color background_color{ WHITE };
};

constexpr float dot(Vector3 a, Vector3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* TODO :
ReflectRay(R, N) {
	return 2 * N * dot(N, R) - R;
}*/

// R defined as direction from P towards light source
// N is normal of surface at point P
// formula derivation at https://gabrielgambetta.com/computer-graphics-from-scratch/03-light.html
Vector3 ReflectRay(Vector3 R, Vector3 N) // R defined as pointing away from the N, and result will be as well
{
	float r_dot_n{ dot(R,N) };
	return { 2 * r_dot_n * N.x - R.x, 2 * r_dot_n * N.y - R.y, 2 * r_dot_n * N.z - R.z };
}

constexpr unsigned char ModulateColorByIntensity(unsigned char base_color_val, float intensity_amplification)
{
	float raw_amplified_val{ static_cast<float>(base_color_val) * intensity_amplification };
	return raw_amplified_val > 255 ? 255 : static_cast<unsigned char>(raw_amplified_val);
}

constexpr std::pair<float,float> IntersectRaySphere(Vector3 O, Vector3 D, Sphere sphere)
{
	Vector3 CO = { O.x - sphere.center.x, O.y - sphere.center.y, O.z - sphere.center.z };   // O - sphere.center

	float a = dot(D, D);
	float b = 2 * dot(CO, D);
	float c = dot(CO, CO) - sphere.radius * sphere.radius;

	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0.0f) return { FLOAT_MAX, FLOAT_MAX };

	return { ((-b + sqrt(discriminant)) / (2.0f*a)), ((-b - sqrt(discriminant)) / (2.0f * a)) };
}

constexpr std::pair<int, float> ClosestIntersection(Vector3 O, Vector3 D, float t_min, float t_max)
{
	float closest_t = FLOAT_MAX;
	int closest_sphere{ -1 }; // -1 because not a valid index, assume these spheres are in an array
	int n{ static_cast<int>(Scene::spheres.size()) };
	for (int i = 0; i < n; ++i)
	{
		std::pair<float, float> intersections{ IntersectRaySphere(O,D,Scene::spheres.at(i)) };
		if (intersections.first <= t_max && intersections.first >= t_min && intersections.first < closest_t)
		{
			closest_t = intersections.first;
			closest_sphere = i;
		}
		if (intersections.second <= t_max && intersections.second >= t_min && intersections.second < closest_t)
		{
			closest_t = intersections.second;
			closest_sphere = i;
		}
	}
	return { closest_sphere,closest_t };
}

// P is point of the object we are tracing the ray from
// N is the unit normal vector to that surface that P is a point on
constexpr float ComputeLightIntensity(Vector3 P, Vector3 N, Vector3 V, float specular_exponent)
{
	float light_intensity{ 0.0f };
	int n{ static_cast<int>(Scene::lights.size()) };
	for (int i = 0; i < n; ++i) // could be iterator loop instead?
								// would it make copies?
	{
		Light this_light{ Scene::lights[i] };
		if (this_light.type == LightType::ambient) light_intensity += this_light.intensity;
		else 
		{
			Vector3 L_vec{};
			float t_max{};
			if (this_light.type == LightType::point)
			{
				L_vec = { this_light.location.x - P.x,
						  this_light.location.y - P.y,
						  this_light.location.z - P.z
				};
				t_max = 1.0f;
			}
			else if (this_light.type == LightType::directional)
			{
				L_vec = this_light.direction;
				t_max = FLOAT_MAX;
			}
			else
			{
				// Ideally, would throw an exception of invalid light type here
				continue;
			}
			
			// Shadow check
			float epsilon{ 0.001f };
			std::pair shadow_check{ ClosestIntersection(P, L_vec, epsilon, t_max) };
			if (shadow_check.first != -1) continue;

			float n_dot_l = dot(N, L_vec);

			// Diffuse lighting component
			if (n_dot_l > 0)
			{
				light_intensity += this_light.intensity * n_dot_l / sqrtf(dot(L_vec, L_vec)); // got rid of the N mag division since it should be 1
			}
			// Specular lighting component
			if (specular_exponent != -1) 
			{
				Vector3 R_vec{ ReflectRay(L_vec,N) };
				float r_dot_v { dot(R_vec,V) };
				if (r_dot_v > 0) light_intensity += this_light.intensity * powf(r_dot_v / sqrtf(dot(R_vec, R_vec)) / sqrtf(dot(V, V)), specular_exponent);
			}
		}
	}
	return light_intensity;
}

// O is viewpoint, D is Direction from O to viewport (but what about reflections?)
// Looking at Gambetta's pseudocode, he just uses R, so maybe D is just a directional vector, and length doesn't matter
// That makes sense, because we are checking for intersections
// Taking a look at https://gabrielgambetta.com/computer-graphics-from-scratch/02-basic-raytracing.html again,
// tmin and tmax are values that we want to consider valid intersections with opaque/reflective objects along vecD direction
constexpr Color TraceRay(Vector3 O, Vector3 D, float t_min, float t_max, int recursion_depth)
{
	std::pair<int, float> closest_obstruction_result{ ClosestIntersection(O, D, t_min, t_max) };
	int closest_sphere = closest_obstruction_result.first;
	float closest_t = closest_obstruction_result.second;

	if (closest_sphere != -1)
	{
		Vector3 P{ O.x + closest_t * D.x,
					O.y + closest_t * D.y,
					O.z + closest_t * D.z };

		// Compute unit normal vector
		Vector3 closest_sphere_center{ Scene::spheres.at(closest_sphere).center };
		Vector3 N{  P.x - closest_sphere_center.x,
					P.y - closest_sphere_center.y,
					P.z - closest_sphere_center.z };
		float mag_N{ sqrtf(dot(N,N)) };
		N = { N.x / mag_N, N.y / mag_N, N.z / mag_N };

		float light_intensity{ ComputeLightIntensity(P,N,{-D.x,-D.y,-D.z},Scene::spheres.at(closest_sphere).specular_exponent) };

		Color base_color{ Scene::spheres.at(closest_sphere).color };
		Color local_color{ Color{
				ModulateColorByIntensity(base_color.r,light_intensity),
				ModulateColorByIntensity(base_color.g,light_intensity),
				ModulateColorByIntensity(base_color.b,light_intensity),
				base_color.a
		} };
		
		// Done if we have hit the recursion limit, or object is not reflective
		float r = Scene::spheres.at(closest_sphere).reflective;
		if (recursion_depth <= 0 || r <= 0) return local_color;

		// Compute reflected color
		Vector3 R = ReflectRay({ -D.x,-D.y,-D.z }, N);
		Color reflected_color = TraceRay(P, R, 0.001f, FLOAT_MAX, recursion_depth - 1);

		// local_color * (1 - r) + reflected_color * r;
		return {
				static_cast<unsigned char>(static_cast<float>(local_color.r) * (1.0f - r) + static_cast<float>(reflected_color.r) * r),
				static_cast<unsigned char>(static_cast<float>(local_color.g) * (1.0f - r) + static_cast<float>(reflected_color.g) * r),
				static_cast<unsigned char>(static_cast<float>(local_color.b) * (1.0f - r) + static_cast<float>(reflected_color.b) * r),
				base_color.a
		};
	}

	// if no sphere in the ray path, return the background color
	return Scene::background_color;
}

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	int resolution_width { 1400 };
	int resolution_height{ 1000 };
	InitWindow(resolution_width, resolution_height, "Graphics from Scratch 1");
	
	float Vw{ 1.0f };	// Viewport width
	float Vh{ 1.0f };	// Viewport height
	float d = 0.75f;	// Viewport distance

	// Camera at origin
	// Viewport center at distance `d`

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// update can go here
		Vector3 O{ 0.0, 0.0, 0.0 };

		// drawing
		BeginDrawing();

			// Setup the back buffer for drawing (clear color and depth buffers)
			ClearBackground(BLACK);

			// Loop over entire canvas (window)
			for (int x = 0; x < resolution_width; ++x)
			{
				for (int y = 0; y < resolution_height; ++y)
				{
					int Cx = x - resolution_width / 2;
					int Cy = -y + resolution_height / 2;
					// map coords to a 0-centered xy plane

					// These are assuming O is at the origin and pointing down the Z axis
					float Vx = Cx * Vw / static_cast<float>(resolution_width);
					float Vy = Cy * Vh / static_cast<float>(resolution_width);
					float Vz = d;

					Vector3 D{ Vx, Vy, Vz };

					int recursion_depth = 3;
					Color seen_color{ TraceRay(O, D, d, FLOAT_MAX, recursion_depth) };
					DrawPixel(x, y, seen_color);
				}
			}

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
