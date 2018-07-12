#include "SSTree.h"

#include <stdio.h>
#include <string.h>

void SSTree::initNode(int node, int level){
    if (level < 0){
        int lev;
        int start = 0, num = 1;
        for (lev = 0; lev <= levels; lev++){
            if (node >= start && node < start+num)
                break;
            
            start += num;
            num *= degree;
        }
        level = lev+1;
    }
    
    // node
    STSphere *s = &nodes.index(node);
    s->c.x = s->c.y = s->c.z = 0.0f;
    s->r = -1.0;
    s->errDec = -1;
    s->hasAux = false;
    s->occupancy = 1.0;
    //Childrens si o no
    if (level < levels){
        int firstChild = node*degree + 1;
        for (int i = 0; i < degree; i++)
            initNode(firstChild+i, level+1);
    }
}

bool SSTree::saveSpheres(const Array<Sphere> &nodes, const char *fileName, float scale){
    int numSph = nodes.getSize();
    
    Sphere boundSphere;
    boundSphere.c = Point3D::ZERO;
    int numValid = 0;
    for (int i = 0; i < numSph; i++){
        Sphere s = nodes.index(i);
        if (s.r > 0){
            boundSphere.c.x += s.c.x;
            boundSphere.c.y += s.c.y;
            boundSphere.c.z += s.c.z;
            numValid++;
        }
    }
    boundSphere.c.x /= numValid;
    boundSphere.c.y /= numValid;
    boundSphere.c.z /= numValid;
    
    //  radio
    boundSphere.r = 0;
    for (int i = 0; i < numSph; i++){
        Sphere s = nodes.index(i);
        if (s.r > 0){
            float r = boundSphere.c.distance(s.c) + s.r;
            if (r > boundSphere.r)
                boundSphere.r = s.r;
        }
    }

    /*FILE *f = fopen(fileName, "w");
    if (!f)
        return false;
    
    fprintf(f, "%d %d\n", 2, numValid);
    
    
    fprintf(f, "%f %f %f %f\n", boundSphere.c.x*scale, boundSphere.c.y*scale, boundSphere.c.z*scale, boundSphere.r*scale);
    
    for (int i = 0; i < numSph; i++){
        Sphere s = nodes.index(i);
        if (s.r > 0)
            fprintf(f, "%f %f %f %f\n", s.c.x*scale, s.c.y*scale, s.c.z*scale, s.r*scale);
    };
    
    fclose(f);*/
    return true;
}

void SSTree::getLevel(Array<Sphere> *nodes, int level) const{
    unsigned long startI, numS;
    getRow(&startI, &numS, level);
    
    nodes->setSize(0);
    for (int i = 0; i < numS; i++){
        Sphere s = this->nodes.index(startI+i);
        if (s.r > 0)
            nodes->addItem() = s;
    }
}

void SSTree::setupTree(int deg, int levs){
    this->degree = deg;
    this->levels = levs;
    
    int total = 1 + deg;
    int nc = deg;
    for (int level = 2; level < levs; level++){
        nc *= deg;
        total += nc;
    }
    
    this->nodes.resize(total);
    initNode(0);
}

void SSTree::growTree(int levs){
    int total = 1 + degree;
    int nc = degree;
    for (int level = 2; level < levs; level++){
        nc *= degree;
        total += nc;
    }
    
    int oldSize = this->nodes.getSize();
    
    this->nodes.resize(total);
    this->levels = levs;
    
    for (int i = oldSize; i < total; i++){
        STSphere *s = &nodes.index(i);
        s->c.x = 0;
        s->c.y = 0;
        s->c.z = 0;
        s->r = -1;
        s->sAux.c = s->c;
        s->sAux.r = s->r;
        s->errDec = -1;
        s->hasAux = false;
        s->occupancy = 1.0;
    }
}

