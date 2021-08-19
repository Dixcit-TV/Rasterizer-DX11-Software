#pragma once

enum class FilterMode
{
	POINT, LINEAR, ANISOTROPIC
	, COUNT
};

enum class RenderMode
{
	SOFTWARE_RENDERING, HARDWARE_RENDERING
	, COUNT
};

enum class CullMode
{
	MESHBASED, NONE, BACKFACE, FRONTFACE
	, COUNT
};

enum class PrimitiveTopology
{
	TRIANGLELIST = 3
	, TRIANGLESTRIP = 1
};

enum class CameraSystem
{
	RIGHTHANDED, LEFTHANDED
	, COUNT
};

enum class MaterialType
{
	OPAQUE_MATERIAL, TRANSPARENT_MATERIAL
};
