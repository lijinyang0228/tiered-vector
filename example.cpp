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

#define MAX (5)


using namespace std;
using namespace Seq;

int main(int argc, char * argv[])
{

    cerr << endl << "Comparing time taken to do " << MAX << " insertions at pseudo random positions in Tiered Vector and STL Vector" << endl << endl;

    srand(0);
    //Seq::Tiered<int, LayerItr<LayerEnd, Layer<64, Layer<64, Layer<64>>>>> tiered;
    Seq::Tiered<int, LayerItr<LayerEnd, Layer<3, Layer<3>>>> tiered;
    // Start time
    clock_t begin = clock();

    // Do random insertions
    for (int i = 0; i < MAX; i++) {
        size_t idx = rand() % (i + 1);
        tiered.insert(idx, i + 1);
    }
    for (int j = 0; j < MAX; ++j) {
        cout<<tiered.operator[](j)<<" ";
    }
    cout<<endl;
    tiered.drawTree();

    tiered.insert(3, 30);
    tiered.drawTree();
    for (int j = 0; j < MAX+1; ++j) {
        cout<<tiered.operator[](j)<<" ";
    }
    cout<<endl;
    tiered.drawString();

    //tiered.print();

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
