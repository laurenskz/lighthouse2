/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#pragma once

// This file contains PBRT-specific functions

// BSDF Inline Functions
LH2_DEVFUNC float CosTheta( const float3& w ) { return w.z; }
LH2_DEVFUNC float Cos2Theta( const float3& w ) { return w.z * w.z; }
LH2_DEVFUNC float AbsCosTheta( const float3& w ) { return std::abs( w.z ); }
LH2_DEVFUNC float Sin2Theta( const float3& w )
{
	return std::max( 0.f, 1.f - Cos2Theta( w ) );
}

LH2_DEVFUNC float SinTheta( const float3& w ) { return std::sqrt( Sin2Theta( w ) ); }

LH2_DEVFUNC float TanTheta( const float3& w ) { return SinTheta( w ) / CosTheta( w ); }

LH2_DEVFUNC float Tan2Theta( const float3& w )
{
	return Sin2Theta( w ) / Cos2Theta( w );
}

LH2_DEVFUNC float CosPhi( const float3& w )
{
	float sinTheta = SinTheta( w );
	return ( sinTheta == 0.f ) ? 1.f : clamp( w.x / sinTheta, -1.f, 1.f );
}

LH2_DEVFUNC float SinPhi( const float3& w )
{
	float sinTheta = SinTheta( w );
	return ( sinTheta == 0.f ) ? 0.f : clamp( w.y / sinTheta, -1.f, 1.f );
}

LH2_DEVFUNC float Cos2Phi( const float3& w ) { return CosPhi( w ) * CosPhi( w ); }

LH2_DEVFUNC float Sin2Phi( const float3& w ) { return SinPhi( w ) * SinPhi( w ); }

LH2_DEVFUNC float CosDPhi( const float3& wa, const float3& wb )
{
	return clamp(
		( wa.x * wb.x + wa.y * wb.y ) / std::sqrt( ( wa.x * wa.x + wa.y * wa.y ) *
												   ( wb.x * wb.x + wb.y * wb.y ) ),
		-1.f, 1.f );
}

LH2_DEVFUNC float3 pbrt_Reflect( const float3& wo, const float3& n )
{
	return -wo + 2 * dot( wo, n ) * n;
}

LH2_DEVFUNC bool SameHemisphere( const float3& w, const float3& wp )
{
	return w.z * wp.z > 0;
}

// ----------------------------------------------------------------

// PBRT Uses t-first. Provide a function for clarity.
template <typename F, typename T>
LH2_DEVFUNC T pbrt_Lerp( F t, T a, T b )
{
	return lerp( a, b, t );
}

LH2_DEVFUNC float AbsDot( const float3& v1, const float3& v2 )
{
	return std::abs( dot( v1, v2 ) );
}

LH2_DEVFUNC float3 SphericalDirection( float sinTheta, float cosTheta, float phi )
{
	return make_float3( sinTheta * std::cos( phi ), sinTheta * std::sin( phi ),
						cosTheta );
}

// LH2_DEVFUNC float3 SphericalDirection( float sinTheta, float cosTheta, float phi,
// 									   const float3& x, const float3& y,
// 									   const float3& z )
// {
// 	return sinTheta * std::cos( phi ) * x + sinTheta * std::sin( phi ) * y +
// 		   cosTheta * z;
// }

LH2_DEVFUNC float SphericalTheta( const float3& v )
{
	return std::acos( clamp( v.z, -1.f, 1.f ) );
}

LH2_DEVFUNC float SphericalPhi( const float3& v )
{
	float p = std::atan2( v.y, v.x );
	return ( p < 0 ) ? ( p + 2.f * PI ) : p;
}

// ----------------------------------------------------------------

LH2_DEVFUNC float ErfInv( float x )
{
	float w, p;
	x = clamp( x, -.99999f, .99999f );
	w = -std::log( ( 1.f - x ) * ( 1.f + x ) );
	if ( w < 5.f )
	{
		w = w - 2.5f;
		p = 2.81022636e-08f;
		p = 3.43273939e-07f + p * w;
		p = -3.5233877e-06f + p * w;
		p = -4.39150654e-06f + p * w;
		p = 0.00021858087f + p * w;
		p = -0.00125372503f + p * w;
		p = -0.00417768164f + p * w;
		p = 0.246640727f + p * w;
		p = 1.50140941f + p * w;
	}
	else
	{
		w = std::sqrt( w ) - 3.f;
		p = -0.000200214257f;
		p = 0.000100950558f + p * w;
		p = 0.00134934322f + p * w;
		p = -0.00367342844f + p * w;
		p = 0.00573950773f + p * w;
		p = -0.0076224613f + p * w;
		p = 0.00943887047f + p * w;
		p = 1.00167406f + p * w;
		p = 2.83297682f + p * w;
	}
	return p * x;
}

LH2_DEVFUNC float Erf( float x )
{
	// constants
	const float a1 = 0.254829592f;
	const float a2 = -0.284496736f;
	const float a3 = 1.421413741f;
	const float a4 = -1.453152027f;
	const float a5 = 1.061405429f;
	const float p = 0.3275911f;

	// Save the sign of x
	int sign = 1.f;
	if ( x < 0 ) sign = -1.f;
	x = std::abs( x );

	// A&S formula 7.1.26
	const float t = 1.f / ( 1.f + p * x );
	const float y =
		1.f -
		( ( ( ( ( a5 * t + a4 ) * t ) + a3 ) * t + a2 ) * t + a1 ) * t * std::exp( -x * x );

	return sign * y;
}

// ----------------------------------------------------------------

// BxDF Utility Functions
LH2_DEVFUNC float FrDielectric( float cosThetaI, float etaI, float etaT )
{
	cosThetaI = clamp( cosThetaI, -1.f, 1.f );
	// Potentially swap indices of refraction
	bool entering = cosThetaI > 0.f;
	if ( !entering )
	{
		// std::swap( etaI, etaT );
		auto tmp = etaI;
		etaI = etaT;
		etaT = tmp;
		cosThetaI = std::abs( cosThetaI );
	}

	// Compute _cosThetaT_ using Snell's law
	float sinThetaI = std::sqrt( std::max( 0.f, 1 - cosThetaI * cosThetaI ) );
	float sinThetaT = etaI / etaT * sinThetaI;

	// Handle total internal reflection
	if ( sinThetaT >= 1 ) return 1;
	float cosThetaT = std::sqrt( std::max( 0.f, 1 - sinThetaT * sinThetaT ) );
	float Rparl = ( ( etaT * cosThetaI ) - ( etaI * cosThetaT ) ) /
				  ( ( etaT * cosThetaI ) + ( etaI * cosThetaT ) );
	float Rperp = ( ( etaI * cosThetaI ) - ( etaT * cosThetaT ) ) /
				  ( ( etaI * cosThetaI ) + ( etaT * cosThetaT ) );
	return ( Rparl * Rparl + Rperp * Rperp ) / 2;
}