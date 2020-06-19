#include "StandardArray.h"
#include <cstdio>
#include <iostream>
using namespace std;

StandardArray::StandardArray(int length, int numItems, int* array_){
    Length = length;
    NumItems = numItems;
    array = new int [Length];
    for (int i = 0; i < numItems; ++i) {
        array[i] = array_[i];
    }
}

StandardArray * NewStandardArray(int * array, int numItems){
    StandardArray * sa = new StandardArray(2*numItems, numItems, array);
    return sa;
}

void StandardArray::PrintArray() const {
    printf("[%d", array[0]);
    for (int i = 1; i < NumItems; ++i) {
        printf(" %d", array[i]);
    }
    printf("]\n");
}

int* StandardArray::RangeQuery(int start, int end, int * lenOfAns) const {
    int dastart = 1;
    int daend = NumItems;
    int realstart = start;
    int realend = end;
    if (realstart < dastart) {
        realstart = dastart;
    }
    if (realend > daend) {
        realend = daend;
    }
    int realnum = realend - realstart + 1;
    *lenOfAns = realnum;
    if (realnum <= 0) {
        return nullptr;
    }
    int * ans = new int[realnum];
    int k = 0;
    realstart--;
    realend--;
    for (int i = realstart; i <= realend; ++i) {
        ans[k++] = array[i];
    }
    return ans;
}

void StandardArray::Insert(int rowID, int pos){
    NumItems++;
    if (pos > NumItems) {
        pos = NumItems;
    }
    if (NumItems > Length) {
        int oldLen = Length;
        int newLen = Length * 2;
        int * newarray = new int[newLen + 1];
        int newNumItem = oldLen + 1;
        int j = 0;
        for (int i = 0; i < newNumItem; ++i) {
            if (i == pos-1) {
                continue;
            }
            newarray[i] = array[j];
            j++;
        }
        newarray[pos-1] = rowID;
        delete []array;
        array = newarray;
        NumItems = newNumItem;
        Length = newLen;
        return;
    }
    int itemToStore = rowID;
    for (int i = pos-1; i <= NumItems-1; ++i) {
        int tmp = array[i];
        array[i] = itemToStore;
        itemToStore = tmp;
    }
}

void StandardArray::Delete(int pos) {
    if (pos > NumItems) {
        pos = NumItems;
    }
    for (int i = pos; i < NumItems; i++) {
        array[i-1] = array[i];
    }
    NumItems--;
}

int StandardArray::Query(int pos) const {
    return array[pos-1];
}

void StandardArray::Reorder(int start, int end, const int * newID) const {
    for (int i = start; i <= end; ++i) {
        array[i-1] = newID[i-start];
    }
}

void StandardArray::Move(int start, int end, int des) const {
    if (des > NumItems) {
        des = NumItems;
    }else if (des >= start && des <= end+1) {
        return;
    }

    if (des < start) {
        int len = end-des+1;
        int * newarray = new int[len];
        int in = 0;
        for (int k = start-1; k <= end-1; k ++) {
            newarray[in] = array[k];
            in++;
        }
        for (int k = des-1; k < start-1; k ++) {
            newarray[in] = array[k];
            in++;
        }
        in = 0;
        for (int k = des-1; k <= end-1; k ++) {
            array[k] = newarray[in];
            in++;
        }
        delete []newarray;
        return;
    } else {
        int len = des-start;
        int * newarray = new int[len];
        int in = 0;
        for (int k = end; k < des-1; k ++) {
            newarray[in] = array[k];
            in++;
        }
        for (int k = start-1; k <= end-1; k ++) {
            newarray[in] = array[k];
            in++;
        }
        in=0;
        for (int k = start-1; k < des-1; k ++) {
            array[k] = newarray[in];
            in++;
        }
        delete []newarray;
        return;
    }
}

void StandardArray::Swap(int start1, int end1, int start2, int end2) const {
    int len = end2-start1+1;
    int * newarray = new int[len];// make([]int, len)
    int in = 0;
    for (int k = start2-1; k <= end2-1; k ++) {
        newarray[in] = array[k];
        in++;
    }

    for (int k = end1; k < start2-1; k ++) {
        newarray[in] = array[k];
        in++;
    }

    for (int k = start1-1; k <= end1-1; k ++) {
        newarray[in] = array[k];
        in++;
    }
    in = 0;
    for (int k = start1-1; k <= end2-1; k ++) {
        array[k] = newarray[in];
        in++;
    }
    delete []newarray;
}
