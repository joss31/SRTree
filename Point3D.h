#ifndef _POINT_3D_H_
#define _POINT_3D_H_

#include <math.h>
typedef double REAL;
#define REAL_MAX  DBL_MAX
#define REAL_TINY DBL_EPSILON
#define REAL_MIN  -DBL_MAX

//  EPSILON
#define EPSILON 1E-10
#define EPSILON_LARGE 1E-5


//  Tipo
struct Point3D{
  REAL x, y, z;

  __inline void assign(REAL xN, REAL yN, REAL zN){
    x = xN;
    y = yN;
    z = zN;
    }

  __inline void assign(const Point3D &p){
    x = p.x;
    y = p.y;
    z = p.z;
    }

  __inline bool equals(const Point3D &p) const{
    return fabs(x - p.x) < EPSILON && fabs(y - p.y) < EPSILON && fabs(z - p.z) < EPSILON;
    }

  //  distancia
  __inline REAL distanceSQR(const Point3D &p) const{
    REAL dX = p.x - x;
    REAL dY = p.y - y;
    REAL dZ = p.z - z;

    return dX*dX + dY*dY + dZ*dZ;
    }

  __inline REAL distance(const Point3D &p) const{
    return (REAL)sqrt(distanceSQR(p));
    }

  __inline void storeMax(const Point3D &p){
    if (p.x > x)
      x = p.x;
    if (p.y > y)
      y = p.y;
    if (p.z > z)
      z = p.z;
    }

  __inline void storeMin(const Point3D &p){
    if (p.x < x)
      x = p.x;
    if (p.y < y)
      y = p.y;
    if (p.z < z)
      z = p.z;
    }

  const static Point3D ZERO;
  const static Point3D MAX;
  const static Point3D MIN;
};



#endif
