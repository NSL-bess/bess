#include <iostream>
#include <boost/circular_buffer.hpp>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <unistd.h>
#include "histogram.h"
#include <queue>

using namespace std;
#define HEAP_SIZE 5
#define SIZE 500
uint64_t tsc_hz = 0;


uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
void SetTSC() {
    uint64_t start = rdtsc();
    usleep(100000); // 0.1 sec
    tsc_hz = (rdtsc() - start) *10;
}
double tsc_to_us(uint64_t cycles) {
     return (cycles * 1000000.0) / tsc_hz;
 }
uint64_t GetTailLatency(uint32_t percentile, boost::circular_buffer<uint64_t>& buf) {
    vector<uint64_t> latency_copy;
    // Create a copy of the latency buffer
    for (auto it = buf.begin(); it != buf.end(); it++) {
      latency_copy.push_back(*it);
    }
    sort(latency_copy.begin(), latency_copy.end());
    size_t idx = ceil((percentile / 100.0) * latency_copy.size());
    return latency_copy[idx];
}
uint64_t GetTailUsingHeap(uint32_t percentile, boost::circular_buffer<uint64_t>& buf) {
    vector<uint64_t> latency_copy;
    // Create a copy of the latency buffer
    for (auto it = buf.begin(); it != buf.end(); it++) {
      latency_copy.push_back(*it);
    }
    make_heap(latency_copy.begin(), latency_copy.end());
    size_t count = latency_copy.size() - ceil((percentile / 100.0) * latency_copy.size());
    while (count-1>0) {
        pop_heap(latency_copy.begin(), latency_copy.end());
        count--;
    }
    return latency_copy.front();
}

class MinHeap{
    private:
        priority_queue<uint64_t, vector<uint64_t>, greater<uint64_t>> heap;
        uint64_t num;
    public:
        MinHeap(): heap() , num(0) {}
        void Insert(uint64_t val) {
            if (num <=5) {
                heap.push(val);

            } else if(val > heap.top()) {
                heap.pop();
                heap.push(val);
            }
            num++;
        }
        uint64_t Get_p99() {
            uint64_t percentile = 99;
            size_t idx = ceil((percentile / 100.0) * num);
            idx = num - idx;
            if (idx >=HEAP_SIZE) {
                return heap.top();
            }
            while(idx>0) {
                heap.pop();
            }
            return heap.top();
        }
};
int main() {
    boost::circular_buffer<uint64_t> c_buffer;
    Histogram <uint64_t> hist(10000000, 100);
    Histogram <uint64_t> hist2(100000, 1000);
    Histogram <uint64_t> hist3(200, 1000);
    MinHeap min_heap;
    c_buffer.set_capacity(SIZE);
    for (int i = 0; i< SIZE; i++) {
        uint64_t num = rand();
        c_buffer.push_back(num);
        hist.Insert(num);
        hist2.Insert(num);
        hist3.Insert(num);
        min_heap.Insert(num);
    }
    SetTSC();
    cout<<"TSC: "<<tsc_hz<<endl;
    uint64_t start = rdtsc();
    cout<<GetTailLatency(99, c_buffer)<<endl;
    uint64_t end= rdtsc();
    cout << "Time vector: "<< tsc_to_us(end-start)<<endl;
    vector<double> latency;
    latency.push_back(99);
    
    start = rdtsc();
    hist.Summarize(latency);
    end= rdtsc();
    cout << "Time histogram: "<< tsc_to_us(end-start)<<endl;
   
    start = rdtsc();
    GetTailUsingHeap(99, c_buffer);
    end= rdtsc();
    cout << "Time heap: "<< tsc_to_us(end-start)<<endl;
   
    start = rdtsc();
    hist2.Summarize(latency);
    end= rdtsc();
    cout << "Time histogram2: "<< tsc_to_us(end-start)<<endl;
  
    start = rdtsc();
    hist3.Summarize(latency);
    end= rdtsc();
    cout << "Time histogram3: "<< tsc_to_us(end-start)<<endl;
  
    start = rdtsc();
    min_heap.Get_p99();
    end= rdtsc();
    cout << "min heap: "<< tsc_to_us(end-start)<<endl;
    return 0;
}
