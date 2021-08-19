#pragma once
#include "EMath.h"
#include <iostream>

template<typename T>
struct Quaternion
{
	//Data Member
	T data[4];

	explicit Quaternion(T real, T i, T j, T k) : data{ real, i, j, k } {  }
	explicit Quaternion(T angleRad, const Elite::Vector<3, T>& axis) 
		: data { } 
	{  
		T halfAngle{ angleRad / 2 };
		T s{ static_cast<T>(sin(halfAngle)) };
		data[0] = static_cast<T>(cos(halfAngle));
		data[1] = axis.x * s;
		data[2] = axis.y * s;
		data[3] = axis.z * s;
	}
	explicit Quaternion(T i, T j, T k) : data{ static_cast<T>(0), i, j, k } {  }

	inline T Real() const { return data[0]; };
	inline Elite::Point<3, T> ImgAsPoint() const { return Elite::Point<3, T>(data[1], data[2], data[3]); };
	inline Elite::Vector<3, T> ImgAsVector() const { return Elite::Vector<3, T>(data[1], data[2], data[3]); };

	template<typename U>
	inline Quaternion& operator *=(U scalar)
	{
		data[0] *= static_cast<T>(scalar);
		data[1] *= static_cast<T>(scalar);
		data[2] *= static_cast<T>(scalar);
		data[3] *= static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	inline Quaternion& operator /=(U scalar) 
	{  
		data[0] /= static_cast<T>(scalar);
		data[1] /= static_cast<T>(scalar);
		data[2] /= static_cast<T>(scalar);
		data[3] /= static_cast<T>(scalar);
		return *this;
	}

	template<typename U>
	inline Quaternion operator *(const Quaternion<U> rhs) const
	{
		return Quaternion<T>{ data[0] * static_cast<T>(rhs.data[0]) - data[1] * static_cast<T>(rhs.data[1]) - data[2] * static_cast<T>(rhs.data[2]) - data[3] * static_cast<T>(rhs.data[3])
							, data[0] * static_cast<T>(rhs.data[1]) + data[1] * static_cast<T>(rhs.data[0]) + data[2] * static_cast<T>(rhs.data[3]) - data[3] * static_cast<T>(rhs.data[2])
							, data[0] * static_cast<T>(rhs.data[2]) + data[2] * static_cast<T>(rhs.data[0]) - data[1] * static_cast<T>(rhs.data[3]) + data[3] * static_cast<T>(rhs.data[1])
							, data[0] * static_cast<T>(rhs.data[3]) + data[3] * static_cast<T>(rhs.data[0]) + data[1] * static_cast<T>(rhs.data[2]) - data[2] * static_cast<T>(rhs.data[1])};
	}
	template<typename U>
	inline Quaternion operator *(U scalar) const { return Quaternion<T>{ *this } *= scalar; }
	template<typename U>
	inline Quaternion operator /(U scalar) const { return Quaternion<T>{ *this } /= scalar; }

	inline T SqrMagnitude() const { return data[0] * data[0] + data[1] * data[1] + data[2] * data[2] + data[3] * data[3];  }
	inline T Magnitude() const { return static_cast<T>(sqrt(SqrMagnitude())); }
	inline Quaternion<T> Inverse() const { return Conjugate() /= (data[0] * data[0] + SqrMagnitude()); }
	inline Quaternion<T> Conjugate() const { return Quaternion<T>{ data[0], -data[1], -data[2], -data[3] }; }
};

template<typename T>
inline void Normalize(Quaternion<T>& q) { q /= q.Magnitude(); }

//Rodriguez formula - wikipedia says it uses less calculation than qvq'

template<typename T>
inline void Rotate(Elite::Matrix<4, 4, T>& m, float radians, const Elite::Vector<3, T>& unitAxis)
{
	Rotate(m, Quaternion<T>(radians, unitAxis));
}

template<typename T>
inline void Rotate(Elite::Matrix<4, 4, T>& m, const Quaternion<T>& q)
{
	Elite::Vector<3, T> img{ q.data[1], q.data[2], q.data[3] };
	m[0] += Cross((img + img), (Cross(img, Elite::Vector<3, T>(m[0])) + q.data[0] * Elite::Vector<3, T>(m[0])));
	m[1] += Cross((img + img), (Cross(img, Elite::Vector<3, T>(m[1])) + q.data[0] * Elite::Vector<3, T>(m[1])));
	m[2] += Cross((img + img), (Cross(img, Elite::Vector<3, T>(m[2])) + q.data[0] * Elite::Vector<3, T>(m[2])));
}

template<typename T>
inline void Rotate(Elite::Vector<4, T>& v, const Quaternion<T>& q)
{
	Elite::Vector<3, T> img{ q.data[1], q.data[2], q.data[3] };
	v += Cross((img + img), (Cross(img, Elite::Vector<3, T>(v)) + q.data[0] * Elite::Vector<3, T>(v)));
}

template<typename T>
inline void Rotate(Elite::Vector<3, T>& v, float radians, const Elite::Vector<3, T>& unitAxis)
{
	Rotate(v, Quaternion<T>(radians, unitAxis));
}

template<typename T>
inline void Rotate(Elite::Vector<3, T>& v, const Quaternion<T>& q)
{
	Elite::Vector<3, T> img{ q.data[1], q.data[2], q.data[3] };
	v += Cross((img + img), (Cross(img, v) + q.data[0] * v));
}


