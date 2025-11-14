#include <cstdint>
#include <cstdio>
#include <array>
#include <vector>
#include <unordered_map>
#include <string>
#include <cassert>
#include <iomanip>
#include <sstream>
#include<iostream>

using namespace std;
// ---------------- helpers ----------------
inline uint32_t fld(uint32_t x, int lo, int hi){
   return (x >> lo) & ((1u<<(hi-lo+1)) - 1u);
}
inline uint32_t ch_id (uint32_t p){ return fld(p,30,31); }
inline uint32_t totalN(uint32_t p){ return fld(p,27,29); }  // valid on seq==0
inline uint32_t seq   (uint32_t p){ return fld(p,24,26); }
inline uint8_t  type8 (uint32_t p){ return static_cast<uint8_t>(fld(p,16,23)); }
inline uint8_t  len8  (uint32_t p){ return static_cast<uint8_t>(fld(p, 8,15)); } // valid on seq==0
inline uint8_t  pay8  (uint32_t p){ return static_cast<uint8_t>(fld(p, 0, 7)); }
struct ChanState {
   bool started = false;
   int N = -1;        // total fragments expected
   int L = -1;        // total value bytes (equal to N in this mock)
   int T = -1;        // type
   unsigned seen = 0; // bitmask for seq seen (0..7)
   std::array<uint8_t,8> val{}; // payload bytes by seq
};
static std::array<ChanState,4> CH;
// ---------------- implement me ----------------

void process_util(uint32_t c)
{
    ChanState &st = CH[c];
    if(!st.started || st.N<=0) return;
    for(int s=0;s<st.N;s++)
    {
        if(!((st.seen>>s)&1)) return;
    }

    
    std::cout << "CH" << c << " T=" << st.T << " L=" << st.L << " V=";
    for (int s = 0; s < st.N; s++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)st.val[s] << " ";
    }
    std::cout << std::dec << std::endl; // reset to decimal output

    st=ChanState{};
}
void process_packet(uint32_t pkt){
   // Extract fields
   uint32_t c  = ch_id(pkt);
   uint32_t s  = seq(pkt);
   uint32_t N0 = totalN(pkt);   // only meaningful if s==0
   uint8_t  T0 = type8(pkt);
   uint8_t  L0 = len8(pkt);     // only meaningful if s==0
   uint8_t  P  = pay8(pkt);
   ChanState &st = CH[c];
   // TODO:
   // - If s==0: set started=true, N=N0, L=L0, T=T0; clear seen; clear val
   // - Store P at val[s] only if not already seen
   // - Mark seen bit for s
   // - If N is known (>0), check if all seq 0..N-1 seen
   //   * If complete: print line:  CH<c> T=<T> L=<L> V=<hex...>
   //     with hex bytes val[0], val[1], ... val[N-1]
   //   * Then reset st to default (ready for next TLV on channel)

   
if (s == 0) {
    // If already started and this is a new TLV, reset
    if (st.started) {
        st.N = N0;
        st.L = L0;
        st.T = T0;
    }
    else
    {
    st.started = true;
    st.N = N0;
    st.L = L0;
    st.T = T0;
    st.seen=0;
    st.val.fill(0);
    }
    }   
   else
   {
        if(!st.started) st.started=true;
   }

if (!((st.seen >> s) & 0x1))
    {
        st.seen|=(1<<s);
        st.val[s] = P;
    }

if(st.N>0)
{
    process_util(c);
}

}
// ---------------- test harness ----------------
static uint32_t build_pkt(uint8_t ch, uint8_t N, uint8_t s, uint8_t T, uint8_t L, uint8_t P){
   assert(ch < 4 && N < 8 && s < 8);
   uint32_t p = 0;
   p |= (uint32_t(ch) & 0x3u) << 30;
   p |= (uint32_t(N)  & 0x7u) << 27;
   p |= (uint32_t(s)  & 0x7u) << 24;
   p |= (uint32_t(T)  & 0xFFu) << 16;
   p |= (uint32_t(L)  & 0xFFu) << 8;
   p |= (uint32_t(P)  & 0xFFu);
   return p;
}

int main(){

   std::vector<uint32_t> stream;
   // Case A: CH0, N=3, T=0x10, L=3, payload [0x0a, 0xff, 0x35], out-of-order
   stream.push_back(build_pkt(0,3,0, 0x10,3, 0x0a)); // seq0 (defines N,L,T) P=0a
   stream.push_back(build_pkt(0,3,2, 0x10,0, 0x35)); // seq2 P=35
   stream.push_back(build_pkt(0,3,1, 0x10,0, 0xff)); // seq1 P=ff
   // -> expect finalize: CH0 T=16 L=3 V=0a ff 35
   // Case B: CH1, N=2, T=0x21, L=2, interleaved with CH0 new TLV
   stream.push_back(build_pkt(1,2,0, 0x21,2, 0x11)); // CH1 seq0 P=11
   // CH0 new TLV starts before CH1 completes:
   stream.push_back(build_pkt(0,2,0, 0x33,2, 0xaa)); // CH0 seq0 P=aa (new TLV resets CH0)
   stream.push_back(build_pkt(1,2,1, 0x21,0, 0x22)); // CH1 seq1 P=22
   // -> expect finalize CH1
   stream.push_back(build_pkt(0,2,1, 0x33,0, 0xbb)); // CH0 seq1 P=bb
   // -> expect finalize CH0
   // Case C: CH2, N=4, seqs arrive before 0 (buffering)
   stream.push_back(build_pkt(2,0,2, 0x44,0, 0x0c)); // seq2 P=0c (N/L unknown yet)
   stream.push_back(build_pkt(2,0,1, 0x44,0, 0x0b)); // seq1 P=0b
   stream.push_back(build_pkt(2,4,0, 0x44,4, 0x0a)); // seq0 defines N=4,L=4,P=0a
   stream.push_back(build_pkt(2,0,3, 0x44,0, 0x0d)); // seq3 P=0d
   // -> expect finalize CH2
   for (auto p : stream) process_packet(p);
   return 0;
}
/*
===== Expected output (exact lines) =====
CH0 T=16 L=3 V=0a ff 35
CH1 T=33 L=2 V=11 22
CH0 T=51 L=2 V=aa bb
CH2 T=68 L=4 V=0a 0b 0c 0d
*/