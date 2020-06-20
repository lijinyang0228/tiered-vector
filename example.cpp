#define _XOPEN_SOURCE 600
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include "templated_tiered.h" 
#include <stdio.h>
#include <stdlib.h>
#include <iostream>  


#include <ctime>
#include <vector>

#define MAX (27)
//# max=262144=64*64*64


using namespace std;
using namespace Seq;

int main(int argc, char * argv[])
{

    cerr << endl << "Comparing time taken to do " << MAX << " insertions at pseudo random positions in Tiered Vector and STL Vector" << endl << endl;

    srand(0);
    Seq::Tiered<int, LayerItr<LayerEnd, Layer<3, Layer<3, Layer<3>>>>> tiered;
    //Seq::Tiered<int, LayerItr<LayerEnd, Layer<3, Layer<3>>>> tiered;

    // Start time
    clock_t begin = clock();

    // Do random insertions
    for (int i = 1; i < MAX; i++) {
        //size_t idx = rand() % (i + 1);
        tiered.insert(i, i + 1);
    }
    tiered.drawTree();
    tiered.drawString();
    for (int j = 27; j < 216; ++j) {
        tiered.insert(j, j+1);
    }

    //tiered.remove(4);
    tiered.drawTree();
    tiered.drawString();




    // Stop time
    clock_t end = clock();

    // Print time
    cerr << double(end - begin) / CLOCKS_PER_SEC << " tiered vector" << endl << endl;

    vector<int> vec(MAX);
    
    // Start time
    begin = clock();

    // Do random insertions
    for (int i = 0; i < MAX; i++) {
        size_t idx = rand() % (i + 1);
        vec.insert(vec.begin() + idx, i + 1);
    }

    // Stop time
    end = clock();

    // Print time
    cerr << double(end - begin) / CLOCKS_PER_SEC << " vector" << endl;

    return 0;
}
