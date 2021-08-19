#pragma once
#include "EMath.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include "EMath.h"
#include "Struct.h"
#include "Enum.h"

class Texture;

namespace Utils
{
	template<typename T>
	inline Elite::Vector<3, T> GetWorldX()
	{
		return { static_cast<T>(1.f), static_cast<T>(0.f), static_cast<T>(0.f) };
	}

	template<typename T>
	inline Elite::Vector<3, T> GetWorldZ()
	{
		return { static_cast<T>(0.f), static_cast<T>(0.f), static_cast<T>(1.f) };
	}

	template<typename T>
	inline Elite::Vector<3, T> GetWorldY()
	{
		return { static_cast<T>(0.f), static_cast<T>(1.f), static_cast<T>(0.f) };
	}

	template<int N, int M, typename T>
	void PrintMatrix(const Elite::Matrix<N, M, T>& matrix)
	{
		for (int m{ 0 }; m < M; ++m)
		{
			std::cout << "|";
			for (int n{ 0 }; n < N; ++n)
			{
				std::cout << std::fixed << std::setw(7) << std::left << std::setprecision(3) << matrix[n][m];
			}
			std::cout << "|\n";
		}

		std::cout << "\n\n";
	}

	template<typename ENUM_TYPE, typename = typename std::enable_if< std::is_enum<ENUM_TYPE>::value >::type>
	ENUM_TYPE ToggleEnum(ENUM_TYPE enumValue, int step = 1)
	{
		return ENUM_TYPE((int(enumValue) + step) % int(ENUM_TYPE::COUNT));
	}

	template<typename TYPENAME>
	void SafeDelete(TYPENAME*& pObj)
	{
		delete pObj;
		pObj = nullptr;
	}

	template<typename TYPENAME>
	void SafeRelease(TYPENAME*& pObj)
	{
		if (pObj)
		{
			pObj->Release();
			pObj = nullptr;
		}
	}
};

namespace Rasterizer
{
	bool ConvertVerticesWorldToScreenSpace(const Vertex_Input originalVertices[TRI_VERTEX_COUNT], Vertex_Output transformedVertices[TRI_VERTEX_COUNT], const Elite::FMatrix4& worldViewProjectionMatrix, const Elite::FMatrix4& worldMatrix, const Elite::FPoint3& cameraPos, uint32_t width, uint32_t height);
	Aabb2D GetAabb2D(const Vertex_Output vertices[TRI_VERTEX_COUNT], uint32_t width, uint32_t height);
	bool IsPixelInTriangle(const Vertex_Output vertices[TRI_VERTEX_COUNT], const Elite::FPoint2& pixel, CullMode culling, float& w0, float& w1, float& w2);
	bool CreateTriangle(PrimitiveTopology topology, const std::vector<Vertex_Input>& vertices, const std::vector<uint32_t>& indexes, size_t currentIdx, Vertex_Input outTriangle[TRI_VERTEX_COUNT]);

	Vertex_Output GetInterpolatedPixelInfo(const Elite::FPoint2& pixePos, const Vertex_Output screenVertices[TRI_VERTEX_COUNT], float zInterpolated, float w0, float w1, float w2);
}

namespace ObjReader
{
	void LoadModel(const std::string& objPath, std::vector<Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer);

	uint32_t GetVertexIdx(const Vertex_Input& vertex, const std::vector<Vertex_Input>& vertexBuffer);
}

