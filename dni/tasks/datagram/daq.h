#pragma once

#include <pcap.h>

namespace dni {

void parse_single_packet(u_char* args, const struct pcap_pkthdr* hdr, const u_char* body);

}
