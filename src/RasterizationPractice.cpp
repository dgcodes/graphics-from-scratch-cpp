#include "gambetta_graphics_helpers.h"

void myDrawLine(CanvasPoint P0, CanvasPoint P1, Color color)
{
	//assert(P1.x != P0.x && "Divide by zero detected in myDrawLine");
	if (abs(P1.x - P0.x) > abs(P1.y - P0.y))
	{
		if (P0.x > P1.x) std::swap(P0, P1); // First point needs to come before second in the independent variable for LinearInterpolate_2d()
		std::vector<float> ys{ LinearInterpolate_2d(P0,P1) };
		for (int x = P0.x; x <= P1.x; ++x) DrawPixel(x, ys[x - P0.x], color);
	}
	else
	{
		if (P0.y > P1.y) std::swap(P0, P1);
		std::vector<float> xs{ LinearInterpolate_2d({P0.y,P0.x},{P1.y,P1.x}) }; // swap x and y for the opposite-method interpolation
		for (int y = P0.y; y <= P1.y; ++y) DrawPixel(xs[y-P0.y], y, color);
	}
}

void RasterizationPractice()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	int resolution_width{ 1400 };
	int resolution_height{ 1000 };
	InitWindow(resolution_width, resolution_height, "Graphics from Scratch 1");

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
		ClearBackground(WHITE);

		myDrawLine({ 500,500 }, { 700,100 }, BLUE);
		myDrawLine({ 300,300 }, { 800,400 }, GREEN);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
}
