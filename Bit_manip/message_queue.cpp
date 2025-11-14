#include<iostream>
#include<string>
#include<unordered_map>
#include<array>

using namespace std;

struct Mpkt{
bool started=false;
uint32_t permissions=-1;
uint32_t num_packets=-1;
unordered_map<int,pair<uint8_t,uint8_t>> seq_by_len;
};

static array<Mpkt,4> msg;

void create_file(const char* name, int permissions) {
 if (permissions == 0) {
   printf("Created read-only file of name %s\n", name);
 } else if (permissions == 1) {
   printf("Created read-write file of name %s\n", name);
 } else {
   printf("Unexpected permissions: %d\n", permissions);
 }
}

static void final_create_file(uint32_t msg_id)
{
    Mpkt &m= msg[msg_id];
    if(!m.started || m.num_packets<=0) return;

    for(int s=0;s<m.num_packets;s++)
    {
        if(m.seq_by_len.find(s)==m.seq_by_len.end()) return;
    }

    string name_bytes;
    name_bytes.reserve(m.num_packets * 2);
    for (int s = 0; s < m.num_packets; ++s) {
    auto [hi, lo] = m.seq_by_len[s];
    name_bytes.push_back(static_cast<char>(hi));
    name_bytes.push_back(static_cast<char>(lo));
    }
 // Stop at first '\0'
 string fname;
 for (unsigned char c : name_bytes) {
   if (c == '\0') break;
   fname.push_back(static_cast<char>(c));
 }
 if (m.permissions == -1) {
   // permissions never set (shouldn’t happen if protocol is followed)
   // choose safe default: read-only
   m.permissions = 0;
 }
 create_file(fname.c_str(), m.permissions);
 // Reset state for this Message ID
 m = Mpkt{};

}
void process_packet(uint32_t packet)
{
     // Layout (bit ranges inclusive):
 // 30-31: Message ID (2 bits)
 // 27-29: Num Packets (3 bits)   [valid only when seq==0]
 // 24-26: Sequence Num (3 bits)
 // 20-23: Permissions (4 bits)   [valid only when seq==0]
 //  4-19: Filename Seg (16 bits) -> two bytes: high then low
 //  0-3 : Error Code (4 bits)
    uint32_t perm =(packet>>20)&0xF;
    uint32_t error_=packet&0xF;
    uint32_t num_packet_=(packet>>27)&0x7;
    uint32_t msg_id_=(packet>>30)&0x3;
    uint32_t fileseg=(packet>>4)&0xFFFF;
    uint32_t seq=(packet>>24)&0x7;

    if(error_!=0) return;

    Mpkt &m= msg[msg_id_];
    if(seq==0)
        {   
            m.started=true;
            m.num_packets=static_cast<int>(num_packet_);
            m.permissions=static_cast<int>(perm);
            m.seq_by_len.clear();
        }
    else
        {
            if(!m.started) m.started=true;
        }
uint8_t hi = static_cast<uint8_t>((fileseg>>8) & 0xFF);
uint8_t lo = static_cast<uint8_t>(fileseg & 0xFF);
m.seq_by_len[static_cast<int>(seq)]={hi,lo};

//cout<<"Print num_packet"<<m.num_packets<<endl;
if(m.num_packets >0)
{
   // cout<<"Sending final_create_file" <<endl;
    final_create_file(msg_id_);
}

}
int main()
{
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
 
 cout<<"exec TC 1"<<endl;
 for (uint32_t p : test_case_1) process_packet(p);
  cout<<"exec TC 2"<<endl;
 for (uint32_t p : test_case_2) process_packet(p);
  cout<<"exec TC 3"<<endl;
 for (uint32_t p : test_case_3) process_packet(p);
  cout<<"exec TC 4"<<endl;
 for (uint32_t p : test_case_4) process_packet(p);
 return 0;
}