#ifndef INDEXSTRUCTURE_STANDARDARRAY_H
#define INDEXSTRUCTURE_STANDARDARRAY_H

#include <cstdio>
#include <iostream>
using namespace std;


class StandardArray
{
public:
    int Length;
    int NumItems;
    int* array;
    StandardArray(int length, int numItems, int* array_);
    ~StandardArray(){delete []array;}
    int * RangeQuery(int start, int end, int *ans) const;

    void PrintArray() const;

    void Insert(int rowID, int pos);

    void Delete(int pos);

    int Query(int pos) const;

    void Reorder(int start, int end, const int *newID) const;

    void Swap(int start1, int end1, int start2, int end2) const;

    void Move(int start, int end, int des) const;
};

StandardArray * NewStandardArray(int * array, int numItems);

#endif //INDEXSTRUCTURE_STANDARDARRAY_H
