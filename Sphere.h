#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "Point3D.h"
#include "Vector3D.h"

#include <math.h>

struct Sphere{
  Point3D c;
  REAL r;

  __inline void assign(const Point3D &C, float R){
    c = C;
    r = R;
    }

  __inline bool contains(const Point3D &pt, float tol = EPSILON) const{
    return (pt.distanceSQR(c) <= r*r + tol);
    }

  __inline bool overlap(const Sphere &s, float tol = EPSILON) const{
    double sR = s.r + r;
    return s.c.distanceSQR(c) <= sR*sR + tol;
    }

  __inline double volume() const{
    return 4.0/3.0*M_PI*r*r*r;
    }

  float overlapVolume(const Sphere &other) const;

  int intersectRay(double t[2], const Point3D &rayOrigin, const Vector3D &rayDirn) const;

  bool intersect(const Point3D &pMin, const Point3D &pMax) const;

  const static Sphere ZERO;
  const static Sphere UNIT;
  const static Sphere INVALID;
};

#endif
