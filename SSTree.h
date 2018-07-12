#ifndef _SPHERE_TREE_H_
#define _SPHERE_TREE_H_

#include "Sphere.h"
#include "Array.h"

struct STSphere : Sphere{
    bool hasAux;
    Sphere sAux;
    float errDec;
    float occupancy;
};


template <class T> class kTree{
public:
    unsigned long levels;
    unsigned long degree;
    Array<T> nodes;
    
    __inline unsigned long getParent(unsigned long node) const{
        return (node - 1)/ degree;
    }
    
    __inline unsigned long getFirstChild(unsigned long node) const{
        return node * degree + 1;
    }
    
    void getRow(unsigned long *pStart, unsigned long *pSize, unsigned long rowNum) const{
        unsigned long start = 0, num = 1;
        for (int i = 0; i < rowNum; i++){
            start += num;
            num *= degree;
        }
        *pStart = start;
        *pSize = num;
    }
};

class SSTree : public kTree<STSphere>{
public:

    void initNode(int node, int level = -1);
    void getLevel(Array<Sphere> *spheres, int level) const;
    
    void growTree(int levs);
    void setupTree(int deg, int levs);
    
    static bool saveSpheres(const Array<Sphere> &spheres, const char *fileName, float scale = 1.0f);
};

#endif
