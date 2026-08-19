#include "raylib.h"
#include <iostream>
#include <vector>

void PrintVertices(std::vector<Vector3> vertices)
{
	for (Vector3 v : vertices)
	{
		std::cout << v.x << ' ' << v.y << ' ' << v.z << '\n';
	}
}

void CppTests()
{
	//std::cout << "Hello from CppTests.cpp\n";

	// Want to test how initialization of a std::vector works
	PrintVertices(
		{
			Vector3{-1.0f, -1.0f, -1.0f},
			{1.0f, 1.0f, 1.0f}
		}
	);
	// Is it slower to not declare the Vector3's explicitly?
	// In other words, does it need more instructions to perform the implicit conversion?
	// Probably not slower, since it should be a compile-time detection
}