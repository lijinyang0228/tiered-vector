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
    for (k = 0; k < 500; ++k) {
        int pos = RandomInt(1, length);
        sa->Insert(toInsert, pos);
        tiered.insert(pos, toInsert);
        printf("insert k=%d, pos=%d, toInsert = %d\n", k, pos, toInsert);
        /*
        printf("after insertion\n");
        tiered.drawTree();
        tiered.drawString();
        sa->PrintArray();
         */
        
        length++;
        toInsert++;
        AnsTV = tiered.RangeQuery(1, length, NumAnsTV);
        /*
        printf("[");
        for (int i = 0; i < NumAnsTV; ++i) {
            printf("%d ", AnsTV[i]);
        }
        printf("]\n");
         */
        AnsSA = sa->RangeQuery(1, length, &NumAnsSA);
        //sa->PrintArray();
        // AnsDANL = daNoLimit->RangeQuery(1, length, &NumAnsDANL);
        if (!CompareArray(AnsTV, AnsSA, NumAnsTV)) {
            cout<< "================================= insert not equal================================"<<endl;
            break;
        }

        pos = RandomInt(1, length);
        sa->Delete(pos);
        tiered.remove(pos);
        printf("delete %d\n", pos);

        length--;
        AnsTV = tiered.RangeQuery(1, length, NumAnsTV);
        //tiered.drawString();
        AnsSA = sa->RangeQuery(1, length, &NumAnsSA);
        //sa->PrintArray();
        // AnsDANL = daNoLimit->RangeQuery(1, length, &NumAnsDANL);
        if (!CompareArray(AnsTV, AnsSA, NumAnsTV)) {
            cout<< "================================= delete not equal================================"<<endl;
            break;
        }
    }

    for (k = 0; k < 1; ++k) {
        int pos = RandomInt(1, length);
        length++;
        sa->Insert(toInsert, pos);
        tiered.insert(pos, toInsert);
        printf("insert k=%d, pos=%d, toInsert = %d\n", k, pos, toInsert);

        tiered.drawTree();
        tiered.drawString();
        tiered.QueryOneByOne();
        sa->PrintArray();

        AnsTV = tiered.RangeQuery(1, length, NumAnsTV);
        AnsSA = sa->RangeQuery(1, length, &NumAnsSA);
        // AnsDANL = daNoLimit->RangeQuery(1, length, &NumAnsDANL);
        if (!CompareArray(AnsTV, AnsSA, NumAnsTV)) {
            cout<< "================================= insert not equal================================"<<endl;
            break;
        }


        pos = RandomInt(1, length);
        sa->Delete(pos);
        tiered.remove(pos);
        printf("delete %d\n", pos);
        tiered.drawTree();
        //tiered.QueryOneByOne();
        tiered.drawString();
        tiered.QueryOneByOne();
        sa->PrintArray();
        size_t s = tiered.Query(33);

        length--;
        AnsTV = tiered.RangeQuery(1, length, NumAnsTV);
        //tiered.drawString();
        AnsSA = sa->RangeQuery(1, length, &NumAnsSA);
        //sa->PrintArray();
        // AnsDANL = daNoLimit->RangeQuery(1, length, &NumAnsDANL);
        if (!CompareArray(AnsTV, AnsSA, NumAnsTV)) {
            cout<< "================================= delete not equal================================"<<endl;
            break;
        }
    }
    //fclose(f);
    return 0;
}
