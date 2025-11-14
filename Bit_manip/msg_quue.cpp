#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <unordered_map>
#include <iostream>
using namespace std;
void create_file(const char* name, int permissions) {
 if (permissions == 0) {
   printf("Created read-only file of name %s\n", name);
 } else if (permissions == 1) {
   printf("Created read-write file of name %s\n", name);
 } else {
   printf("Unexpected permissions: %d\n", permissions);
 }
}
struct MsgState {
 bool started = false;
 int num_packets = -1;          // set on seq==0
 int permissions = -1;          // set on seq==0
 // store two bytes for each sequence index we’ve seen
 unordered_map<int, pair<uint8_t,uint8_t>> seg_by_seq;
};
static array<MsgState, 4> g_msgs;
static void maybe_finalize_and_reset(uint32_t msg_id) {
 MsgState &st = g_msgs[msg_id];
 if (!st.started || st.num_packets <= 0) return;
 // Check if we have all sequences 0..num_packets-1
 for (int s = 0; s < st.num_packets; ++s) {
   if (st.seg_by_seq.find(s) == st.seg_by_seq.end()) return; // not complete yet
 }
 // Assemble bytes in order of sequence
 string name_bytes;
 name_bytes.reserve(st.num_packets * 2);
 for (int s = 0; s < st.num_packets; ++s) {
   auto [hi, lo] = st.seg_by_seq[s];
   name_bytes.push_back(static_cast<char>(hi));
   name_bytes.push_back(static_cast<char>(lo));
 }
 // Stop at first '\0'
 string fname;
 for (unsigned char c : name_bytes) {
   if (c == '\0') break;
   fname.push_back(static_cast<char>(c));
 }
 if (st.permissions == -1) {
   // permissions never set (shouldn’t happen if protocol is followed)
   // choose safe default: read-only
   st.permissions = 0;
 }
 create_file(fname.c_str(), st.permissions);
 // Reset state for this Message ID
 st = MsgState{};
}
void process_packet(uint32_t packet) {
 // Layout (bit ranges inclusive):
 // 30-31: Message ID (2 bits)
 // 27-29: Num Packets (3 bits)   [valid only when seq==0]
 // 24-26: Sequence Num (3 bits)
 // 20-23: Permissions (4 bits)   [valid only when seq==0]
 //  4-19: Filename Seg (16 bits) -> two bytes: high then low
 //  0-3 : Error Code (4 bits)
 uint32_t msg_id     = (packet >> 30) & 0x3;
 uint32_t num_pkts   = (packet >> 27) & 0x7;
 uint32_t seq        = (packet >> 24) & 0x7;
 uint32_t perms      = (packet >> 20) & 0xF;
 uint32_t seg16      = (packet >> 4)  & 0xFFFF;
 uint32_t err        =  packet        & 0xF;
 // Optional: drop/ignore packets with error codes (if nonzero)
 if (err != 0) {
   // For robustness you might log and return.
   // fprintf(stderr, "Dropping packet: error code %u\n", err);
   return;
 }
 MsgState &st = g_msgs[msg_id];
 // First packet of this message (sequence 0) carries num_packets & permissions
 if (seq == 0) {
   st.started = true;
   st.num_packets = static_cast<int>(num_pkts);
   st.permissions = static_cast<int>(perms);
   st.seg_by_seq.clear();  // starting (or restarting) a message
 } else {
   // If we haven’t seen seq 0 yet, we still accept the segment but can’t finalize
   if (!st.started) {
     st.started = true;
     // num_packets/permissions unknown until we see seq 0
   }
 }
 // Split the 16-bit segment into two bytes, high then low (per example: 0x4E,0x65,...)
 uint8_t hi = static_cast<uint8_t>((seg16 >> 8) & 0xFF);
 uint8_t lo = static_cast<uint8_t>( seg16       & 0xFF);
 st.seg_by_seq[static_cast<int>(seq)] = {hi, lo};
 // If we already know num_packets (got seq 0), try to finalize
 if (st.num_packets > 0) {
   maybe_finalize_and_reset(msg_id);
 }
}
int main() {
 // === Example test case
 // this should write a file of name “puppy” (with trailing zeros aka null character) of permissions R/W
 uint32_t test_case_1[3] = {0x18170750, 0x01070700, 0x02079000};
 // For the below test cases we have:
 // Message 0 has text “Mario” with permissions 1 (Read-write)
 // Message 1 has text “Luigi” with permissions 0 (Read-only)
 // Two messages, back to back, with packets in order
 uint32_t test_case_2[6] = {0x1814d610, 0x01172690, 0x0216f000, 0x5804c750, 0x04106967, 0x04206900};
 // Two messages interspersed, packets in order
 uint32_t test_case_3[6] = {0x1814d610, 0x5804c750, 0x04106967, 0x01172690, 0x0216f000, 0x04206900};
 // Two messages interspersed, packets after the first packet out of order
 uint32_t test_case_4[6] = {0x1814d610, 0x5804c750, 0x01172690, 0x0216f000, 0x04206900, 0x04106967};
 for (uint32_t p : test_case_1) process_packet(p);
 for (uint32_t p : test_case_2) process_packet(p);
 for (uint32_t p : test_case_3) process_packet(p);
 for (uint32_t p : test_case_4) process_packet(p);
 return 0;
}