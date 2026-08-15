#include "gambetta_graphics_helpers.h"

struct CanvasPoint
{
	int x;
	int y;
};

void myDrawLine(CanvasPoint P0, CanvasPoint P1, Color color)
{
	//assert(P1.x != P0.x && "Divide by zero detected in myDrawLine");
	float a = (P1.y - P0.y) / static_cast<float>(P1.x - P0.x);
	int y = P0.y;
	for (int x = P0.x; x <= P1.x; ++x)
	{
		DrawPixel(x, y, color);
		y += a;
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

		myDrawLine({ 500,500 }, { 700,300 }, BLUE);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
}
