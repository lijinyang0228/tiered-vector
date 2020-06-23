/*
 * # items = 100,000
 * # operations = 100,000
 */
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#include "../HeaderFiles/templated_tiered.h"
#include "../HeaderFiles/StandardArray.h"
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
// A function to return a seeded random number generator.
inline std::mt19937& generator() {
    // the generator will only be seeded once (per thread) since it's static
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

// A function to generate integers in the range [min, max]
int RandomInt(int min, int max) {
    //  std::uniform_int_distribution<int> dist(min, max);
    //  return dist(generator());
    return (rand() % (max-min+1))+ min;
}


int* RangeDistributionRandom( int num, int min, int max) {
    int * n = new int[num];
    for (int j = 0; j < num; ++j) {
        n[j] = RandomInt(min, max);
    }
    shuffle(n, n + num, generator());
    return n;
}

int main() {
    //var time1, time2 time.Time
    string filepath[3] = {"Basicinsert.csv", "BasicinsertLog.txt"};
    ofstream finstant, flog, ffinal;
    finstant.open(filepath[0], ios::out | ios::in | ios::trunc);
    flog.open(filepath[1], ios::out | ios::in | ios::trunc);
    //ffinal.open(filepath[2], ios::out | ios::in | ios::trunc);
    finstant<<" ,SA,TV"<<endl;

    int iniNum = 10000; //0.03 = 30000
    int danodesize = 50;
    int m = 500;  //for linked list
    int NumOfOutput = 10;
    int operations = 10000;
    int InsertActions = operations * 2 / 10;
    int DeleteActions = operations * 2 / 10;
    int ReorderActions = operations * 2 / 10;
    int SwapActions = operations * 2 / 10;
    int MoveActions = operations * 2 / 10;
    int TotalActions = DeleteActions + InsertActions + ReorderActions+SwapActions + MoveActions; // 550 000
    int ComputeInterval = TotalActions / NumOfOutput; // 10%
    int Interval100 = TotalActions / 100;
    printf("%d, %d, %d, %d, %d\n", InsertActions, DeleteActions, ReorderActions, SwapActions, MoveActions);
    printf("# of operations = %d\n", operations);
    int CurOutputNum = 0;

    int *a = new int[TotalActions];
    int ua = 0;
    for (int y = 0; y < ReorderActions; y++) {
        a[ua] = 4;
        ua++;
    }
    for (int y = 0; y < SwapActions; y++) {
        a[ua] = 5;
        ua++;
    }
    for (int y = 0; y < MoveActions; y++) {
        a[ua] = 6;
        ua++;
    }
    for (int y = 0; y < InsertActions; y++) {
        a[ua] = 2;
        ua++;
    }
    for (int y = 0; y < DeleteActions; y++) {
        a[ua] = 3;
        ua++;
    }

    //shuffle(a, a + TotalActions, generator());
    //random_shuffle(&a[DeleteActions+InsertActions], &a[TotalActions]);
    //random_shuffle(&a[0], &a[MoveActions+SwapActions+ReorderActions]);
    random_shuffle(&a[0], &a[operations]);
    //int * shortqueryrange = RangeDistributionRandom(, 1, 100);
    int * reorderrange = RangeDistributionRandom(ReorderActions, 1, 100);
    int ir = 0;

    TimeVar time1, time2;

    int NowTotalNum = iniNum;
    int toInsert = iniNum + 1;
    int * array = new int[iniNum];
    for (int i = 0; i < iniNum; ++i) {
        array[i] = i+1;
    }
    Seq::Tiered<int, LayerItr<LayerEnd, Layer<10, Layer<10, Layer<10>>>>> tiered;
    tiered.initialize(array, iniNum);
    StandardArray *sa = NewStandardArray(array, iniNum);      
            
    delete []array;
    int NumDA, NumDANL, NumST;
    
    int numUpdate = 0;
    for (int lt = 0; lt < TotalActions; lt++) {
        //printf("a[%d] = %d\n", lt, a[lt]);
        if (lt % 5000 == 0) {
            cout<<"lt = "<<lt;
        }
        switch (a[lt]) {

            case 2: //insert
            {
                int pos = RandomInt(1, NowTotalNum);
                sa->Insert(toInsert, pos);
                tiered.insert(pos, toInsert);

                toInsert++;
                NowTotalNum++;
                break;
            }
            case 3: //delete
            {
                int pos = RandomInt(1, NowTotalNum);
                sa->Delete(pos);
                tiered.remove(pos);

                NowTotalNum--;
                break;
            }

            case 4: //reorder
            {
                int len = reorderrange[ir];
                ir++;
                if (len >= NowTotalNum) {
                    len = NowTotalNum-1;
                }
                int start = RandomInt(1, NowTotalNum - len);
                int end = start + len - 1;
                if (end >= NowTotalNum) {
                    end = NowTotalNum - 1;
                }
                //cout<<"reorder "<<start<<"  "<<end<<endl;
                //cout<<"new order: ";
                int * oldArray = sa->RangeQuery(start, end, &NumDA);
                int * newArray = new int[len];
                for (int j = 0; j < len; ++j) {
                    newArray[j] = oldArray[len-j-1];
                }

                sa->Reorder(start, end, newArray);
                tiered.Reorder(start, end, newArray);

                delete []newArray;
                delete []oldArray;
                break;
            }
            case 5: //swap
            {
                int x[4] = {};
                for (int & j : x) {
                    j = RandomInt(1, NowTotalNum);
                }
                sort(x, x+4);
                if (x[1] == x[2]) {
                    continue;
                }
                int start1 = x[0];
                int end1 = x[1];
                int start2 = x[2];
                int end2 = x[3];
                sa->Swap(start1, end1, start2, end2);
                tiered.Swap(start1, end1, start2, end2);

                break;
            }
            case 6: //move
            {
                int a[3] = {};
                for (int & j : a) {
                    j = RandomInt(1, NowTotalNum);
                }
                sort(a, a+3);
                if (a[1] == a[2]) {
                    continue;
                }
                int start1, end1, start2;

                if (RandomInt(1, 10) % 2 == 1) {
                    start1 = a[0];
                    end1 = a[1];
                    start2 = a[2];
                } else {
                    start1 = a[1];
                    end1 = a[2];
                    start2 = a[0];
                }
                sa->Move(start1, end1, start2);
                tiered.Move(start1, end1, start2);

                break;
            }
        }

        if ( (lt+1 <= 10) || ((lt+1<=100) && ((lt+1)%10 == 0)) || ((lt+1<=1000) && ((lt+1)%100 == 0))
             || ((lt+1<=10000) && ((lt+1)%1000 == 0)) || ((lt+1<=100000) && ((lt+1)%10000 == 0))
             || ((lt+1<=1000000) && ((lt+1)%100000 == 0))) {


            int num;
            int * AnsSA = sa->RangeQuery(1, NowTotalNum, &num);
            int * AnsTV = tiered.RangeQuery(1, NowTotalNum, num);
            bool flag1 = false;
            if (!CompareArray(AnsSA, AnsTV, num)) {
                flag1 =true;
            }

            if (flag1) {
                printf(" ---------------------lt = %d, move not equal: %d\n", lt, flag1);
                //stree->PrintTree();
                break;
            }

            delete []AnsTV, delete []AnsSA;

            numUpdate ++;

            long long Tsa = 0, Ttv = 0;
            CurOutputNum ++;
            double fl = (lt+1)*1.0/operations ;
            finstant << fl << ",";
            int pos = RandomInt(1, NowTotalNum);

            time1 = timeNow();
            sa->Insert(toInsert, pos);
            time2 = timeNow();
            Tsa = duration(time2-time1);

            time1 = timeNow();
            tiered.insert(pos, toInsert);
            time2 = timeNow();
            Ttv = duration(time2 - time1);

            toInsert++;
            NowTotalNum++;
            finstant <<Tsa << ","<< Ttv <<endl;
        }
        numUpdate++;
    } //for lt <= loopTime
    //cout<<"da depth = "<<da->Depth() << endl;
    delete []reorderrange;
    delete []a;
    delete sa;
    flog.close();
    finstant.close();
    return 0;
}

