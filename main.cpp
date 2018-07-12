#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <stdio.h>
#include "RStarTree.h"
#include <cstdlib>
#include "SSTree.h"
#include <fstream>
#define RANDOM_DATASET

using namespace std;


#ifdef RANDOM_DATASET
	typedef RStarTree<int, 2, 32, 64> 			RsTree;
#else
	typedef RStarTree<std::string, 2, 2, 3> 	RsTree;
#endif

typedef RsTree::BoundingBox			BoundingBox;


BoundingBox bounds(int x, int y, int w, int h)
{
	BoundingBox bb;

	bb.edges[0].first  = x;
	bb.edges[0].second = x + w;

	bb.edges[1].first  = y;
	bb.edges[1].second = y + h;

	return bb;
}


struct Visitor {
	int count;
	bool ContinueVisiting;

	Visitor() : count(0), ContinueVisiting(true) {};

	void operator()(const RsTree::Leaf * const leaf)
	{
#if defined( RANDOM_DATASET )
		//std::cout << "Visiting " << count << std::endl;
#elif defined( GUTTMAN_DATASET )
		std::cout << "#" << count << ": visited " << leaf->leaf << " with bound " << leaf->bound.ToString() << std::endl;
#else
		#error "Undefined dataset"
#endif
		count++;
	}
};



int main(int argc, char ** argv)
{

    RsTree tree;
	Visitor x;
    //int dimension, dataSize, blockSize;

    string cadena;
    string lineas;
    vector<double> atributos;
    string temp = "";
    ifstream fs("YearPredictionMSD.txt");
    for(int i=0;i<100;i++)
    {
        getline(fs, cadena);
        lineas += cadena;
        lineas += ",";

        for(int a = 0; a < lineas.size(); a++)
        {
            if(lineas[a] != ',')
            {
                temp += lineas[a];
            }
            else{
                atributos.push_back(strtod(temp.c_str(), 0));
                temp.clear();
            }
        }

        tree.Insert(i, bounds( atributos[1], atributos[2], atributos[3], atributos[4]));

        temp.clear();
        atributos.clear();

        lineas.clear();
    }


#ifdef RANDOM_DATASET
	srand(time(0));

	#define nodes 20000

	//for (int i = 0; i < nodes/2; i++)
		//tree.Insert(i, bounds( rand() % 1000, rand() % 1000, rand() % 10, rand() % 10));

	for (int i = 0; i < nodes/2; i++)
		 tree.Insert(i, bounds( rand() % 1000, rand() % 1000, rand() % 20, rand() % 20));

	BoundingBox bound = bounds( 100,100, 300,400 );

	x = tree.Query(RsTree::AcceptAny(), Visitor());
	std::cout << "AcceptAny: " << x.count << " nodes visited (" << tree.GetSize() << " nodes in tree)" << std::endl;


	std::cout << "Searching in " << bound.ToString() << std::endl;
	x = tree.Query(RsTree::AcceptEnclosing(bound), Visitor());
	std::cout << "Visited " << x.count << " nodes (" << tree.GetSize() << " nodes in tree)" << std::endl;


	std::cout << "Searching in " << bound.ToString() << std::endl;
	x = tree.Query(RsTree::AcceptEnclosing(bound), Visitor());
	std::cout << "Visited " << x.count << " nodes. (" << tree.GetSize() << " nodes in tree)" << std::endl;

	//tree.Print();

#endif


	return 0;
}
