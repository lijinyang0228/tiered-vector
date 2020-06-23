#include <iostream>
#include <fstream>
#include <chrono>
#include "../HeaderFiles/StandardArray.h"
#include "../HeaderFiles/templated_tiered.h"
using namespace std;
using namespace Seq;
typedef std::chrono::high_resolution_clock::time_point TimeVar;
#define duration(a) std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()

bool CompareArray(const int *a, const int *b, int len) {
    if (a == nullptr || b == nullptr) {
        cout<<"null pointer error!" << endl;
        return 0 ;
    }
    for (int i = 0; i < len; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}


int RandomInt(int s, int e) {
    return (rand() % (e-s+1))+ s;
}


int main() {
    int length = 50;
    int toInsert = length+1;
    int * AnsSA, * AnsTV, * AnsDANL;
    int * array = new int[length];
    for (int i = 0; i < length; ++i) {
        array[i] = i+1;
    }

    Seq::Tiered<int, LayerItr<LayerEnd, Layer<4, Layer<4, Layer<4>>>>> tiered;
    tiered.initialize(array, length);
    StandardArray *sa = NewStandardArray(array, length);
    delete []array;
    int NumAnsTV, NumAnsSA, NumAnsDANL;
    int k = 0;
    int index[4];
    for (k = 0; k < 1000; ++k) {
        //cout<<"---------------------------------------------------- k =  " <<k<<endl;
        index[0] = RandomInt(1, length);
        index[1] = RandomInt(1, length);
        index[2] = RandomInt(1, length);
        index[3] = RandomInt(1, length);
        sort(index, index+4);
        //printf("swap %d %d %d %d\n", index[0], index[1], index[2], index[3]);

        if (index[1] == index[2]) {
            continue;
        }
        sa->Swap(index[0], index[1], index[2], index[3]);
        //sa->PrintArray();
        //tiered.drawTree();
        tiered.Swap(index[0], index[1], index[2], index[3]);
        //tiered.drawTree();
        //tiered.drawString();


        AnsTV = tiered.RangeQuery(1, length, NumAnsTV);
        AnsSA = sa->RangeQuery(1, length, &NumAnsSA);
        if (!CompareArray(AnsTV, AnsSA, NumAnsSA)) {
            printf("============================ k=%d, not equal=========================\n", k);
            break;
        }

    }

    for (k = 0; k < 1; ++k) {
        //cout<<"---------------------------------------------------- k =  " <<k<<endl;
        index[0] = RandomInt(1, length);
        index[1] = RandomInt(1, length);
        index[2] = RandomInt(1, length);
        index[3] = RandomInt(1, length);
        sort(index, index+4);
        //printf("swap %d %d %d %d\n", index[0], index[1], index[2], index[3]);

        if (index[1] == index[2]) {
            continue;
        }

        sa->Swap(index[0], index[1], index[2], index[3]);

        tiered.Swap(index[0], index[1], index[2], index[3]);

        AnsTV = tiered.RangeQuery(1, length, NumAnsTV);
        AnsSA = sa->RangeQuery(1, length, &NumAnsSA);
        if (!CompareArray(AnsTV, AnsSA, NumAnsSA)) {
            printf("============================ k=%d, not equal=========================\n", k);
            break;
        }
    }
    //fclose(f);
    return 0;
}
