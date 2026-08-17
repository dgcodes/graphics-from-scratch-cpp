#include "gambetta_graphics_helpers.h"

namespace RasterGlobals
{
	constexpr Color background_color{ WHITE };
	constexpr int resolution_width{ 1400 };
	constexpr int resolution_height{ 1000 };
};

// So we can use Gambetta's conventions of a centered Cartesian system
// with y increasing up instead of downward
void DrawPixel_gbettaCoords(int posX, int posY, Color color)
{
	// Convert to raylib coordinate system (0,0) at top left and y increasing downward
	int raylibX =  posX + RasterGlobals::resolution_width  / 2;
	int raylibY = -posY + RasterGlobals::resolution_height / 2;
	DrawPixel(raylibX, raylibY, color);
}

void myDrawLine(CanvasPoint P0, CanvasPoint P1, Color color)
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

void DrawWireframeTriangle(CanvasPoint P0, CanvasPoint P1, CanvasPoint P2, Color color)
{
	myDrawLine(P0, P1, color);
	myDrawLine(P1, P2, color);
	myDrawLine(P2, P0, color);
}

void DrawFilledTriangle(CanvasPoint P0, CanvasPoint P1, CanvasPoint P2, Color color)
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

void DrawShadedTriangle(CanvasPoint P0, CanvasPoint P1, CanvasPoint P2, Color color)
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
		std::vector<float> hs = { LinearInterpolate_2d(static_cast<int>(x_left[y-P0.y]),static_cast<int>(x_right[y-P0.y]),
				h_left[y-P0.y],h_right[y-P0.y]) };
		for (int x = static_cast<int>(x_left[y - P0.y]); x <= static_cast<int>(x_right[y - P0.y]); ++x)
		{
			float h = hs[x - static_cast<int>(x_left[y - P0.y])];
			DrawPixel_gbettaCoords(x, y, ModulateColorByIntensity(color,h));
		}
	}
}

void RasterizationPractice()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context

	InitWindow(RasterGlobals::resolution_width, RasterGlobals::resolution_height, "Graphics from Scratch 1");

	float Vw{ 1.0f };	// Viewport width
	float Vh{ 1.0f };	// Viewport height
	float d = 0.75f;	// Viewport distance

	// Camera at origin (to start)
	// Viewport center at distance `d`
	Vector3 O{ 0.0, 0.0, 0.0 };

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
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
		DrawWireframeTriangle({ -200,-250 }, { 200,50 }, { 20,250 }, BLACK);
		DrawFilledTriangle({ -200,-250 }, { 200,50 }, { 20,250 }, ORANGE);
		DrawShadedTriangle({ -500,-300,0.1f }, { -300,-450,1.0f }, { -50,-50,0.4f }, PURPLE);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
}
