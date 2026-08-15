#include "gambetta_graphics_helpers.h"

struct CanvasPoint
{
	int x;
	int y;
};

void myDrawLine(CanvasPoint P0, CanvasPoint P1, Color color)
{
	assert(P1.x != P0.x && "Divide by zero detected in myDrawLine");
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

		// Loop over entire canvas (window)
		for (int x = 0; x < resolution_width; ++x)
		{
			for (int y = 0; y < resolution_height; ++y)
			{
				int Cx = x - resolution_width / 2;
				int Cy = -y + resolution_height / 2;
				// map coords to a 0-centered xy plane

				// These are assuming O is at the origin and pointing down the Z axis?
				// Maybe not? They only involve canvas and viewport, which are fixed for the camera
				float Vx = Cx * Vw / static_cast<float>(resolution_width);
				float Vy = Cy * Vh / static_cast<float>(resolution_width);
				float Vz = d;

				// This part, however, should probably change based on rotation of the camera
				Vector3 D{ Vx, Vy, Vz };
				// TODO : Determine some rotation matrix and multiply here
				Vector3 rotated_D = RotateCameraCCW_xzPlane(D, 30); // Try a 30 degree left "yaw?" rotation of the camera

				//int recursion_depth = 3;
				Vector3 camera_translation{ 1.0f, 1.0f, 0.0f };
				//Vector3 camera_translation{ -1.0f, -1.0f, 0.0f };
				Vector3 translated_O = { O.x + camera_translation.x, O.y + camera_translation.y, O.z + camera_translation.z };

				//Color seen_color{ TraceRay(translated_O, rotated_D, d, FLOAT_MAX, recursion_depth) };
				//DrawPixel(x, y, RED);
				myDrawLine({ 500,500 }, { 700,300 }, BLUE);
			}
		}

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
}
