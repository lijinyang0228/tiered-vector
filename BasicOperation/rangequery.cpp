#include <iostream>
#include <fstream>
#include <chrono>
#include "../HeaderFiles/StandardArray.h"
#include "../HeaderFiles/2tiered.h"
using namespace std;
using namespace Seq;
typedef std::chrono::high_resolution_clock::time_point TimeVar;
#define duration(a) std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()

bool CompareArray(const int *a, const int *b, int len) {
    if (a == nullptr || b == nullptr) {
        cout<<"null pointer error!" << endl;
        return false;
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
    int length = 1000;
    int toInsert = length+1;
    int * AnsSA, * AnsTV, * AnsDANL;
    int * array = new int[length];
    for (int i = 0; i < length; ++i) {
        array[i] = i+1;
    }

    Seq::Tiered<int, LayerItr<LayerEnd, Layer<10, Layer<10, Layer<10>>>>> tiered;
    tiered.initialize(array, length);
    StandardArray *sa = NewStandardArray(array, length);
    //tiered.insert(10, 21);
    //tiered.drawTree();
    delete []array;
    int NumAnsTV, NumAnsSA;
    int k = 0, queryLength = 600;
    for (k = 0; k < 800; ++k) {
        int start = RandomInt(1, length-queryLength-3);
        int end = start + queryLength - 1;
        AnsSA = sa->RangeQuery(start, end, &NumAnsSA);

        TimeVar time1 = timeNow();
        AnsTV = tiered.RangeQuery(start, end, NumAnsTV);
        TimeVar time2 = timeNow();
        printf("range query time %lld\n", duration(time2-time1));


        if (!CompareArray(AnsTV, AnsSA, NumAnsTV)) {
            cout<< "================================= insert not equal================================"<<endl;
            break;
        }
    }

    for (k = 0; k < 1; ++k) {
        int start = RandomInt(1, length-queryLength-3);
        int end = start + queryLength - 1;
        AnsSA = sa->RangeQuery(start, end, &NumAnsSA);

        printf("query %d - %d\n", start, end);

        TimeVar time1 = timeNow();
        AnsTV = tiered.RangeQuery(start, end, NumAnsTV);
        TimeVar time2 = timeNow();
        printf("range query time %lld\n", duration(time2-time1));


        if (!CompareArray(AnsTV, AnsSA, NumAnsTV)) {
            cout<< "================================= insert not equal================================"<<endl;
            break;
        }
    }
    //fclose(f);
    return 0;
}
