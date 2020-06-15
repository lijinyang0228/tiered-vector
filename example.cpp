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

#define MAX (9)
//# max=262144=64*64*64


using namespace std;
using namespace Seq;

int main(int argc, char * argv[])
{

    cerr << endl << "Comparing time taken to do " << MAX << " insertions at pseudo random positions in Tiered Vector and STL Vector" << endl << endl;

    srand(0);
    //Seq::Tiered<int, LayerItr<LayerEnd, Layer<3, Layer<3, Layer<3>>>>> tiered;
    Seq::Tiered<int, LayerItr<LayerEnd, Layer<3, Layer<3>>>> tiered;

    // Start time
    clock_t begin = clock();

    // Do random insertions
    for (int i = 0; i < MAX; i++) {
        //size_t idx = rand() % (i + 1);
        tiered.insert(i, i + 1);
    }
    tiered.drawTree();
    tiered.drawString();

    tiered.remove(8);
    tiered.drawTree();
    tiered.drawString();
    tiered.remove(5);
    tiered.drawTree();
    tiered.drawString();
    tiered.remove(3);
    tiered.drawTree();
    tiered.drawString();
    tiered.insert(4,100);
    tiered.drawTree();
    tiered.drawString();

    for (int j = 0; j < MAX-2; ++j) {
        printf("%d ", tiered.operator[](j));
    }
    printf("\n");

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
