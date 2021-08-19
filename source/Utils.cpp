#include "pch.h"
#include "Utils.h"
#include <fstream>
#include "Enum.h"
#include "Texture.h"

#pragma region Rasterizer

/// <summary>
/// Vertex Transofmration
/// </summary>
/// <param name="originalVertices">triangle vertices in model space</param>
/// <param name="transformedVertices">output triangle vertices in raster space</param>
/// <param name="worldViewProjectionMatrix">world view projection matrix</param>
/// <param name="worldMatrix">Triangle world matrix</param>
/// <param name="cameraPos">Camera position</param>
/// <param name="width">window width</param>
/// <param name="height">widnow height</param>
/// <returns>Return true if triangle wasn't clipped out, false otherwise.</returns>
bool Rasterizer::ConvertVerticesWorldToScreenSpace(const Vertex_Input originalVertices[TRI_VERTEX_COUNT], Vertex_Output transformedVertices[TRI_VERTEX_COUNT], const Elite::FMatrix4& worldViewProjectionMatrix, const Elite::FMatrix4& worldMatrix, const Elite::FPoint3& cameraPos, uint32_t width, uint32_t height)
{
	for (int idx{}; idx < TRI_VERTEX_COUNT; ++idx)
	{
		//To View Space
		Elite::FPoint4 tPosition{ worldViewProjectionMatrix * Elite::FPoint4(originalVertices[idx].position) };

		//Projection
		tPosition.x /= tPosition.w;
		tPosition.y /= tPosition.w;
		tPosition.z /= tPosition.w;
		tPosition.w = 1 / tPosition.w;

		if (tPosition.x < -1.f || tPosition.x > 1.f
			|| tPosition.y < -1.f || tPosition.y > 1.f
			|| tPosition.z < 0.f || tPosition.z > 1.f)
			return false;

		//To ScreanSpace
		tPosition.x = (tPosition.x + 1.f) / 2.f * width;
		tPosition.y = (1 - tPosition.y) / 2.f * height;

		transformedVertices[idx].position = tPosition;
		transformedVertices[idx].uv = originalVertices[idx].uv;
		transformedVertices[idx].normal = Elite::FVector3(worldMatrix * Elite::FVector4(originalVertices[idx].normal));
		transformedVertices[idx].tangent = Elite::FVector3(worldMatrix * Elite::FVector4(originalVertices[idx].tangent));
		transformedVertices[idx].viewVector = cameraPos - Elite::FPoint3(worldMatrix * Elite::FPoint4(originalVertices[idx].position));
	}

	return true;
}

/// <summary>
/// Get Axis Aligned bounding box for the current triangle
/// </summary>
/// <param name="vertices">Triangle vertices</param>
/// <param name="width">Window width</param>
/// <param name="height">Window window height</param>
/// <returns>Bounding box data</returns>
Aabb2D Rasterizer::GetAabb2D(const Vertex_Output vertices[TRI_VERTEX_COUNT], uint32_t width, uint32_t height)
{
	int bot{ INT_MAX }, left{ INT_MAX }, top{ -INT_MAX }, right{ -INT_MAX };

	for (int idx{}; idx < TRI_VERTEX_COUNT; ++idx)
	{
		auto vertex{ vertices[idx] };
		left = int(vertex.position.x) < left ? int(vertex.position.x) : left;
		right = int(vertex.position.x + 1) > right ? int(vertex.position.x + 1) : right;
		bot = int(vertex.position.y) < bot ? int(vertex.position.y) : bot;
		top = int(vertex.position.y + 1) > top ? int(vertex.position.y + 1) : top;
	}

	left = std::clamp(left, 0, int(width));
	right = std::clamp(right, 0, int(width));
	bot = std::clamp(bot, 0, int(height));
	top = std::clamp(top, 0, int(height));

	return Aabb2D{ uint32_t(bot), uint32_t(left), uint32_t(top), uint32_t(right) };
}

/// <summary>
/// Rasterizer Inside outside test
/// </summary>
/// <param name="vertices">Triangle vertices</param>
/// <param name="pixel">Pixel to test</param>
/// <param name="culling">Chosen cullmode</param>
/// <param name="w0">out weight of vertex 0</param>
/// <param name="w1">out weight of vertex 1</param>
/// <param name="w2"out weight of vertex 1></param>
/// <returns>Return wether or not the pixel is inside the triangle</returns>
bool Rasterizer::IsPixelInTriangle(const Vertex_Output vertices[TRI_VERTEX_COUNT], const Elite::FPoint2& pixel, CullMode culling, float& w0, float& w1, float& w2)
{
	const Vertex_Output& v0{ vertices[0] };
	const Vertex_Output& v1{ vertices[1] };
	const Vertex_Output& v2{ vertices[2] };

	Elite::FVector2 edge_v1v2{ v2.position - v1.position };
	Elite::FVector2 edge_v2v0{ v0.position - v2.position };
	float tempW0, tempW1, invTriArea;

	tempW0 = Cross(pixel - Elite::FPoint2(v1.position), edge_v1v2);
	tempW1 = Cross(pixel - Elite::FPoint2(v2.position), edge_v2v0);

	//Change tests based on culling mode
	switch (culling)
	{
	case CullMode::NONE:
		if (abs(tempW0) < FLT_EPSILON || abs(tempW1) < FLT_EPSILON)
			return false;
		break;
	case CullMode::BACKFACE:
		if (tempW0 < FLT_EPSILON || tempW1 < FLT_EPSILON)
			return false;
		break;
	case CullMode::FRONTFACE:
		if (tempW0 > -FLT_EPSILON || tempW1 > -FLT_EPSILON)
			return false;
		break;
	}

	invTriArea = 1 / Cross(-edge_v1v2, edge_v2v0);

	tempW0 = tempW0 * invTriArea;
	if (tempW0 < 0.f || tempW0 > 1.f)
		return false;

	tempW1 = tempW1 * invTriArea;
	if (tempW1 < 0.f || tempW0 + tempW1 > 1.f)
		return false;

	w0 = tempW0;
	w1 = tempW1;
	w2 = 1 - (w0 + w1);

	return true;
}

/// <summary>
/// Assemble triangle based on topology and start index
/// </summary>
/// <param name="topology">Topology used to create Index buffer</param>
/// <param name="vertices">Vertex buffer</param>
/// <param name="indexes">Index Buffer</param>
/// <param name="currentIdx">Index of vertex 0 for the current triangle</param>
/// <param name="outTriangle">output trinangle vertices</param>
/// <returns>Returns wether or not a valid tirangle was contructed</returns>
bool Rasterizer::CreateTriangle(PrimitiveTopology topology, const std::vector<Vertex_Input>& vertices, const std::vector<uint32_t>& indexes, size_t currentIdx, Vertex_Input outTriangle[TRI_VERTEX_COUNT])
{
	bool isValidTri{ true };
	uint32_t idx1{ indexes[currentIdx] };
	uint32_t idx2{ indexes[currentIdx + 1] };
	uint32_t idx3{ indexes[currentIdx + 2] };

	switch (topology)
	{
	case PrimitiveTopology::TRIANGLELIST:
		outTriangle[0] = vertices[idx1];
		outTriangle[1] = vertices[idx2];
		outTriangle[2] = vertices[idx3];
		break;
	case PrimitiveTopology::TRIANGLESTRIP:
		if (idx1 != idx2 && idx2 != idx3 && idx3 != idx1)
		{
			if ((currentIdx & 1) == 0)
			{
				outTriangle[0] = vertices[idx1];
				outTriangle[1] = vertices[idx2];
				outTriangle[2] = vertices[idx3];
			}
			else
			{
				outTriangle[0] = vertices[idx1];
				outTriangle[1] = vertices[idx3];
				outTriangle[2] = vertices[idx2];
			}
		}
		else
			isValidTri = false;
		break;
	}

	return isValidTri;
}

/// <summary>
/// Interpolate pixel attributes
/// </summary>
/// <param name="pixePos">Current pixel</param>
/// <param name="screenVertices">Rasterized triangle vertices</param>
/// <param name="zInterpolated">Interpolated depth value</param>
/// <param name="w0">Vertex 0 weight</param>
/// <param name="w1">Vertex 1 weight</param>
/// <param name="w2">Vertex 2 weight</param>
/// <returns>Interpolated pixel information</returns>
Vertex_Output Rasterizer::GetInterpolatedPixelInfo(const Elite::FPoint2& pixePos, const Vertex_Output screenVertices[TRI_VERTEX_COUNT], float zInterpolated, float w0, float w1, float w2)
{
	Vertex_Output out{};

	const Vertex_Output& screenV0{ screenVertices[0] };
	const Vertex_Output& screenV1{ screenVertices[1] };
	const Vertex_Output& screenV2{ screenVertices[2] };
	//vertices W components already stored as 1 / w
	float interpolatedW0{ screenV0.position.w * w0 };
	float interpolatedW1{ screenV1.position.w * w1 };
	float interpolatedW2{ screenV2.position.w * w2 };

	float w = 1 / (interpolatedW0 + interpolatedW1 + interpolatedW2);

	out.position = Elite::FPoint4{ pixePos, Elite::Clamp(Elite::Remap(zInterpolated, 0.975f, 1.f), 0.f, 1.f), w };
	out.uv = (screenV0.uv * interpolatedW0 + screenV1.uv * interpolatedW1 + screenV2.uv * interpolatedW2) * w;
	out.normal = (screenV0.normal * interpolatedW0 + screenV1.normal * interpolatedW1 + screenV2.normal * interpolatedW2) * w;
	Elite::Normalize(out.normal);
	out.tangent = (screenV0.tangent * interpolatedW0 + screenV1.tangent * interpolatedW1 + screenV2.tangent * interpolatedW2) * w;
	Elite::Normalize(out.tangent);
	out.viewVector = (screenV0.viewVector * interpolatedW0 + screenV1.viewVector * interpolatedW1 + screenV2.viewVector * interpolatedW2) * w;
	Elite::Normalize(out.viewVector);

	return out;
}

#pragma endregion


#pragma region OBJReader

/// <summary>
/// Load data from Obj file
/// </summary>
/// <param name="objPath">Path to Obj</param>
/// <param name="vertexBuffer">output vertexBuffer</param>
/// <param name="indexBuffer">output indexBuffer</param>
void ObjReader::LoadModel(const std::string& objPath, std::vector<Vertex_Input>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
{
	std::ifstream objStream{ objPath, std::ios::in | std::ios::ate };

	if (!objStream.is_open())
		std::cout << "Error: Could not open obj file \"" << objPath << "\".";

	std::string line{};
	std::vector<Elite::FPoint3> tmpVertices;
	std::vector<Elite::FVector3> tmpVNormals;
	std::vector<Elite::FVector2> tmpUVs;
	std::vector<std::string> tmpFaces;

	//Parse all data in temporary arrays
	objStream.seekg(0, std::ios::beg);
	while (!objStream.eof())
	{
		std::getline(objStream, line, '\n');

		if (line._Starts_with("f "))
		{
			tmpFaces.push_back(line);
		}
		else
		{
			float f0, f1, f2;
			sscanf_s(line.c_str(), "%*s %f %f %f", &f0, &f1, &f2);

			if (line._Starts_with("v "))
			{
				tmpVertices.push_back({ f0, f1, f2 });
			}
			else if (line._Starts_with("vn "))
			{
				tmpVNormals.push_back({ f0, f1, f2 });
			}
			else if (line._Starts_with("vt "))
			{
				tmpUVs.push_back({ f0, 1 - f1 });
			}
		}
	}

	indexBuffer.reserve(tmpFaces.size());
	vertexBuffer.reserve(tmpFaces.size());
	int vIdx, uVIdx, nVIdx;

	//construct vertex and index buffer based on faces information
	for (const std::string& faceStr : tmpFaces)
	{
		int iv0, iv1, iv2, iuv0, iuv1, iuv2, in0, in1, in2;
		sscanf_s(faceStr.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &iv0, &iuv0, &in0, &iv1, &iuv1, &in1, &iv2, &iuv2, &in2);

		vIdx = iv0 - 1;
		const Elite::FPoint3& p0{ tmpVertices[vIdx] };
		vIdx = iv1 - 1;
		const Elite::FPoint3& p1{ tmpVertices[vIdx] };
		vIdx = iv2 - 1;
		const Elite::FPoint3& p2{ tmpVertices[vIdx] };

		uVIdx = iuv0 - 1;
		const Elite::FVector2& uv0{ tmpUVs[uVIdx] };
		uVIdx = iuv1 - 1;
		const Elite::FVector2& uv1{ tmpUVs[uVIdx] };
		uVIdx = iuv2 - 1;
		const Elite::FVector2& uv2{ tmpUVs[uVIdx] };

		nVIdx = in0 - 1;
		const Elite::FVector3& n0{ tmpVNormals[nVIdx] };
		nVIdx = in1 - 1;
		const Elite::FVector3& n1{ tmpVNormals[nVIdx] };
		nVIdx = in2 - 1;
		const Elite::FVector3& n2{ tmpVNormals[nVIdx] };

		Vertex_Input v0{ p0, n0, Elite::FVector3{}, uv0 };
		Vertex_Input v1{ p1, n1, Elite::FVector3{}, uv1 };
		Vertex_Input v2{ p2, n2, Elite::FVector3{}, uv2 };
		uint32_t idxV0{ GetVertexIdx(v0, vertexBuffer) };
		uint32_t idxV1{ GetVertexIdx(v1, vertexBuffer) };
		uint32_t idxV2{ GetVertexIdx(v2, vertexBuffer) };

		if (idxV0 == -1 || idxV1 == -1 || idxV2 == -1)
		{
			//Calculate vertex tangent
			//https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
			const Elite::FVector3 edge0{ p1 - p0 };
			const Elite::FVector3 edge1{ p2 - p0 };
			const Elite::FVector2 diffX{ uv1.x - uv0.x, uv2.x - uv0.x };
			const Elite::FVector2 diffY{ uv1.y - uv0.y, uv2.y - uv0.y };
			float r = 1.f / Cross(diffX, diffY);

			Elite::FVector3 tangent{ (edge0 * diffY.y - edge1 * diffY.x) * r };

			if (idxV0 == -1)
			{
				v0.tangent = Elite::GetNormalized(Elite::Reject(tangent, n0));
				idxV0 = int(vertexBuffer.size());
				vertexBuffer.push_back(std::move(v0));
			}
			if (idxV1 == -1)
			{
				v1.tangent = Elite::GetNormalized(Elite::Reject(tangent, n1));
				idxV1 = int(vertexBuffer.size());
				vertexBuffer.push_back(std::move(v1));
			}
			if (idxV2 == -1)
			{
				v2.tangent = Elite::GetNormalized(Elite::Reject(tangent, n2));
				idxV2 = int(vertexBuffer.size());
				vertexBuffer.push_back(std::move(v2));
			}
		}

		indexBuffer.push_back(idxV0);
		indexBuffer.push_back(idxV1);
		indexBuffer.push_back(idxV2);
	}
}

/// <summary>
/// Look for existing vertex in buffer
/// </summary>
/// <param name="vertex">Vertex to check</param>
/// <param name="vertexBuffer">Existing vertices</param>
/// <returns>Vertex index or -1 if not found</returns>
uint32_t ObjReader::GetVertexIdx(const Vertex_Input& vertex, const std::vector<Vertex_Input>& vertexBuffer)
{
	std::vector<Vertex_Input>::const_iterator cIt = std::find_if(vertexBuffer.cbegin(), vertexBuffer.cend(), [&vertex](const Vertex_Input& vBuffer) {
		return vBuffer.position == vertex.position && vBuffer.uv == vertex.uv && vBuffer.normal == vertex.normal;
		});

	return cIt == vertexBuffer.cend() ? -1 : uint32_t(std::distance(vertexBuffer.cbegin(), cIt));
}

#pragma endregion
