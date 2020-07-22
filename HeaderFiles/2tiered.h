/********************************************************************************
* MIT License
*
* Copyright (c) 2017 Mikko Berggren Ettienne
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
********************************************************************************/
/*
size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
size_t layerwidth = Layer::width * myPow(2, doubleTimes);
size_t childwidth = Layer::child::width * myPow(2, doubleTimes);
*/

/*
 * change getNextStart() function
 * try to get result without for loop
 *
 */

#ifndef _TEMPLATED_TIERED_H_
#define _TEMPLATED_TIERED_H_

#include <vector>
#include <cstring>
#include <ctime>
#include <list>

#include <iostream>
#include <cmath>
#include <assert.h>
#include <stack>
#include <queue>
#include <bitset>
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point TimeVar;
#define duration(a) std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()

#ifdef PPACK
#define INODE FakeNode<Elem>
#else
#define INODE FakeNode<void *>
#endif
#define LNODE FakeNode<T>

#define WRAP(a,b) (((a) + (b)) % (b))

using namespace std;
size_t myPow(int x, int p) {
    if (p == 0) return 1;
    if (p == 1) return x;

    int tmp = myPow(x, p/2);
    if (p%2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
}
/*
size_t getNextStart(size_t start1, size_t offset, size_t childCapacity, size_t layerCapacity, size_t childIdx) {
    size_t r = childCapacity * childIdx ;
    if ((start1 + offset) % childCapacity == 0) {
        return start1 + childCapacity;
    } else {
        if (r >= start1 + offset) {
            return r - offset;
        } else {
            TimeVar time1 = timeNow();
            for (int i = start1 + 1; i < start1 + childCapacity; ++i) {
                size_t x = (i + offset) % layerCapacity;
                if ( x / childCapacity == childIdx ) {
                    TimeVar time2 = timeNow();
                    printf("time in getNextStart loop is %lld\n", duration(time2-time1));
                    return i;
                }
            }
            return -1;
        }
    }
}
*/

size_t getNextStart(size_t start1, size_t offset, size_t childCapacity, size_t layerCapacity, size_t childIdx) {
    size_t r = childCapacity * childIdx ;
    if ((start1 + offset) % childCapacity == 0) {
        return start1 + childCapacity;
    } else {
        if (r >= start1 + offset) {
            return r - offset;
        } else {
            size_t r1 = (start1 + offset) % layerCapacity;
            size_t r2 = r1 % childCapacity;
            size_t x = childCapacity - r2;
            return start1 + x;
        }
    }
}


size_t doubleTimes = 0;
double threshold = 0.3;
static size_t totalHeight;
/*
size_t doubleCap(size_t height) {
    return myPow(myPow(2, height+1), doubleTimes);
}
 */

namespace Seq
{
    struct Info {
#ifdef ARRAY
        size_t* offsets;
#ifdef PACK
#else
        void** ptrs;
#endif
#ifdef PFREE
        void* elems;
#endif
#endif
    };

    struct Elem {
        size_t offset;
        size_t child;
    };

    template <size_t Num>
    struct Math {
        enum { log = Math<(Num + 1) / 2>::log + 1, logdown = Math<Num / 2>::logdown + 1 };
    };

    template <>
    struct Math<1> {
        enum { log = 0, logdown = 0 };
    };

    template <size_t Num>
    struct Pow2 {
        enum { value = Pow2<Num - 1>::value * 2 };
    };

    template <>
    struct Pow2<0> {
        enum { value = 1 };
    };

    struct LayerEnd { typedef LayerEnd child;
        enum { width = 0, capacity = 0, height = 0, nodes = 0, depth = 0, twopower = Pow2<height+1>::value  };
    };

    template <size_t Width, typename NextType = LayerEnd>
    struct Layer {
        enum { width = Width, capacity = NextType::capacity * Width, height = NextType::height + 1,
            nodes = NextType::nodes * Width + 1, twopower = Pow2<height+1>::value  };
        typedef NextType child;
    };

    template <size_t Width>
    struct Layer<Width, LayerEnd> {
        enum { width = Width, capacity = Width, height = 0, nodes = 1, twopower = Pow2<height+1>::value };
        typedef LayerEnd child;
    };

    template<typename Parents, typename Childs>
    struct LayerItr {
        typedef LayerItr<Layer<Childs::width, Parents>, typename Childs::child> child;
        typedef LayerItr<typename Parents::child, Layer<Parents::width, Childs> > parent;

        enum {
            width = Childs::width,
            capacity = Childs::capacity,
            height = Childs::height,
            twopower = Pow2<height+1>::value,
            nodes = Childs::nodes,
            depth = Parents::height,
            leaves = (unsigned long long)parent::width * parent::leaves,
            top_nodes = parent::top_nodes + leaves,
            bit_width = Math<capacity>::log,
            offsets_per = Pow2<Math<sizeof(size_t) * 8 / bit_width>::logdown>::value,
            top_width = parent::top_width + (leaves + offsets_per - 1) / offsets_per
        };
    };

    struct FakeParent {
        typedef FakeParent parent;
        enum { top_nodes = 0, width = 1, top_width = 0 };
    };

    template<typename Childs>
    struct LayerItr<LayerEnd, Childs> {
        typedef LayerItr<Layer<Childs::width, LayerEnd>, typename Childs::child> child;
        typedef FakeParent parent;

        enum { width = Childs::width, capacity = Childs::capacity, height = Childs::height,
            nodes = Childs::nodes, depth = 0, leaves = 1, top_nodes = 1, bit_width = Math<capacity>::log,
            offsets_per = 1, top_width = 1, twopower = Pow2<height+1>::value };
    };

    template <class T, size_t width>
    class Node {
    public:
        Node(size_t depth, size_t realWidth);
        size_t depth;
        size_t size = 0;
        size_t id;
        size_t nodewidth = width;
#ifdef PPACK
#else
        size_t offset = 0;
#endif
        T* elems;
    };

    template <class T>
    class FakeNode {
    public:
        size_t depth;
        size_t size = 0;
        size_t id;
        size_t nodewidth;
#ifdef PPACK
#else
        size_t offset = 0;
#endif
        T* elems;
    };

    template <class T, class Layer>
    class Tiered {

    public:
        Info info;
        size_t size = 0;
#ifdef ARRAY
        const static size_t root = 0;
#else

#ifdef PPACK

        #define root ((size_t) &relem)
                Elem relem;

#else
        size_t root;
#endif
#endif

        int print_helper(INODE * node, int n);

        T replace(T const elem, INODE * node, size_t index);
        T pop_push(T elem, INODE * node, size_t from, size_t count, bool goRight);
        size_t make_room(size_t node, size_t idx);

        void fill(T *&res, INODE * node, size_t from, size_t count);

        Tiered();
        void print();
        void fill(T *res);
        void remove(size_t idx);

        T sum(size_t from, size_t count);
        size_t successor(T elem);

        void insert(size_t idx, T elem);
        void insert_sorted(T elem);

        const T& Query(size_t idx) const;
        void randomize();
        T * RangeQuery(int start, int end, int & lenOfAns);
        void initialize(int * array, size_t length);
        void drawTree();
        void setTotalHeight();
        void expand(size_t addr, size_t idx, int *allEle, size_t allSize, size_t startIdx);

        void shrink(size_t addr, size_t idx, int *allEle, size_t allSize, size_t startIdx);

        void drawString();

        void Swap(int start1, int end1, int start2, int end2);

        void Move(int start, int end, int des);

        void Reorder(int start, int end, T *newID);

        void QueryOneByOne();
    };

};
#endif

#define TT template <class T, class Layer>


size_t ID = 0;


namespace Seq
{
    TT
    struct helper {

#ifdef ARRAY
        static size_t get_offset(size_t addr, Info info){
#ifdef LINE
            if(Layer::height % 2 == 0){
                return info.offsets[addr + Layer::parent::parent::top_nodes];
            }
            else{
                return info.offsets[addr + Layer::parent::top_nodes];
            }
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = Layer::parent::top_width + addr / Layer::offsets_per;
            return (info.offsets[pos] >> (Layer::bit_width * (addr % Layer::offsets_per))) & (((size_t)1 << Layer::bit_width) - 1);

#else
            addr += Layer::parent::top_nodes;
#endif
#endif
            return info.offsets[addr];
        }
        static void set_offset(size_t addr, size_t offset, Info info){
#ifdef LINE
            if(Layer::height % 2 == 0){
                addr =addr + Layer::parent::parent::top_nodes;
            }
            else{
                addr =addr + Layer::parent::top_nodes;
            }
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = Layer::parent::top_width + addr / Layer::offsets_per;
            size_t mask = (((size_t)1 << Layer::bit_width) - 1) << (Layer::bit_width * (addr % Layer::offsets_per));
            info.offsets[pos] = (info.offsets[pos] & ~mask) | (offset << (Layer::bit_width * (addr % Layer::offsets_per)));
            return;

#else
            addr += Layer::parent::top_nodes;
#endif
#endif
            info.offsets[addr] = offset;
        }
        static size_t get_child(size_t addr, size_t idx){
#ifdef LINE
            if(Layer::height % 2 == 0){
                return (addr - (addr/(Layer::parent::width + 1))) * (Layer::width + Layer::width * Layer::child::width) + idx*(Layer::child::width+1);
            }
            else{
                return addr + idx + 1;
            }
#elif defined(LEVEL)
            return Layer::width * addr + idx;
#endif
            return Layer::child::nodes * idx + 1 + addr;
        }
        static size_t make_room(size_t addr, size_t idx, Info info) {
            idx = (idx + get_offset(addr, info)) % Layer::capacity;
            auto childIdx = idx / Layer::child::capacity;

            return helper<T, typename Layer::child>::make_room(
                   get_child(addr, childIdx), idx, info);


        }
#else
        static size_t get_offset(size_t addr, Info info){
#ifdef PPACK
            return ((Elem*) addr)->offset;
#else
            return ((INODE*) addr)->offset;
#endif
        }
        static void set_offset(size_t addr, size_t offset, Info info){
#ifdef PPACK
            ((Elem*) addr)->offset = offset;
#else
            ((INODE*) addr)->offset = offset;
#endif
        }
        static size_t get_child(size_t addr, size_t idx){
#ifdef PPACK
            Elem* node = (Elem*) addr;
            auto child = &((INODE*)node->child)->elems[idx];
#else
            INODE* node = (INODE*) addr;
            auto child = node->elems[idx];
#endif
            return (size_t) child;
        }

        static size_t make_room(size_t addr, size_t idx, Info info) {
#ifdef PPACK
            auto elem = (Elem*) addr;
            auto node = (INODE*) elem->child;
            idx = (idx + elem->offset) % Layer::capacity;
            auto childIdx = idx / Layer::child::capacity;
            if (node->elems[childIdx].child == NULL) {
#else
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            size_t layerwidth = Layer::width * myPow(2, doubleTimes);
            size_t childwidth = Layer::child::width * myPow(2, doubleTimes);

            auto node = (INODE*) addr;
            idx = (idx + node->offset) % layerCapacity;
            auto childIdx = idx / childCapacity;
            if (node->elems[childIdx] == NULL) {
#endif
                if (Layer::height == 1) {
#ifdef PPACK

                    node->elems[childIdx] = {0, (size_t)new Node<T, Layer::child::width>(0)};
#else
                    node->elems[childIdx] = new Node<T, Layer::child::width>(0, childwidth);
#endif
                } else {
#ifdef PPACK
                    node->elems[childIdx] = {0, (size_t)new Node<Elem, Layer::child::width>(0)};
#else
                    node->elems[childIdx] = new Node<void*, Layer::child::width>(0, childwidth);
#endif
                }
                node->size++;
            }

#ifdef PPACK
            return helper<T, typename Layer::child>::make_room((size_t)&node->elems[childIdx], idx, info);
#else
            return helper<T, typename Layer::child>::make_room((size_t)node->elems[childIdx], idx, info);
#endif
        }
#endif
        static T& get(size_t addr, size_t idx, Info info) {
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            //size_t layerwidth = Layer::width * myPow(2, doubleTimes);
            //size_t childwidth = Layer::child::width * myPow(2, doubleTimes);
            idx = (idx + get_offset(addr, info)) % layerCapacity;
            auto child = get_child(addr, idx / childCapacity);
            return helper<T, typename Layer::child>::get(child, idx, info);
        }
        static size_t get_leaf(size_t addr, size_t idx, Info info) {
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            idx = (idx + get_offset(addr, info)) % layerCapacity;
            auto child = get_child(addr, idx/ childCapacity);
            return helper<T, typename Layer::child>::get_leaf(child, idx, info);
        }
        static void drawTree(size_t addr, list<size_t>&lst, int & rank, int & exp, int& totalHeight, int nodewidth) {
            INODE * node = (INODE*) addr;
            if (node == nullptr) {
                printf("EMPTY | ");
                rank++;
                if (rank == myPow(nodewidth, exp)) {
                    rank = 0;
                    exp++;
                    printf("\n");
                }
                return;
            }
            printf("[%zu, %zu, %zu]", node->offset, node->size, node->id);
            if (totalHeight == exp) { // leaf node
                LNODE * leaf = (LNODE*) addr;
                int i = 0;
                for (; i < nodewidth; ++i) {
                    printf(" %d", leaf->elems[i]);
                }
                printf(" | ");
                return;
            }
            for (int i = 0; i < nodewidth; ++i) {
                printf(" _");
            }
            printf(" | ");
            int j = 0;
            for (; j < node->nodewidth; ++j) {
                void* child = node->elems[j];// get_child(addr, j % myPow(Layer::child::capacity, doubleTimes));
                lst.push_back(size_t (child));
            }
            rank++;
            if (rank == myPow(nodewidth, exp)) {
                rank = 0;
                exp++;
                printf("\n");
            }
        }
        static void RangeQuery(size_t addr, size_t startIdx, size_t end, Info info, T * ans, size_t ansStart, size_t ansEnd, int expectedSize) {
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            size_t layerwidth = Layer::width * myPow(2, doubleTimes);
            size_t childwidth = Layer::child::width * myPow(2, doubleTimes);
            size_t offset = get_offset(addr, info);
            size_t start1 = startIdx, start2, len ; // start without offset
            size_t startOff; //start after plus offset
            size_t childIdx; // = start1 / childCapacity;
            //size_t innerIdx = startIdx;
            bool flag = false;
            INODE* cur = (INODE*) addr;
            //start1 = (startIdx + offset) % layerCapacity;
            //childIdx = start1 / childCapacity;
            //size_t childIdxBeforeRemainer = childIdx;
            for (int j = 0; j < cur->size + 1; ++j) {
                startOff = (start1 + offset) % layerCapacity;
                childIdx = startOff / childCapacity;
                TimeVar time1 = timeNow();
                start2 = getNextStart(start1, offset, childCapacity, layerCapacity, (childIdx + 1) % childwidth);
                TimeVar time2 = timeNow();
                if (start2 > end) {
                    start2 = end + 1;
                    flag = true;
                }
                len = start2 - start1;
                auto child = get_child(addr, childIdx);
                TimeVar time3 = timeNow();
                helper<T, typename Layer::child>::RangeQuery(child, startOff, startOff+len-1, info, ans, ansStart, ansStart+len-1, len);
                TimeVar time4 = timeNow();
                //printf("%d %d %d\n", duration(time2-time1), duration(time3-time2), duration(time4-time3));
                if (flag) {
                    break;
                }
                ansStart += len;
                //startIdx += len;
                start1 = start2; //% layerCapacity;
                //childIdxBeforeRemainer++;
            }
        }
        static void updateElems(size_t addr, size_t startIdx, size_t end, Info info, T * ans, size_t ansStart, size_t ansEnd, int expectedSize) {
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            size_t layerwidth = Layer::width * myPow(2, doubleTimes);
            size_t childwidth = Layer::child::width * myPow(2, doubleTimes);
            size_t offset = get_offset(addr, info);
            size_t start1 = startIdx, start2, len ; // start without offset
            size_t startOff; //start after plus offset
            size_t childIdx; // = start1 / childCapacity;
            bool flag = false;
            INODE* cur = (INODE*) addr;
            for (int j = 0; j < cur->size + 1; ++j) {
                startOff = (start1 + offset) % layerCapacity;
                childIdx = startOff / childCapacity;
                start2 = getNextStart(start1, offset, childCapacity, layerCapacity, (childIdx + 1) % childwidth);
                if (start2 > end) {
                    start2 = end + 1;
                    flag = true;
                }
                len = start2 - start1;
                auto child = get_child(addr, childIdx);
                helper<T, typename Layer::child>::updateElems(child, startOff, startOff+len-1, info, ans, ansStart, ansStart+len-1, len);
                if (flag) {
                    break;
                }
                ansStart += len;
                //startIdx += len;
                start1 = start2; //% layerCapacity;
                //childIdxBeforeRemainer++;
            }
        }
        static T* drawString(size_t addr, size_t idx, Info info, size_t ExpectedSize, size_t& RealSize) {
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            size_t layerwidth = Layer::width * myPow(2, doubleTimes);
            size_t childwidth = Layer::child::width * myPow(2, doubleTimes);
            idx = (idx + get_offset(addr, info)) % layerCapacity;
            T * ans = new int[ExpectedSize];
            INODE* cur = (INODE*) addr;
            RealSize = 0;
            size_t thisSize = 0;
            int ansIdx = 0;
            for (int j = 0; j < cur->size; ++j) {
                auto child = get_child(addr, idx / childCapacity);
                T * re = helper<T, typename Layer::child>::drawString(child, idx, info, childCapacity, thisSize);
                copy(re, re+int(thisSize), ans+ansIdx);
                ansIdx += thisSize;
                RealSize += thisSize;
                delete []re;
                idx = (idx+thisSize) % layerCapacity;
            }
            return ans;
        }
        static void deallocate(size_t addr) {
            INODE* node = (INODE*) addr;
            if (node == nullptr) {
                return;
            }
            for (int i = 0; i < node->nodewidth; ++i) {
                auto child = get_child(addr, i);
                helper<T, typename Layer::child>::deallocate(child);
            }
            delete []node->elems;
            delete node;
        }
        // make room for addr[idx], return addr[idx]
        static void expandFromNull(void*& addr, int * allEle, size_t allSize, size_t& startIdx, size_t& maxID) {
            size_t layerwidth = Layer::width * myPow(2, doubleTimes);
            size_t childwidth = Layer::child::width * myPow(2, doubleTimes);
            if (Layer::height == 1) { // leaf parent
                addr = new Node<T, Layer::width>(0, layerwidth); // leaf node
            } else {
                addr = new Node<void*, Layer::child::width>(0, childwidth); // non-leaf node
            }
            INODE* newNode = (INODE*) addr;
            newNode->id = maxID;
            maxID++;
            for (int i = 0; i < newNode->nodewidth; ++i) {
                //size_t childAddr = (size_t) newNode->elems[i];
                helper<T, typename Layer::child>::expandFromNull(newNode->elems[i], allEle, allSize, startIdx, maxID);
                newNode->size++;
                if (startIdx == allSize) {
                    break;
                }
            }
        }

        static void expand(size_t addr, size_t idx, int * allEle, size_t allSize, size_t& startIdx, size_t& maxID) {
            INODE* node = (INODE*) addr;
            node->id = maxID;
            maxID++;
            void ** newElems = new void* [node->nodewidth * 2];
            memset(newElems, 0, sizeof(void*)*node->nodewidth * 2);
            node->offset = 0;
            int i = 0;
            for (; i < node->nodewidth; ++i) {
                auto child = get_child(addr, i);
                newElems[i] = (void*) child;
                helper<T, typename Layer::child>::expand(child, idx, allEle, allSize, startIdx, maxID);
                //idx = (idx+1) % layerCap;
                if (startIdx == allSize) {
                    i++;
                    break;
                }
            }
            node->size = i;
            if (startIdx == allSize) {
                for (; i < node->nodewidth; ++i) {
                    auto child = get_child(addr, i);
                    helper<T, typename Layer::child>::deallocate(child);
                }
            } else {
                for ( ; i < node->nodewidth; ++i) {
                    newElems[i] = node->elems[idx];
                    helper<T, typename Layer::child>::expandFromNull(node->elems[idx], allEle, allSize, startIdx, maxID);
                    node->size++;
                    if (startIdx == allSize) {
                        break;
                    }
                }
                for ( ; i < node->nodewidth * 2; ++i) {
                    helper<T, typename Layer::child>::expandFromNull(newElems[i], allEle, allSize, startIdx, maxID);
                    node->size++;
                    if (startIdx == allSize) {
                        break;
                    }
                }
            }
            node->nodewidth *= 2;
            delete []node->elems;
            node->elems = newElems;
        }
        static void expandAndInitialize(size_t addr, size_t idx) {
            INODE* node = (INODE*) addr;
            node->nodewidth *= 2;
            delete []node->elems;
            node->elems = new typename Layer::child [node->nodewidth];
            node->offset = 0;
            //cout<<Layer::capacity<<","<<Layer::child::capacity<<endl;
            //cout<<Layer::height<<","<<Layer::child::height<<endl;
            int layerCap = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            int childCap = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            //idx = (idx + get_offset(addr, Info{})) % (layerCap);
            int i = 0;
            for (; i < node->nodewidth; ++i) {
                auto child = get_child(addr, i);
                helper<T, typename Layer::child>::expandAndInitialize(child, idx);
            }
        }

        static void setTotalHeight() { // start from 1
            totalHeight = Layer::height + 2;
        }

        static bool remove_room(size_t addr, size_t idx) {
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            size_t layerwidth = Layer::width * myPow(2, doubleTimes);
            size_t childwidth = Layer::child::width * myPow(2, doubleTimes);
#ifdef PPACK
            auto elem = (Elem*) addr;
            auto node = (INODE*) elem;
            idx = (idx + elem->offset) % Layer::capacity;
#else
            auto node = (INODE*) addr;
            idx = (idx + node->offset) % layerCapacity;
#endif
            auto childIdx = idx / childCapacity;

            if (helper<T, typename Layer::child>::remove_room((INODE *)node->elems[childIdx], idx)) {
                if (Layer::height == 1) {
                    delete (Node<T, Layer::child::width>*)node->elems[childIdx];
                } else {
                    delete (Node<void*, Layer::child::width>*)node->elems[childIdx];
                }
                node->elems[childIdx] = NULL;
                return --node->size == 0;
            }

            return false;
        }

        static T pop_push(T elem, size_t addr, size_t from, size_t count, bool goRight, Info info, bool& forRemove){
            //printf("%d %d %d %d\n", Layer::height, Layer::capacity, Layer::child::height, Layer::child::capacity);
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);

            size_t idx = (from + helper<T, Layer>::get_offset(addr, info)) % layerCapacity;

            while (count > 0) {
                size_t doCount = min(count, goRight ? (childCapacity - (idx % childCapacity))
                                                    : (idx % childCapacity + 1));

                auto child = get_child(addr, idx / childCapacity);


                if (doCount == childCapacity) {
                    helper<T, typename Layer::child>::set_offset(child, WRAP((helper<T, typename Layer::child>::get_offset(child, info)) + (goRight ? -1 : 1), childCapacity), info);
                    if (forRemove) {
                        elem = helper<T, typename Layer::child>::replaceForRemove(elem, child, idx, info);
                        forRemove = false;
                    } else {
                        elem = helper<T, typename Layer::child>::replace(elem, child, idx, info);
                    }
                } else {
                    elem = helper<T, typename Layer::child>::pop_push(elem, child, idx, doCount, goRight, info, forRemove);
                    forRemove = false;
                }

                idx = WRAP(idx + (goRight ? doCount : -doCount), layerCapacity);
                count -= doCount;
            }
            return elem;
        }

        inline static T sum(size_t addr, size_t from, size_t count, Info info) {
            T s = T();
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            size_t idx = (from + get_offset(addr, info)) % layerCapacity;

            while (count > 0) {
                size_t doCount = min(count, childCapacity - (idx % childCapacity));
                auto child = get_child(addr, idx / childCapacity);
                s += helper<T, typename Layer::child>::sum(child, idx, doCount, info);
                idx = (idx + doCount) % layerCapacity;
                count -= doCount;
            }

            return s;
        }
        static T replace(T elem, size_t addr, size_t idx, Info info) {
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
            return res;
        }

        static T replaceForInsert(T elem, size_t addr, size_t idx, Info info) {
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
            LNODE *leaf;
            leaf = (LNODE *) get_leaf(addr, idx, info);
            leaf->size++;
            return res;
        }

        static T replaceForRemove(T elem, size_t addr, size_t idx, Info info) {
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
            LNODE *leaf;
            leaf = (LNODE *) get_leaf(addr, idx, info);
            leaf->size--;
            return res;
        }

        static int print_helper(size_t addr, int n, Info info) {
            int x = n + 1;

#ifdef PPACK
            auto node = ((INODE*) ((Elem*) addr)->child);
#else
            auto node = (INODE*) addr;
#endif
            cout << n << " [label=\"" << node->id << " (" << node->size << "/" << get_offset(addr, info) << ")\"]" << endl;

            for (int i = 0; i < Layer::width; i++) {
                auto elem = node->elems[i];

#ifdef PPACK
                if(elem.child == 0){
#else
                if(elem == NULL){
#endif
                    cout << n << " -> "<< x << ';' << endl;
                    cout << x << " [label=\"NULL\"]" << endl;
                    x++;
                }
                else {
                    cout << n << " -> " << x << ';' << endl;

                    x = helper<T, typename Layer::child>::print_helper((size_t)&elem, x);
                }
            }

            return x;
        }

        static void randomize(size_t addr, size_t max, Info info) {
            size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
            size_t childCapacity = Layer::child::capacity * myPow(Layer::child::twopower, doubleTimes);
            for (int i = 0; i < Layer::width * myPow(2, doubleTimes); i++) {
                if (max > i * childCapacity) {
                    helper<T, typename Layer::child>::randomize(get_child(addr, i), max - i * childCapacity, info);
                }
            }

            if (max >= layerCapacity) {
                set_offset(addr, rand() % layerCapacity, info);
            }
        }
    };

    template <class T, typename A, size_t W>
    struct helper<T, LayerItr<A, Layer<W, LayerEnd> > > {
        typedef LayerItr<A, Layer<W, LayerEnd> >  L;


#ifdef PACK
        static size_t get_fake_offset(size_t addr, Info info){
#ifdef LINE
            addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
            addr += L::parent::top_nodes;
#endif
            return info.offsets[addr];
        }
#endif

#ifdef ARRAY
        static size_t get_offset(size_t addr, Info info){
#ifdef LINE
            addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = L::parent::top_width + addr / L::offsets_per;
            return (info.offsets[pos] >> (L::bit_width * (addr % L::offsets_per))) & (((size_t)1 << L::bit_width) - 1);
#else
            addr += L::parent::top_nodes;
#endif
#endif
#ifdef PACK
            size_t off = info.offsets[addr];
            return off >> 48;
#else

           return info.offsets[addr];
#endif
        }
        static void set_offset(size_t addr, size_t offset, Info info){

#ifdef LINE
            addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = L::parent::top_width + addr / L::offsets_per;
            size_t mask = (((size_t)1 << L::bit_width) - 1) << (L::bit_width * (addr % L::offsets_per));
            info.offsets[pos] = (info.offsets[pos] & ~mask) | (offset << (L::bit_width * (addr % L::offsets_per)));
            return;
#else
            addr += L::parent::top_nodes;
#endif
#endif

#ifdef PACK
            size_t off = info.offsets[addr];
            size_t ptr = ((off << 16) >> 16);
            size_t n_offset = (offset << 48) | ptr;
            info.offsets[addr] = n_offset;
#else
            info.offsets[addr] = offset;
#endif
        }
        static T& get_elem(size_t addr, size_t idx, Info info) {
#ifdef PFREE
            return ((T*)info.elems)[addr * L::width + idx];
#elif defined(PACK)
            size_t off = get_fake_offset(addr, info);
            size_t ptr = ((off << 16) >> 16);
            //cerr << ((T*)ptr)[idx] << " GET " <<endl;
            return ((T*)ptr)[idx];
#else

            return ((T*)info.ptrs[addr])[idx];
#endif
        }
        static size_t make_room(size_t addr, size_t idx, Info info) {
#ifdef PFREE
            return addr;
#elif defined(PACK)
            size_t offset = get_fake_offset(addr, info);
            if(offset << 16 == 0) {
                size_t arr_addr = (size_t) new T[L::width];
                assert((arr_addr >> 48) == 0);

                size_t n_offset = (offset << 48) | arr_addr;

#ifdef LINE
                addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
                addr += L::parent::top_nodes;
#endif
                info.offsets[addr] = n_offset;
            }
#else
            if (info.ptrs[addr] == NULL) {
                info.ptrs[addr] = new T[L::width];
            }
#endif
            return addr;
        }
#else
        static size_t get_offset(size_t addr, Info info){
#ifdef PPACK
            return ((Elem*) addr)->offset;
#else
            return ((INODE*) addr)->offset;
#endif
        }
        static void set_offset(size_t addr, size_t offset, Info info){
#ifdef PPACK
            ((Elem*) addr)->offset = offset;
#else
            ((INODE*) addr)->offset = offset;
#endif
        }
        static T& get_elem(size_t addr, size_t idx, Info info) {
#ifdef PPACK
            auto elem = (Elem*) addr;
            auto child = (LNODE*)elem->child;
            return child->elems[idx];
#else
            return ((LNODE *)addr)->elems[idx];
#endif
        }
        static size_t get_leaf(size_t addr, size_t idx, Info info) {
            return addr;
        }
        static size_t make_room(size_t addr, size_t idx, Info info) {
            return addr;
        }
        static void expandFromNull(void*& addr, int * allEle, size_t allSize, size_t& startIdx, size_t& maxID) {
            size_t childwidth = L::width * myPow(2, doubleTimes);
            addr = new Node<T, L::width>(0, childwidth);
            LNODE* node = (LNODE*) addr;
            node->id = maxID;
            maxID++;
            for (int i = 0; i < node->nodewidth; ++i) {
                node->elems[i] = allEle[startIdx];
                startIdx++;
                node->size++;
                if (startIdx == allSize) {
                    break;
                }
            }
        }
#endif

        static T pop_push(T elem, size_t addr, size_t from, size_t count, bool goRight, Info info, bool & forRemove) {
            size_t layerCapacity = L::capacity * myPow(L::twopower, doubleTimes);
            T res;

#ifdef ARRAY
            #ifdef PFREE
            auto elems = &((T*)info.elems)[addr*L::width];
#elif defined(PACK)
            size_t off = get_fake_offset(addr, info);
            size_t ptr = ((off << 16) >> 16);
            auto elems =  (T*)ptr;
#else
            auto elems = (T*)info.ptrs[addr];
#endif
#else
#ifdef PPACK
            auto elems = ((LNODE*) ((Elem*)addr)->child)->elems;
#else
            auto elems = ((LNODE*)addr)->elems;
#endif
#endif
            if (forRemove) {
                ((LNODE*)addr)->size--;
            }
            if (goRight) {
                res = elems[(from + get_offset(addr, info) + count - 1) % layerCapacity];

                size_t start = (from + get_offset(addr, info)) % layerCapacity;
                size_t beforeWrap = min(layerCapacity - start - 1, count - 1);

                // Move last part
                if (beforeWrap < count - 1) {
                    memmove(&elems[1], &elems[0], (count - beforeWrap - 2) * sizeof(T));
                    elems[0] = elems[layerCapacity - 1];
                }

                // Move first part
                memmove(&elems[start + 1], &elems[start], beforeWrap * sizeof(T));
            } else {
                res = elems[(from + get_offset(addr, info) - count + 1) % layerCapacity];

                size_t start = (from + get_offset(addr, info)) % layerCapacity;
                size_t beforeWrap = min(start, count - 1);

                if (beforeWrap < count - 1) {
                    size_t afterWrap = count - beforeWrap - 2;
                    memmove(&elems[layerCapacity - afterWrap - 1], &elems[layerCapacity - afterWrap], afterWrap * sizeof(T));
                    elems[layerCapacity - 1] = elems[0];
                }

                memmove(&elems[start - beforeWrap], &elems[start - beforeWrap + 1], beforeWrap * sizeof(T));
            }

            elems[(from + get_offset(addr, info)) % layerCapacity] = elem;

            return res;
        }
        static T& get(size_t addr, size_t idx, Info info) {
            size_t layerCapacity = L::capacity * myPow(L::twopower, doubleTimes);
            idx = (idx + helper<T, L>::get_offset(addr, info)) % layerCapacity;
            return get_elem(addr, idx, info);
        }
        static void updateElems(size_t addr, size_t startIdx, size_t end, Info info, T * ans,
                                size_t ansStart, size_t ansEnd, int expectedSize) {
            size_t layerCapacity = L::capacity * myPow(L::twopower, doubleTimes);
            startIdx = (startIdx + helper<T, L>::get_offset(addr, info)) % layerCapacity;
            LNODE * node = (LNODE*)addr;
            for (int j = 0; j < node->size; ++j) {
                node->elems[startIdx] = ans[ansStart];
                ansStart++;
                if (ansStart > ansEnd) {
                    break;
                }
                startIdx = (startIdx+1) % layerCapacity;
            }
        }

        static void RangeQuery(size_t addr, size_t startIdx, size_t end, Info info, T * ans,
                               size_t ansStart, size_t ansEnd, int expectedSize) {
            size_t layerCapacity = L::capacity * myPow(L::twopower, doubleTimes);
            startIdx = (startIdx + helper<T, L>::get_offset(addr, info)) % layerCapacity;
            LNODE * node = (LNODE*)addr;
            for (int j = 0; j < node->size; ++j) {
                size_t cur = node->elems[startIdx];
                ans[ansStart] = cur;
                ansStart++;
                if (ansStart > ansEnd) {
                    break;
                }
                startIdx = (startIdx+1) % layerCapacity;
            }
        }
        static T* drawString(size_t addr, size_t idx, Info info, size_t expectedSize, size_t& realSize) {
            size_t layerCapacity = L::capacity * myPow(L::twopower, doubleTimes);
            idx = (idx + helper<T, L>::get_offset(addr, info)) % layerCapacity;
            LNODE * node = (LNODE*)addr;
            T * ans = new T[node->size];
            for (int j = 0; j < node->size; ++j) {
                size_t cur = node->elems[idx];
                ans[j] = cur;
                idx = (idx+1) % layerCapacity;
            }
            realSize = node->size;
            return ans;
        }

        static void expand(size_t addr, size_t idx, int * allEle, size_t allSize, size_t& startIdx, size_t& maxID) {
            if (startIdx == allSize) {
                LNODE *node = (LNODE *) addr;
                delete[]node->elems;
                delete node;
                return;
            }
            LNODE *node = (LNODE *) addr;
            node->id = maxID;
            maxID++;
            node->nodewidth *= 2;
            delete[]node->elems;
            node->elems = new T[node->nodewidth];
            memset(node->elems, 0, sizeof(T) * node->nodewidth);
            node->offset = 0;
            int curIdx = 0;
            int layerCap = L::capacity * myPow(L::twopower, doubleTimes);
            for (; startIdx < allSize && curIdx < layerCap; ++startIdx, ++curIdx) {
                node->elems[curIdx] = allEle[startIdx];
            }
            node->size = curIdx;
        }

        static void deallocate(size_t addr) {
            LNODE* node = (LNODE*) addr;
            if (node == nullptr) {
                return;
            }
            delete []node->elems;
            delete node;
        }

        static void expandAndInitialize(size_t addr, size_t idx) {
            INODE* node = (INODE*) addr;
            node->nodewidth *= 2;
            delete []node->elems;
            node->elems = new void* [node->nodewidth];
            node->offset = 0;
            //cout<<Layer::capacity<<","<<Layer::child::capacity<<endl;
            //cout<<Layer::height<<","<<Layer::child::height<<endl;
            //int layerCap = L::capacity * myPow(L::twopower, doubleTimes);
            //int childCap = L::child::capacity * myPow(L::child::twopower, doubleTimes);
        }

        inline static T sum(size_t addr, size_t from, size_t count, Info info) {
            T s = T();

#ifdef ARRAY
            #ifdef PFREE
            auto elems = &((T*)info.elems)[addr*L::width];
#elif defined(PACK)
            size_t off = get_fake_offset(addr, info);
            size_t ptr = ((off << 16) >> 16);
            auto elems =  (T*)ptr;
#else
            auto elems = (T*)info.ptrs[addr];
#endif
#else
#ifdef PPACK
            auto elems = ((LNODE*) ((Elem*)addr)->child)->elems;
#else
            auto elems = ((LNODE*)addr)->elems;
#endif
#endif


            from = (from + get_offset(addr, info)) % L::capacity;

            if (from + count < L::capacity) {
                for (size_t i = 0; i < count; i++)
                    s += elems[from + i];
            } else {
                size_t firstCount = L::capacity - from;
                size_t secondCount = count - firstCount;

                for (size_t i = 0; i < firstCount; i++)
                    s += elems[from + i];

                for (size_t i = 0; i < secondCount; i++)
                    s += elems[i];
            }

            return s;
        }


        static bool remove_room(INODE *node, size_t idx) {
            return --node->size == 0;
        }

        static T replace(T elem, size_t addr, size_t idx, Info info) {
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
            return res;
        }

        static T replaceForInsert(T elem, size_t addr, size_t idx, Info info) {
            LNODE *leaf;
            leaf = (LNODE *) get_leaf(addr, idx, info);
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
            leaf->size++;
            return res;
        }

        static T replaceForRemove(T elem, size_t addr, size_t idx, Info info) {
            LNODE *leaf;
            leaf = (LNODE *) get_leaf(addr, idx, info);
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
            leaf->size--;
            return res;
        }

        static int print_helper(size_t addr, int n, Info info) {
            int x = n + 1;

            LNODE * leaf = (LNODE *) addr;
            cout << n << " [label=\"" << leaf->id << " (" << leaf->size << "/" << get_offset(addr, info) << ")\"]" << endl;

            for (int i = 0; i < L::width; i++) {
                auto elem = leaf->elems[i];
                cout << n << " -> " << x << endl;
                cout << x << " [label=\"" << elem << "\"];" << endl;
                x++;
            }

            return x;
        }

        static void randomize(size_t addr, size_t max, Info info) {
            if (max >= L::width) {
                set_offset(addr, rand() % L::width, info);
            }
        }
    };

#ifdef ARRAY
    TT
        Tiered<T, Layer>::Tiered() {
#ifdef PACK

#else
            info.ptrs = new void*[Layer::nodes];
            memset(info.ptrs, 0, sizeof(void*)*Layer::nodes);
#endif

#ifdef PFREE
            info.elems = new T[Layer::capacity];
#endif
            info.offsets = new size_t[Layer::nodes];
            memset(info.offsets, 0, sizeof(size_t)*Layer::nodes);
        }
#else
    TT
    Tiered<T, Layer>::Tiered() {

#ifdef PPACK
        relem = {0, (size_t) new Node<Elem, Layer::width>(0)};
#else
        root = (size_t ) new Node<void*, Layer::width>(0, Layer::width * myPow(2, doubleTimes));
#endif
    }


#endif


    template<class T, size_t width>
    Node<T, width>::Node(size_t depth, size_t realWidth) : depth(depth) {
        elems = new T[realWidth];
        memset(elems, 0, sizeof(T)*realWidth);
        id = ID;
        ID++;
        nodewidth = realWidth;
    }

    TT
    T Tiered<T, Layer>::sum(size_t from, size_t count){
        return helper<T, Layer>::sum((size_t)root, from-1, count, info);
    }

    TT
    void Tiered<T, Layer>::insert(size_t idxFromOne, T elem){
        bool forRemove = false;
        size_t idx = idxFromOne-1;
        size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
        if (size == layerCapacity) {
            size_t realSize = 0;
            //printf("before expansion\n");
            int * allEle = helper<T, Layer>::drawString(root, 0, Info{}, size, realSize);
            //drawTree();
            //INODE* node = (INODE*) root;
            expand((size_t)root, 0, allEle, realSize,0);
            /*
            printf("after expansion\n");
            drawTree();
            drawString();
             */
            layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);;
        }

        assert (idx <= size);
        if (idx >= size/2) {
            elem = helper<T, Layer>::pop_push(elem, (size_t)root, idx, size - idx, true, info, forRemove);
            helper<T, Layer>::make_room(root, size, info);
            helper<T, Layer>::replaceForInsert(elem, (size_t)root, size, info);
        } else {
            elem = helper<T, Layer>::pop_push(elem, (size_t)root, WRAP(idx - 1, layerCapacity), idx, false, info, forRemove);
            helper<T, Layer>::set_offset(root, WRAP((helper<T, Layer>::get_offset(root, info) - 1), layerCapacity), info);
            helper<T, Layer>::make_room(root, 0, info);
            helper<T, Layer>::replaceForInsert(elem, (size_t)root, 0, info);
        }

        size++;
    }

    TT
    void Tiered<T, Layer>::insert_sorted(T elem){
        size_t left = 0, right = size;

        while (left < right) {
            size_t mid = (left + right) / 2;
            if (elem < (*this)[mid]) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }

        insert(left, elem);
    }

    TT
    const T& Tiered<T, Layer>::Query(size_t idx) const{
        assert (idx-1 < size);

        return helper<T, Layer>::get((size_t)root, idx-1, info);
    }

    TT
    size_t Tiered<T, Layer>::successor(T elem){
        size_t left = 0, right = size;

        while (left < right) {
            size_t mid = (left + right) / 2;
            if (elem < (*this)[mid]) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }

        return left;
    }

    TT
    void Tiered<T, Layer>::fill(T *res){
        fill(res, root, 0, size);
    }


    TT
    void Tiered<T, Layer>::remove(size_t idx) {
        idx--;
        size_t layerCapacity = Layer::capacity * myPow(Layer::twopower, doubleTimes);
        bool forRemove = true;
        if (idx >= size/2) {
            size--;
            T garbage = {};
            // bug in original code: size - idx should be size + 1 - idx
            helper<T, Layer>::pop_push(garbage, root, size, size + 1 - idx, false, info, forRemove);
        } else {
            T garbage = {};
            helper<T, Layer>::pop_push(garbage, root, 0, idx + 1, true, info, forRemove);
            size--;

            helper<T, Layer>::set_offset(root, WRAP((helper<T, Layer>::get_offset(root, info)) + 1, layerCapacity), info);
        }
        /*
        if (size < layerCapacity * threshold) {
            size_t realSize = 0;
            int * allEle = helper<T, Layer>::drawString(root, 0, Info{}, size, realSize);
            //drawTree();
            //INODE* node = (INODE*) root;
            shrink((size_t)root, 0, allEle, realSize,0);
        }
         */
    }

    TT
    void Tiered<T, Layer>::print(){
        cout << "digraph G {" << endl;
        helper<T, Layer>::print_helper(root, 0);
        cout << "}" << endl;
    }

    TT
    void Tiered<T, Layer>::randomize() {
        helper<T, Layer>::randomize(root, size, info);
    }
    TT
    void Tiered<T, Layer>::expand(size_t addr, size_t idx, int * allEle, size_t allSize, size_t startIdx) {
        //cout<<"tiered.expand():"<<endl;
        doubleTimes ++;
        size_t maxID = 0;
        helper<T, Layer>::expand(addr, idx, allEle, allSize, startIdx, maxID);
        cout<<endl;
        ID = maxID;
    }
    TT
    void Tiered<T, Layer>::drawString(){
        printf("tiered.drawString(): \n[");
        int realSize = 0;
        T * s = RangeQuery(1, size, realSize);
        for (int i = 0; i < realSize; ++i) {
            cout<<s[i]<<" ";
        }
        printf("]\n");
    }
    TT
    T * Tiered<T, Layer>::RangeQuery(int start, int end, int & lenOfAns){
        TimeVar time1 = timeNow();
        int tvstart = 1;
        int tvend = size;
        int realstart = start;
        int realend = end;
        if (realstart < tvstart) {
            realstart = tvstart;
        }
        if (realend > tvend) {
            realend = tvend;
        }
        int realnum = realend - realstart + 1;
        lenOfAns = realnum;
        if (realnum <= 0) {
            return nullptr;
        }
        T * ans = new T[realnum];
        size_t ansIdx = 0;
        // static void RangeQuery(size_t addr, size_t start, size_t end, Info info, T * ans,
        // size_t ansStart, size_t ansEnd, int expectedSize)
        TimeVar time2 = timeNow();
        helper<T, Layer>::RangeQuery((size_t)root, realstart-1, realend-1, info, ans, 0, realnum-1, realnum);
        TimeVar time3 = timeNow();
        //printf("top function %lld, %lld\n", duration(time2-time1), duration(time3-time2));
        return ans;
    }
    TT
    void Tiered<T, Layer>::drawTree(){
        printf("tiered.drawTree() [offset, size, id] \n");
        INODE* r = (INODE*) root;
        int nodewidth = 0;
        if (r != nullptr) {
            nodewidth = r->nodewidth;
        }
        list<size_t> lst;
        lst.push_back((size_t)root);
        int rank = 0, exp = 0;
        int totalHeight = Layer::height;

        while (!lst.empty()) {
            size_t cur = lst.front();
            lst.pop_front();
            helper<T, Layer>::drawTree(cur, lst, rank, exp, totalHeight, nodewidth);

        }
        cout<<endl;
    }
    TT
    void Tiered<T, Layer>::setTotalHeight(){
        helper<T, Layer>::setTotalHeight();
    }
    TT
    void Tiered<T, Layer>::QueryOneByOne() {
        printf("Query one by one:\n");
        for (int j = 1; j <= size; ++j) {
            printf("%d ", Query(j));
        }
        printf("\n");
    }
    TT
    void Tiered<T, Layer>::initialize(int * array, size_t length) {
        for (int i = 0; i < length; ++i) {
            insert(i+1, array[i]);
        }
    }
    TT
    void Tiered<T, Layer>::Swap(int start1, int end1, int start2, int end2) {
        int len = end2-start1+1;
        int in = 0;
        T * oldArray = RangeQuery(start1, end2, len);
        T * newArray = new T[len];
        for (int k = start2; k <= end2; k ++) {
            newArray[in] = oldArray[k-start1];
            in++;
        }

        for (int k = end1+1; k <= start2-1; k ++) {
            newArray[in] = oldArray[k-start1];
            in++;
        }

        for (int k = start1; k <= end1; k ++) {
            newArray[in] = oldArray[k-start1];
            in++;
        }
        size_t updateIdx = 0;
        //helper<T, Layer>::RangeQuery(root, realstart-1, realend-1, info, ans, 0, realnum-1, realnum);
        helper<T, Layer>::updateElems(root, start1-1, end2-1, info, newArray, 0, len-1, len);
        delete []oldArray;
        delete []newArray;
    }
    TT
    void Tiered<T, Layer>::Move(int start, int end, int des) {
        if (des > size) {
            des = size;
        } else if (des >= start && des <= end+1) {
            return;
        }
        if (des < start) {
            int len = end-des+1;
            T * oldArray = RangeQuery(des, end, len);
            T * newArray = new T [len];

            int in = 0;
            for (int k = start; k <= end; k ++) {
                newArray[in] = oldArray[k-des];
                in++;
            }
            for (int k = des; k < start; k ++) {
                newArray[in] = oldArray[k-des];
                in++;
            }
            size_t updateIdx = 0;
            helper<T, Layer>::updateElems(root, des-1, end-1, info, newArray, 0, len-1, len);
            delete []oldArray;
            delete []newArray;
            return;
        } else {
            int len = des-start;
            T * oldArray = RangeQuery(start, des-1, len);
            T * newArray = new T [len];

            int in = 0;
            for (int k = end+1; k < des; k ++) {
                newArray[in] = oldArray[k-start];
                in++;
            }
            for (int k = start; k <= end; k ++) {
                newArray[in] = oldArray[k-start];
                in++;
            }
            size_t updateIdx = 0;
            helper<T, Layer>::updateElems(root, start-1, des-2, info, newArray, 0, len-1, len);
            delete []oldArray;
            delete []newArray;
            return;
        }
    }
    TT
    void Tiered<T, Layer>::Reorder(int start, int end, T* newID) {
        size_t len = end-start+1;
        size_t updateIdx= 0;
        helper<T, Layer>::updateElems(root, start-1, end-1, info, newID, 0, len-1, len);
    }
    TT
    void Tiered<T, Layer>::shrink(size_t addr, size_t idx, int * allEle, size_t allSize, size_t startIdx) {
        cout<<"tiered.shrink():"<<endl;
        doubleTimes --;
        size_t maxID = 0;
        helper<T, Layer>::shrink(addr, idx, allEle, allSize, startIdx, maxID);
        cout<<endl;
        ID = maxID;
    }
}
