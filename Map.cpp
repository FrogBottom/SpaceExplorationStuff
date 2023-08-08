#include "Core.h"

#define EPSILON 0.0000001

bool Intersect(Vec3 d, Vec3 v0, Vec3 v1, Vec3 v2, Vec3* out)
{
	Vec3 e1 = v1 - v0;
	Vec3 e2 = v2 - v0;
	Vec3 h = Cross(d, e2);
	double a = Dot(e1, h);

	if (a > -EPSILON && a < EPSILON) return false;

	double f = 1.0 / a;
	//Vec3 s = -v0;
	double u = f * Dot(-v0, h);
	if (u < 0.0 || u > 1.0) return false;

	Vec3 q = Cross(-v0, e1);
	double v = f * Dot(d, q);
	if (v < 0.0 || u + v > 1.0) return false;

	double t = f * Dot(e2,q);
	if (t > EPSILON)
	{
		*out = d * t;
		return true;
	}
	else return false;
}

int rayIntersectsTriangle(float *p, float *d,
			float *v0, float *v1, float *v2) {

	float e1[3],e2[3],h[3],s[3],q[3];
	float a,f,u,v;
	vector(e1,v1,v0);
	vector(e2,v2,v0);

	crossProduct(h,d,e2);
	a = innerProduct(e1,h);

	if (a > -0.00001 && a < 0.00001)
		return(false);

	f = 1/a;
	vector(s,p,v0);
	u = f * (innerProduct(s,h));

	if (u < 0.0 || u > 1.0)
		return(false);

	crossProduct(q,s,e1);
	v = f * innerProduct(d,q);

	if (v < 0.0 || u + v > 1.0)
		return(false);

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	t = f * innerProduct(e2,q);

	if (t > 0.00001) // ray intersection
		return(true);

	else // this means that there is a line intersection
		 // but not a ray intersection
		 return (false);

}