//
// Created by laurens on 11/12/20.
//

#include <graphics/raytracer.h>

#include "core_settings.h"
namespace lh2core
{

ostream& operator<<( ostream& os, const float3& s )
{
	return ( os << "{" << s.x << "," << s.y << "," << s.z << "}" << std::endl );
}

float3 RayTracer::rayDirection( float u, float v, const ViewPyramid& view )
{

	return normalize( ( screenPos( u, v, view ) - view.pos ) );
}
float3 RayTracer::trace( Ray& r, int count )
{
	if ( count <= 0 ) return BLACK; //Recursion limit
	r.t = MAX_DISTANCE;
	auto intersection = environment->intersect( r );
	if ( !intersection.hitObject )
		return environment->skyColor( r.direction );
	if ( intersection.mat.type == DIFFUSE )
	{
		return computeDiffuseColor( intersection );
	}
	if ( intersection.mat.type == SPECULAR && intersection.mat.specularity > 0 )
	{
		return computeSpecularColor( r, count, intersection );
	}
	if ( intersection.mat.type == GLASS )
	{
		return computeGlassColor( r, count, intersection );
	}
	return BLACK;
}

void RayTracer::traceTo( Ray* rays, Intersection* intersections, float3* buffer, int rayCount, int recursionCount )
{
	//	if ( recursionCount <= 0 )
	//	{
	//		for ( int i = 0; i < rayCount; ++i )
	//		{
	//			buffer[i] = BLACK; //Recursion limit
	//		}
	//	}
	//	environment->insersectTo( rays, rayCount, intersections );
	//	for ( int i = 0; i < rayCount; ++i )
	//	{
	//		if ( !intersections[i].hitObject )
	//		{
	//			rays[i].alive = false;
	//			buffer[i] = make_float3( 0.529, 0.808, 0.929 ); //Black if nothing hit
	//			continue;
	//		}
	//	}
	//	if ( intersection.mat.type == DIFFUSE )
	//	{
	//		return computeDiffuseColor( intersection );
	//	}
	//	if ( intersection.mat.type == SPECULAR && intersection.mat.specularity > 0 )
	//	{
	//		return computeSpecularColor( r, count, intersection );
	//	}
	//	if ( intersection.mat.type == GLASS )
	//	{
	//		return computeGlassColor( r, count, intersection );
	//	}
	//	return BLACK;
}

float3 RayTracer::computeGlassColor( const Ray& r, int count, Intersection& intersection )
{
	Ray refracted;
	Ray reflected;
	float reflectivityFraction;
	calculateGlass( reflected, refracted, reflectivityFraction, r, intersection );
	if ( reflectivityFraction < 1 )
	{
		return ( 1 - reflectivityFraction ) * trace( refracted, count - 1 ) + reflectivityFraction * trace( reflected, count - 1 );
	}
	return trace( reflected, count - 1 );
}

inline float schlick( float n1, float n2, float cosTheta )
{
	float r0 = sqr( ( n1 - n2 ) / ( n1 + n2 ) );
	return r0 + ( 1 - r0 ) * pow( 1 - cosTheta, 5 );
}
void calculateGlass( Ray& reflected, Ray& refracted, float& reflectivityFraction, const Ray& r, const Intersection& intersection )
{
	float cosTheta1 = dot( intersection.normal, -r.direction );
	bool rayEntersObject = cosTheta1 > 0;
	cosTheta1 = rayEntersObject ? cosTheta1 : -cosTheta1;
	auto normal = rayEntersObject ? intersection.normal : -intersection.normal;
	float n1 = rayEntersObject ? 1 : intersection.mat.refractionIndex;
	float n2 = rayEntersObject ? intersection.mat.refractionIndex : 1;
	float n1overn2 = n1 / n2;
	float k = 1 - ( n1overn2 * n1overn2 ) * ( 1 - cosTheta1 );
	reflectivityFraction = schlick( n1, n2, cosTheta1 );
	const Intersection& newIntersection = Intersection{ intersection.location, normal, intersection.mat, intersection.hitObject };
	if ( k >= 0 ) //Refraction and reflection
	{
		float3 T = n1overn2 * r.direction + normal * ( n1overn2 * cosTheta1 - sqrt( k ) );
		refracted = Ray{ intersection.location + 1e-3 * T, T };
	}
	else
	{
		reflectivityFraction = 1;
	}
	const float3& reflectedDirection = reflect( r.direction, normal );
	reflected = Ray{ intersection.location + ( 1e-3 * reflectedDirection ), reflectedDirection };
}

float3 RayTracer::computeSpecularColor( const Ray& r, int count, Intersection& intersection )
{
	const float3& diffuseColor = ( 1 - intersection.mat.specularity ) * computeDiffuseColor( intersection );
	const float3& reflectionColor = traceReflectedRay( r, count, intersection );
	return diffuseColor + intersection.mat.specularity * reflectionColor;
}
float3 RayTracer::traceReflectedRay( const Ray& r, int count, Intersection& intersection )
{
	Ray reflectedRay = reflect( intersection, r );
	const float3& reflectionColor = trace( reflectedRay, count - 1 );
	return reflectionColor;
}
Ray RayTracer::reflect( const Intersection& intersection, const Ray& r )
{
	const float3& direction = reflect( r.direction, intersection.normal );
	const Ray& reflectedRay = Ray{ intersection.location + ( 1e-3 * direction ), direction };
	return reflectedRay;
}
float3 RayTracer::computeDiffuseColor( const Intersection& intersection )
{
	auto illumination = lighting->directIllumination( intersection.location, intersection.normal );
	auto diffuseColor = illumination * intersection.mat.color;
	return diffuseColor;
}

inline float3 RayTracer::screenPos( float u, float v, const ViewPyramid& view )
{
	return view.p1 + u * ( view.p2 - view.p1 ) + v * ( view.p3 - view.p1 );
}
float3 RayTracer::reflect( const float3& direction, const float3& normal )
{
	return normalize( direction - ( ( 2 * dot( direction, normal ) ) * normal ) );
}

float3 PathTracer::trace( Ray& r, int count )
{
	r.t = MAX_DISTANCE;
	if ( count <= 0 ) return BLACK; //Recursion limit
	auto intersection = environment->intersect( r );
	if ( !intersection.hitObject )
		return environment->skyColor( r.direction );
	if ( intersection.mat.type == LIGHT )
	{
		return intersection.mat.color;
	}
	float dice = randFloat( 0, 1 );
	bool goDiffuse = intersection.mat.type == DIFFUSE || ( intersection.mat.type == SPECULAR && intersection.mat.specularity < dice );
	bool goReflected = ( intersection.mat.type == SPECULAR && !goDiffuse );
	if ( goDiffuse )
	{
		float3 newDirection;
		do {
			newDirection = randomDirectionFrom( intersection.normal );
		} while ( isnan( newDirection.x ) );
		Ray newRay{ intersection.location + 1e-3 * newDirection, newDirection };
		const float3& brdf = intersection.mat.color / PI;
		float3 hit = trace( newRay, count - 1 ) * dot( newDirection, intersection.normal );
		return hit * PI * 2.0 * brdf;
	}

	if ( goReflected )
	{
		Ray ray = RayTracer::reflect( intersection, r );
		return trace( ray, count - 1 );
	}
	if ( intersection.mat.type == GLASS )
	{
		Ray refracted;
		Ray reflected;
		float reflectivityFraction;
		calculateGlass( reflected, refracted, reflectivityFraction, r, intersection );
		if ( dice < reflectivityFraction )
		{
			return trace( reflected, count - 1 );
		}
		return trace( refracted, count - 1 );
	}
	return make_float3( 0.529, 0.808, 0.929 );
}
float PathTracer::randFloat( float min, float max )
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * ( max - min );		/* [min, max] */
}

float3 PathTracer::randomDirectionFrom( const float3& normal )
{
	float3 nt, nb;
	if ( std::fabs( normal.x ) > std::fabs( normal.y ) )
		nt = make_float3( normal.z, 0, -normal.x ) / sqrtf( normal.x * normal.x + normal.z * normal.z );
	else
		nt = make_float3( 0, -normal.z, normal.y ) / sqrtf( normal.y * normal.y + normal.z * normal.z );
	nb = cross( normal, nt );
	const float3 random = randomHemisphereDirection();
	return make_float3(
		random.x * nb.x + random.y * normal.x + random.z * nt.x,
		random.x * nb.y + random.y * normal.y + random.z * nt.y,
		random.x * nb.z + random.y * normal.z + random.z * nt.z );
}
float3 PathTracer::randomHemisphereDirection()
{
	float r1 = randFloat( 0, 1 );
	float r2 = randFloat( 0, 1 );
	float sinTheta = sqrtf( 1 - r1 * r1 );
	float phi = 2 * M_PI * r2;
	float x = sinTheta * cosf( phi );
	float z = sinTheta * sinf( phi );
	float u1 = sqrt( 1 - ( x * x ) - ( z * z ) );
	return make_float3( x, u1, z );
}

} // namespace lh2core
