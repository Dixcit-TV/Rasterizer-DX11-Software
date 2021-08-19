#pragma once
#include "Emath.h"
#include "ERGBColor.h"

const int TRI_VERTEX_COUNT{ 3 };

struct Vertex_Input
{
	Elite::FPoint3 position;
	Elite::FVector3 normal;
	Elite::FVector3 tangent;
	Elite::FVector2 uv;
};

struct Vertex_Output
{
	Elite::FPoint4 position;
	Elite::FVector3 normal;
	Elite::FVector3 tangent;
	Elite::FVector3 viewVector;
	Elite::FVector2 uv;
};

struct Aabb2D
{
	uint32_t bot;
	uint32_t left;
	uint32_t top;
	uint32_t right;
};


struct DirectionalLight
{
	Elite::RGBColor color;
	Elite::FVector3 nDirection;
	float intensity;
};