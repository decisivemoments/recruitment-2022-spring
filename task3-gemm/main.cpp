#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <vector>
#include <cassert>
#include <string.h>
#include <smmintrin.h>
#include <emmintrin.h>

#define PRINT_TIME(code) do { \
    auto start = system_clock::now(); \
    code \
    auto end   = system_clock::now(); \
    auto duration = duration_cast<microseconds>(end - start); \
    cout << "time spent: " << double(duration.count()) << "us" << endl; \
} while(0)

using namespace std;

using namespace chrono;

using vec = vector<int>; 

const int scale[] = {256, 512, 1024, 2048};
const string data_path("./data/");

int a[2048*2048],b[2048*2048],c[2048*2048];


#define blocksize 64
#define open 4
void f1(const int size,int ta,int tb,int tc)
{
    int tae=ta+size*blocksize;
    for(int i = ta; i < tae; i+=size*4)
    {
        int tbe=tb+blocksize;
        for(int j = tb; j < tbe; j+=4)
        {
            int tce=tc+blocksize;
            __m128i c0=_mm_stream_load_si128((__m128i*)(c+i+j));
            __m128i c1=_mm_stream_load_si128((__m128i*)(c+i+size+j));
            __m128i c2=_mm_stream_load_si128((__m128i*)(c+i+size*2+j));
            __m128i c3=_mm_stream_load_si128((__m128i*)(c+i+size*3+j));
            for(int k = tc,sk = tc*size; k < tce; k++,sk+=size)
            {
                __m128i a0=_mm_set1_epi32(*(a+i+k));
                __m128i b0=_mm_stream_load_si128((__m128i*)(b+j+sk));
                c0=_mm_add_epi32(c0,
                    _mm_mullo_epi32(a0,b0)
                );
                __m128i a1=_mm_set1_epi32(*(a+i+size+k));
                //__m128i b1=_mm_stream_load_si128((__m128i*)(b+j+sk));
                c1=_mm_add_epi32(c1,
                    _mm_mullo_epi32(a1,b0)
                );
                __m128i a2=_mm_set1_epi32(*(a+i+size*2+k));
                //__m128i b2=_mm_stream_load_si128((__m128i*)(b+j+sk));
                c2=_mm_add_epi32(c2,
                    _mm_mullo_epi32(a2,b0)
                );
                __m128i a3=_mm_set1_epi32(*(a+i+size*3+k));
                //__m128i b0=_mm_stream_load_si128((__m128i*)(b+j+sk));
                c3=_mm_add_epi32(c3,
                    _mm_mullo_epi32(a3,b0)
                );
            }
            _mm_store_si128((__m128i*)(c+i+j),c0);
            _mm_store_si128((__m128i*)(c+i+size+j),c1);
            _mm_store_si128((__m128i*)(c+i+size*2+j),c2);
            _mm_store_si128((__m128i*)(c+i+size*3+j),c3);
        }    
    }
}

void Gemm(const int &size) {
    //#pragma omp parallel for
    // for(int i = 0; i < size; i++)
    // {
    //     for(int j = 0; j < size; j++)
    //     {
    //         for(int k = 0; k < size; k++)
    //         {
    //             c[i*size+j] += a[i*size+k] * b[k*size+j];
    //         }
    //     }    
    // }
    int size2=size*size;
    #pragma omp parallel for 
    for(int i = 0; i <size2; i+=size*blocksize)
    {
        for(int j = 0; j < size; j+=blocksize)
        {
            for(int k = 0; k < size; k+=blocksize)
            {
                f1(size,i,j,k);
            }
        }    
    }
        
}

void CheckResult(const int *c, const string &result_path,const int &size) {
    ifstream file_result(result_path);
    int nelems = size;
    float res_i;
    for(int i = 0; i < nelems; i++) {
        file_result >> res_i;
        assert(c[i] == res_i);
    }
    file_result.close();
}

// c = a * b
void Benchmark(const int &size) {
    const int nelems = size * size;
    const string a_path(data_path+to_string(size)+"/a");
    const string b_path(data_path+to_string(size)+"/b");
    const string result_path(data_path+to_string(size)+"/result");
    ifstream file_a(a_path);
    ifstream file_b(b_path);

    
    memset(a,0,sizeof(a));
    memset(b,0,sizeof(b));
    memset(c,0,sizeof(c));

    for(int i = 0; i < nelems; i++) {
        file_a >> a[i];
    }
    for(int i = 0; i < nelems; i++) {
        file_b >> b[i];
    }

    for(int i=0;i<5;i++)
    {
        memset(c,0,sizeof(c));
        PRINT_TIME(
        Gemm(size);
        );
    }

    CheckResult(c, result_path,size);

    file_a.close();
    file_b.close();
}

int main() {
    for(auto size: scale) {
        cout << "Running, dataset: size " << size << endl;
        Benchmark(size);
        cout << "Passed, dataset: size " << size << endl;
        cout << endl;
    }
    return 0;
}