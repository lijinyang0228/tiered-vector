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
    finstant<<" ,DA,DANL,ST,DAODP"<<endl;

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
    int ToInsert = iniNum + 1;
    int * array = new int[iniNum];
    for (int i = 0; i < iniNum; ++i) {
        array[i] = i+1;
    }
    DynamicArray *da = NewDynamicArray(array, iniNum, danodesize);
    DAnolimit * danolimit = NewDAnolimit(array, iniNum, danodesize);
    DAstatic * stree = NewStree(array, iniNum, danodesize);

    delete []array;
    int NumDA, NumDANL, NumST;
    //da->PrintTree();
    cout<<"da depth = "<<da->Depth() << endl;
    int DAdepth = 1, DANLdepth = 1, STdepth = stree->Depth();
    int numUpdate = 0;
    for (int lt = 0; lt < TotalActions; lt++) {
        //printf("lt = %d\n", lt);
        if (lt == 9) {
            da->PrintTree();
        }
        if (lt % 5000 == 0) {
            cout<<"lt = "<<lt;
            cout<<"da depth = "<<da->Depth() << endl;
            flog<<"lt = "<<lt<<endl;
            flog<<"da depth = "<<da->Depth() << endl;
        }
        if (da->Depth() > DAdepth) {
            DAdepth++;
            flog<<"numUpdate = "<<numUpdate<<" da depth = "<<DAdepth<<endl;
        }
        if (danolimit->Depth() > DANLdepth) {
            DANLdepth++;
            flog<<"numUpdate = "<<numUpdate<<" danolimit depth = "<<DANLdepth<<endl;
        }
        if (stree->Depth() > STdepth) {
            STdepth++;
            flog<<"numUpdate = "<<numUpdate<<" stree depth = "<<STdepth<<endl;
        }
        switch (a[lt]) {

            case 2: //insert
            {
                int pos = RandomInt(1, NowTotalNum);
                danolimit->Insert(ToInsert, pos);
                da->Insert(ToInsert, pos);
                stree->Insert(ToInsert, pos);

                ToInsert++;
                NowTotalNum++;
                break;
            }
            case 3: //delete
            {
                int pos = RandomInt(1, NowTotalNum);
                danolimit->Delete(pos);
                da->Delete(pos);
                stree->Delete(pos);

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
                int * oldArray = da->RangeQuery(start, end, &NumDA);
                int * newArray = new int[len];
                for (int j = 0; j < len; ++j) {
                    newArray[j] = oldArray[len-j-1];
                    //cout<<newArray[j]<<" ";
                }
                // cout<<endl;

                danolimit->Reorder(start, end, newArray);
                da->Reorder(start, end, newArray);
                stree->Reorder(start, end, newArray);

                delete []newArray;
                delete []oldArray;
                break;
            }
            case 5: //swap
            {
                int a[4] = {};
                for (int & j : a) {
                    j = RandomInt(1, NowTotalNum);
                }
                sort(a, a+4);
                if (a[1] == a[2]) {
                    continue;
                }
                int start1 = a[0];
                int end1 = a[1];
                int start2 = a[2];
                int end2 = a[3];
                danolimit->Swap(start1, end1, start2, end2);
                da->Swap(start1, end1, start2, end2);
                stree->Swap(start1, end1, start2, end2);

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
                danolimit->Move(start1, end1, start2);
                da->Move(start1, end1, start2);
                stree->Move(start1, end1, start2);

                break;
            }
        }

        if ( (lt+1 <= 10) || ((lt+1<=100) && ((lt+1)%10 == 0)) || ((lt+1<=1000) && ((lt+1)%100 == 0))
             || ((lt+1<=10000) && ((lt+1)%1000 == 0)) || ((lt+1<=100000) && ((lt+1)%10000 == 0))
             || ((lt+1<=1000000) && ((lt+1)%100000 == 0))) {


            int num;
            int * AnsDA = da->RangeQuery(1, NowTotalNum, &num);
            int * Ansdanl = danolimit->RangeQuery(1, NowTotalNum, &num);
            int * Ansst = stree->RangeQuery(1, NowTotalNum, &num);
            bool flag1 = false, flag2 = false, flag3 = false;
            if (!CompareArray(AnsDA, Ansdanl, num)) {
                flag1 =true;
            }
            if (!CompareArray(AnsDA, Ansst, num)) {
                flag2 =true;
            }
            if (!CompareArray(Ansst, Ansdanl, num)) {
                flag3 =true;
            }

            if (flag3||flag2||flag1) {
                printf(" ---------------------lt = %d, move not equal: %d %d %d\n", lt, flag1, flag2, flag3);
                //stree->PrintTree();
                break;
            }

            delete []Ansst, delete []Ansdanl, delete []AnsDA;

            numUpdate ++;

            long long Tda = 0, Tdanl = 0, Tst = 0, Tod = 0;
            CurOutputNum ++;
            double fl = (lt+1)*1.0/operations ;
            finstant << fl << ",";
            int pos = RandomInt(1, NowTotalNum);

            time1 = timeNow();
            danolimit->Insert(ToInsert, pos);
            time2 = timeNow();
            Tdanl = duration(time2-time1);

            time1 = timeNow();
            da->Insert(ToInsert, pos);
            time2 = timeNow();
            Tda = duration(time2-time1);

            time1 = timeNow();
            stree->Insert(ToInsert, pos);
            time2 = timeNow();
            Tst = duration(time2 - time1);

            ToInsert++;
            NowTotalNum++;
            finstant <<Tda << ","<< Tdanl<<","<<Tst <<endl;
            cout<<"lt = "<< lt <<" da depth = "<<da->Depth()<<endl;
        }
        numUpdate++;
    } //for lt <= loopTime
    //cout<<"da depth = "<<da->Depth() << endl;
    flog<<"da depth = "<<da->Depth() << endl;
    delete []reorderrange;
    delete []a;
    delete da;
    delete danolimit;
    delete stree;
    flog.close();
    finstant.close();
    return 0;
}

