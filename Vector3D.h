#ifndef _VECTOR_3D_H_
#define _VECTOR_3D_H_

#include <stdlib.h>

#include "Point3D.h"

struct Vector3D{
  REAL x, y, z;

  __inline void difference(const Point3D &p, const Point3D &q){
    x = p.x - q.x;
    y = p.y - q.y;
    z = p.z - q.z;
    }

  __inline void assign(REAL xN, REAL yN, REAL zN){
    x = xN;
    y = yN;
    z = zN;
    }

  __inline void assign(const Vector3D &v){
    x = v.x;
    y = v.y;
    z = v.z;
    }

  __inline void assign(const Point3D &p){
    x = p.x;
    y = p.y;
    z = p.z;
    }

  __inline bool equals(const Vector3D &v) const{
    return (fabs(v.x - x) < EPSILON && fabs(v.y - y) < EPSILON && fabs(v.z - z) < EPSILON);
    }

  __inline void add(Point3D *p, REAL mult = 1.0f) const{
    p->x += x*mult;
    p->y += y*mult;
    p->z += z*mult;
    }

  __inline void add(Point3D *p, const Point3D &q, REAL mult = 1.0f) const{

    p->x = q.x + x*mult;
    p->y = q.y + y*mult;
    p->z = q.z + z*mult;
    }

  __inline void sub(Point3D *p) const{

    p->x -= x;
    p->y -= y;
    p->z -= z;
    }

  __inline void sub(Point3D *p, const Point3D &q) const{

    p->x = q.x - x;
    p->y = q.y - y;
    p->z = q.z - z;
    }

  __inline void add(const Vector3D &v, REAL mult = 1.0f){
    x += v.x*mult;
    y += v.y*mult;
    z += v.z*mult;
    }

  __inline void add(const Vector3D &v1, const Vector3D &v2, REAL mult = 1.0f){
    x = v1.x + v2.x*mult;
    y = v1.y + v2.y*mult;
    z = v1.z + v2.z*mult;
    }

  __inline void sub(const Vector3D &v){
    x -= v.x;
    y -= v.y;
    z -= v.z;
    }

  __inline void sub(const Vector3D &v, const Vector3D &v1){
    x = v.x - v1.x;
    y = v.y - v1.y;
    z = v.z - v1.z;
    }

  __inline void scale(REAL s){
    x *= s;
    y *= s;
    z *= s;
    }

  __inline void scale(const Vector3D &v, REAL s){
    x = v.x * s;
    y = v.y * s;
    z = v.z * s;
    }

  __inline REAL dot(const Vector3D &v) const{
    return x*v.x + y*v.y + z*v.z;
    }

  __inline void cross(const Vector3D &v1, const Vector3D &v2){
    x = v1.y*v2.z - v1.z*v2.y;
    y = v1.z*v2.x - v1.x*v2.z;
    z = v1.x*v2.y - v1.y*v2.x;
    }

  __inline REAL mag() const{
    return (REAL)sqrt(x*x + y*y + z*z);
    }

  __inline REAL magSQR() const{
    return (REAL)(x*x + y*y + z*z);
    }

  REAL getAngle(const Vector3D &v) const;

  void norm();
  void norm(const Vector3D &v);

  const static Vector3D ZERO;
  const static Vector3D X;
  const static Vector3D Y;
  const static Vector3D Z;
};

#endif
