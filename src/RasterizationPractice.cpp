#include "gambetta_graphics_helpers.h"
#include <cmath>

namespace RasterGlobals
{
	constexpr Color background_color{ WHITE };

	constexpr int resolution_width{ 1400 };
	constexpr int resolution_height{ 1000 };
	constexpr float resolution_width_float{ static_cast<float>(resolution_width) };
	constexpr float resolution_height_float{ static_cast<float>(resolution_height) };
	// To avoid some unnecessary casting in various functions

	constexpr float Vw{ 1.0f };		// Viewport width
	constexpr float Vh{ 1.0f };		// Viewport height
	constexpr float Vd{ 0.75f };	// Viewport distance from eye
	// Note : These 3 are in 'scene units'
};

class Triangle
{
private:
	static constexpr Color default_color{ BLACK };
	Color m_color;
	std::array<int,3> vertex_indices; // refers to indices in the triangle list of the myModel
public:
	Triangle() = delete;
	Triangle(int v0, int v1, int v2, Color color = default_color)
		: vertex_indices{ v0, v1, v2 }, m_color{ color }
	{}
	friend void RenderTriangle(bool wireframe_only, const std::vector<CanvasPoint>& vertex_list, const Triangle& triangle);
};

class myModel
{
private:
	std::string m_name;
	std::vector<Vector3> m_vertices; // Triangle.vertex_indices refers to these
	std::vector<Triangle> m_triangles;
public:
	myModel() = delete;
	myModel(std::string name, std::vector<Vector3> vertices, std::vector<Triangle> triangles)
		: m_name{name},m_vertices{vertices},m_triangles{triangles}
	{}
	
	const std::string GetName() const { return m_name; }
	const std::vector<Vector3>& GetVertices() const { return m_vertices; }
	const std::vector<Triangle>& GetTriangles() const { return m_triangles; }
};

namespace RasterObjectModels
{
	// I wonder what the best practices are for loading and storing these kinds of data
	myModel model_cube{ "cube",
		// Vertex list
		{
			Vector3{-1.0f, -1.0f, -1.0f},
			Vector3{ 1.0f, -1.0f, -1.0f},
			Vector3{ 1.0f,  1.0f, -1.0f},
			Vector3{-1.0f,  1.0f, -1.0f},
			Vector3{-1.0f, -1.0f,  1.0f},
			Vector3{ 1.0f, -1.0f,  1.0f},
			Vector3{ 1.0f,  1.0f,  1.0f},
			Vector3{-1.0f,  1.0f,  1.0f}
		},
		// Triangles list
		{
			Triangle{0,1,5,MAROON},
			Triangle{0,4,5,MAROON},
			Triangle{3,2,6,DARKPURPLE},
			Triangle{3,7,6,DARKPURPLE},
			Triangle{0,3,4,DARKGREEN},
			Triangle{7,3,4,DARKGREEN},
			Triangle{2,1,5,GOLD},
			Triangle{2,6,5,GOLD},
			Triangle{0,1,2,DARKGRAY},
			Triangle{0,3,2,DARKGRAY},
			Triangle{4,5,6,DARKBLUE},
			Triangle{4,7,6,DARKBLUE}
		}
	};
};

// So we can use Gambetta's conventions of a centered Cartesian system
// with y increasing up instead of downward
void DrawPixel_gbettaCoords(int posX, int posY, const Color& color)
{
	// Convert to raylib coordinate system (0,0) at top left and y increasing downward
	int raylibX =  posX + RasterGlobals::resolution_width  / 2;
	int raylibY = -posY + RasterGlobals::resolution_height / 2;
	DrawPixel(raylibX, raylibY, color);
}

constexpr CanvasPoint ViewportToCanvas(float x, float y)
{
	return {
		static_cast<int>(x * RasterGlobals::resolution_width_float / RasterGlobals::Vw),
		static_cast<int>(y * RasterGlobals::resolution_height_float / RasterGlobals::Vh)
	};
}

constexpr CanvasPoint ProjectVertex(Vector3 v)
{
	return ViewportToCanvas(v.x * RasterGlobals::Vd / v.z, v.y * RasterGlobals::Vd / v.z);
}

void myDrawLine(CanvasPoint P0, CanvasPoint P1, const Color& color)
{
	//assert(P1.x != P0.x && "Divide by zero detected in myDrawLine");
	if (abs(P1.x - P0.x) > abs(P1.y - P0.y))
	{
		if (P0.x > P1.x) std::swap(P0, P1); // First point needs to come before second in the independent variable for LinearInterpolate_2d()
		std::vector<float> ys{ LinearInterpolate_2d(P0.x,P1.x,P0.y,P1.y) };
		for (int x = P0.x; x <= P1.x; ++x) DrawPixel_gbettaCoords(x, static_cast<int>(ys[x - P0.x]), color);
	}
	else
	{
		if (P0.y > P1.y) std::swap(P0, P1);
		std::vector<float> xs{ LinearInterpolate_2d(P0.y,P1.y,P0.x,P1.x) }; // swap x and y for the opposite-method interpolation
		for (int y = P0.y; y <= P1.y; ++y) DrawPixel_gbettaCoords(static_cast<int>(xs[y-P0.y]), y, color);
	}
}

void DrawWireframeTriangle(CanvasPoint P0, CanvasPoint P1, CanvasPoint P2, const Color& color)
{
	myDrawLine(P0, P1, color);
	myDrawLine(P1, P2, color);
	myDrawLine(P2, P0, color);
}

void DrawFilledTriangle(CanvasPoint P0, CanvasPoint P1, CanvasPoint P2, const Color& color)
{
	if (P1.y < P0.y) std::swap(P0, P1);
	if (P2.y < P0.y) std::swap(P0, P2);
	if (P2.y < P1.y) std::swap(P1, P2);
	// Now order of increasing y-val is P0.y <= P1.y <= P2.y (although are we assuming not all collinear?)

	// Now get the interpolated x-vals of each edge
	// Need to feed the points in with swapped x-y's to get the x's interpolated instead of the y's
	std::vector<float> x01{ LinearInterpolate_2d(P0.y,P1.y,P0.x,P1.x) };
	std::vector<float> x12{ LinearInterpolate_2d(P1.y,P2.y,P1.x,P2.x) };
	std::vector<float> x02{ LinearInterpolate_2d(P0.y,P2.y,P0.x,P2.x) };
	
	x01.pop_back(); // because we concatenate 01 and 12 when drawing horizontal lines, there is overlap at the end from interp step
	std::vector<float> x012;
	x012.reserve(x01.size() + x12.size());
	x012.append_range(x01);
	x012.append_range(x12);

	std::vector<float>& x_left{x02};
	std::vector<float>& x_right{x012};
	int mid_x = static_cast<int>(x02.size() / 2);
	if (x02[mid_x] < x012[mid_x])
	{
		x_left = x02;
		x_right = x012;
	}
	else
	{
		x_left = x012;
		x_right = x02;
	}

	for (int y = P0.y; y <= P2.y; ++y)
	{
		for (int x = static_cast<int>(x_left[y - P0.y]); x <= static_cast<int>(x_right[y - P0.y]); ++x)
		{
			DrawPixel_gbettaCoords(x, y, color);
		}
	}
}

void DrawShadedTriangle(CanvasPoint P0, CanvasPoint P1, CanvasPoint P2, const Color& color)
{
	if (P1.y < P0.y) std::swap(P0, P1);
	if (P2.y < P0.y) std::swap(P0, P2);
	if (P2.y < P1.y) std::swap(P1, P2);
	// Now order of increasing y-val is P0.y <= P1.y <= P2.y (although are we assuming not all collinear?)

	// Now get the interpolated x-vals of each edge
	// Need to feed the points in with swapped x-y's to get the x's interpolated instead of the y's
	std::vector<float> x01{ LinearInterpolate_2d(P0.y,P1.y,P0.x,P1.x) };
	std::vector<float> h01{ LinearInterpolate_2d(P0.y,P1.y,P0.h,P1.h) };
	std::vector<float> x12{ LinearInterpolate_2d(P1.y,P2.y,P1.x,P2.x) };
	std::vector<float> h12{ LinearInterpolate_2d(P1.y,P2.y,P1.h,P2.h) };
	std::vector<float> x02{ LinearInterpolate_2d(P0.y,P2.y,P0.x,P2.x) };
	std::vector<float> h02{ LinearInterpolate_2d(P0.y,P2.y,P0.h,P2.h) };

	x01.pop_back(); // because we concatenate 01 and 12 when drawing horizontal lines, there is overlap at the end from interp step
	std::vector<float> x012;
	x012.reserve(x01.size() + x12.size());
	x012.append_range(x01);
	x012.append_range(x12);

	h01.pop_back(); // because we concatenate 01 and 12 when drawing horizontal lines, there is overlap at the end from interp step
	std::vector<float> h012;
	h012.reserve(h01.size() + h12.size());
	h012.append_range(h01);
	h012.append_range(h12);

	int mid_x = static_cast<int>(x02.size() / 2);
	bool x02_is_left = x02[mid_x] < x012[mid_x];
	std::vector<float>& x_left{ x02_is_left ? x02 : x012 };
	std::vector<float>& h_left{ x02_is_left ? h02 : h012 };
	std::vector<float>& x_right{ x02_is_left ? x012 : x02 };
	std::vector<float>& h_right{ x02_is_left ? h012 : h02 };

	for (int y = P0.y; y <= P2.y; ++y)
	{
		std::vector<float> hs = { LinearInterpolate_2d(static_cast<int>(std::floor(x_left[y-P0.y])),static_cast<int>(std::floor(x_right[y-P0.y])),
				h_left[y-P0.y],h_right[y-P0.y]) };
		for (int x = static_cast<int>(std::floor(x_left[y - P0.y])); x <= static_cast<int>(std::floor(x_right[y - P0.y])); ++x)
		{
			float h = hs[x - static_cast<int>(std::floor(x_left[y - P0.y]))];
			DrawPixel_gbettaCoords(x, y, ModulateColorByIntensity(color,h));
		}
	}
}

void RenderTriangle(bool wireframe_only, const std::vector<CanvasPoint>& vertex_list, const Triangle& triangle)
{
	if (wireframe_only)
	{
		DrawWireframeTriangle(
			vertex_list[triangle.vertex_indices[0]],
			vertex_list[triangle.vertex_indices[1]],
			vertex_list[triangle.vertex_indices[2]],
			triangle.m_color
			);
	}
	else
	{
		DrawShadedTriangle(
			vertex_list[triangle.vertex_indices[0]],
			vertex_list[triangle.vertex_indices[1]],
			vertex_list[triangle.vertex_indices[2]],
			triangle.m_color
		);
	}
}

class Instance
{
private:
	myModel* m_model;
	std::vector<CanvasPoint> m_latest_projected_vertices; // Useful?
	std::vector<Vector3> m_latest_transformed_vertices;
	Matrix m_rotation;
	Matrix m_translation; // Identity but last column is Tx Ty Tz 1
	float m_scale{ 1.0f };

public:
	Instance() = delete;
	Instance(myModel* model, Matrix rotation = MatrixIdentity(), Matrix translation = MatrixIdentity(), float scale = 1.0f)
		: m_model{ model }, m_rotation{ rotation }, m_translation{ translation }, m_scale{ scale }, m_latest_transformed_vertices{ model->GetVertices() } // TODO : check if : does initializing a std::vector work like this?
	{
		SimpleRenderInstance_translation(true, false, false); // init the translation
	}

	// More of a test function, next we'll try implementing the more full render including the transformation matrices?
	// TODO : Extend to full transform matrix (or maybe just the instance transform components here? Where would camera part go best?)
	void SimpleRenderInstance_translation(bool perform_transform, bool wireframe_only, bool do_render)
	{
		if (perform_transform)
		{
			// Probably unnecessary code, if the initializer works?
			auto model_num_vertices{ m_model->GetVertices().size() };
			if (m_latest_transformed_vertices.size() != model_num_vertices
				|| m_latest_projected_vertices.size() != model_num_vertices)
			{
				m_latest_transformed_vertices.resize(model_num_vertices);
				m_latest_projected_vertices.resize(model_num_vertices);
			}

			int i = 0;
			for (const Vector3& vertex : m_model->GetVertices())
			{
				Vector3 v_prime = Vector3Transform(vertex, m_translation);
				m_latest_transformed_vertices[i] = v_prime; // For later optimization purposes (not coded yet)
				m_latest_projected_vertices[i] = ProjectVertex(v_prime);
				i++;
			}
		}

		if (do_render)
		{
			for (const Triangle& triangle : m_model->GetTriangles())
			{
				RenderTriangle(wireframe_only, m_latest_projected_vertices, triangle);
			}
		}
	}
};

namespace SceneRaster
{

};

void CubeDrawTest()
{
	// The four "front" vertices
	Vector3 vAf { -2.0f, -0.5f, 5.0f };
	Vector3 vBf { -2.0f, 0.5f, 5.0f };
	Vector3 vCf { -1.0f, 0.5f, 5.0f };
	Vector3 vDf { -1.0f, -0.5f, 5.0f };

	// The four "back" vertices
	Vector3 vAb { -2.0f, -0.5f, 6.0f };
	Vector3 vBb { -2.0f, 0.5f, 6.0f };
	Vector3 vCb { -1.0f, 0.5f, 6.0f };
	Vector3 vDb { -1.0f, -0.5f, 6.0f };

	// The front face
	myDrawLine(ProjectVertex(vAf), ProjectVertex(vBf), BLUE);
	myDrawLine(ProjectVertex(vBf), ProjectVertex(vCf), BLUE);
	myDrawLine(ProjectVertex(vCf), ProjectVertex(vDf), BLUE);
	myDrawLine(ProjectVertex(vDf), ProjectVertex(vAf), BLUE);

	// The back face
	myDrawLine(ProjectVertex(vAb), ProjectVertex(vBb), RED);
	myDrawLine(ProjectVertex(vBb), ProjectVertex(vCb), RED);
	myDrawLine(ProjectVertex(vCb), ProjectVertex(vDb), RED);
	myDrawLine(ProjectVertex(vDb), ProjectVertex(vAb), RED);

	// The front-to-back edges
	myDrawLine(ProjectVertex(vAf), ProjectVertex(vAb), GREEN);
	myDrawLine(ProjectVertex(vBf), ProjectVertex(vBb), GREEN);
	myDrawLine(ProjectVertex(vCf), ProjectVertex(vCb), GREEN);
	myDrawLine(ProjectVertex(vDf), ProjectVertex(vDb), GREEN);
}

void RasterizationPractice()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context

	InitWindow(RasterGlobals::resolution_width, RasterGlobals::resolution_height, "Graphics from Scratch 1");

	// Camera at origin (to start)
	// Viewport center at distance `d`
	Vector3 O{ 0.0, 0.0, 0.0 };

	// Cube draw test using instancing
	// -Instantiate a cube myModel -> Done in RasterObjectsModels namespace
	// -Create an Instance
	// -Render (use some translation) -> Do in the frame update loop
	Matrix translation{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 5.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
	}; // Should just push it along the +Z so we can see it without camera movement
	Instance cube_instance_test1{ &RasterObjectModels::model_cube, MatrixIdentity(), translation };

	// game loop
	while (!WindowShouldClose())	// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// update can go here


		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(RasterGlobals::background_color);

		// X-drawing test
		//myDrawLine({ -200,-100 }, { 240,120 }, BLUE);
		//myDrawLine({ -50,-200 }, { 60,240 }, GREEN);

		// Triangle drawing tests
		//DrawWireframeTriangle({ -200,-250 }, { 200,50 }, { 20,250 }, BLACK);
		//DrawFilledTriangle({ -200,-250 }, { 200,50 }, { 20,250 }, ORANGE);
		//DrawShadedTriangle({ -500,-300,0.1f }, { -300,-450,1.0f }, { -50,-50,0.4f }, PURPLE);

		// Triangle drawing tests 2 (with RenderTriangle)
		/*
		std::vector<CanvasPoint> render_triangle_test_vertices_1{
			{ -500, -300, 0.1f }, { -300,-450,1.0f }, { -50,-50,0.4f }
		};
		Triangle render_triangle_test_triangle_1(0, 2, 1, PURPLE);
		RenderTriangle(true, render_triangle_test_vertices_1, render_triangle_test_triangle_1);
		*/

		// Cube draw Test
		//CubeDrawTest();

		// Cube draw test using Instance::SimpleRenderInstance_translation !
		cube_instance_test1.SimpleRenderInstance_translation(false, true, true);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
}
