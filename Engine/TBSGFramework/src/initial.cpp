#include "Initial.h"
#include <iostream>

// #TEMP BEGIN
//#include "shader_loader.h"

//bool readRawTextureData(const char *path, void *address, size_t size)
//{
//	bool success = false;
//
//	FILE *fp = fopen(path, "rb");
//	if (fp != NULL)
//	{
//		success = readFileContents(address, size, fp);
//		fclose(fp);
//	}
//
//	return success;
//}
// #TEMP END

int Initial::initial() {
	//gfx::PS4Renderer renderer;
	//renderer.Initialize();
	//std::cout << "SUCCESSFULLY INITIALIZED!" << std::endl;

	//const gfx::Vertex vertexData[] =
	//{
	//	// 2    3
	//	// +----+
	//	// |\   |
	//	// | \  |
	//	// |  \ |
	//	// |   \|
	//	// +----+
	//	// 0    1

	//	//   POSITION                COLOR               UV
	//	{-0.5f, -0.5f, 0.5f,    0.7f, 0.7f, 1.0f,    0.0f, 1.0f},
	//	{ 0.5f, -0.5f, 0.5f,    0.7f, 0.7f, 1.0f,    1.0f, 1.0f},
	//	{ 0.5f,  0.5f, 0.5f,    1.0f, 0.7f, 1.0f,    1.0f, 0.0f},
	//	{-0.5f,  0.5f, 0.5f,    0.7f, 1.0f, 1.0f,    0.0f, 0.0f},

	//	{-0.5f, 0.5f,  0.5f,   0.7f, 0.7f, 1.0f,    0.0f, 1.0f},
	//	{ 0.5f, 0.5f,  0.5f,   0.7f, 0.7f, 1.0f,    1.0f, 1.0f},
	//	{ 0.5f, 0.5f, -0.5f,   0.7f, 1.0f, 1.0f,    1.0f, 0.0f},
	//	{-0.5f, 0.5f, -0.5f,   1.0f, 0.7f, 1.0f,    0.0f, 0.0f},

	//	{ 0.5f, -0.5f, -0.5f,    0.7f, 0.7f, 1.0f,    0.0f, 1.0f},
	//	{-0.5f, -0.5f, -0.5f,    0.7f, 0.7f, 1.0f,    1.0f, 1.0f},
	//	{-0.5f,  0.5f, -0.5f,    1.0f, 0.7f, 1.0f,    1.0f, 0.0f},
	//	{ 0.5f,  0.5f, -0.5f,    0.7f, 1.0f, 1.0f,    0.0f, 0.0f},

	//	{-0.5f, -0.5f, -0.5f,   0.7f, 0.7f, 1.0f,    0.0f, 1.0f},
	//	{ 0.5f, -0.5f, -0.5f,   0.7f, 0.7f, 1.0f,    1.0f, 1.0f},
	//	{ 0.5f, -0.5f,  0.5f,   0.7f, 1.0f, 1.0f,    1.0f, 0.0f},
	//	{-0.5f, -0.5f,  0.5f,   1.0f, 0.7f, 1.0f,    0.0f, 0.0f},

	//	{-0.5f, -0.5f, -0.5f,    0.7f, 0.7f, 1.0f,    0.0f, 1.0f},
	//	{-0.5f, -0.5f,  0.5f,    0.7f, 0.7f, 1.0f,    1.0f, 1.0f},
	//	{-0.5f,  0.5f,  0.5f,    1.0f, 0.7f, 1.0f,    1.0f, 0.0f},
	//	{-0.5f,  0.5f, -0.5f,    0.7f, 1.0f, 1.0f,    0.0f, 0.0f},

	//	{ 0.5f, -0.5f,  0.5f,   0.7f, 0.7f, 1.0f,    0.0f, 1.0f},
	//	{ 0.5f, -0.5f, -0.5f,   0.7f, 0.7f, 1.0f,    1.0f, 1.0f},
	//	{ 0.5f,  0.5f, -0.5f,   0.7f, 1.0f, 1.0f,    1.0f, 0.0f},
	//	{ 0.5f,  0.5f,  0.5f,   1.0f, 0.7f, 1.0f,    0.0f, 0.0f},
	//};

	//const uint16_t indexData[] =
	//{
	//  0,  1,  2,
	// 2,  3,  0,
	// // top
	//  4,  5,  6,
	//  6,  7,  4,
	//  // back
	//   8,  9, 10,
	//  10, 11,  8,
	//  // bottom
	//  12, 13, 14,
	//  14, 15, 12,
	//  // left
	//  16, 17, 18,
	//  18, 19, 16,
	//  // right
	//  20, 21, 22,
	//  22, 23, 20,
	//};

	//renderer.SetVertertexBuffer(&vertexData[0], 24, &indexData[0], 36);

	/*while(1)
		renderer.RenderWorld();

	renderer.Shutdown();*/

	return 0;
}

