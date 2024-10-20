#include<pcap.h>
#include<stdio.h>
#include<stdlib.h> // for exit()
#include<string.h> //for memset

#include<sys/socket.h>
#include<arpa/inet.h> // for inet_ntoa()
#include<net/ethernet.h>
#include<netinet/ip_icmp.h>   //Provides declarations for icmp header
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header

void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void process_ip_packet(const u_char *, int);
void print_ip_packet(const u_char *, int);
void print_tcp_packet(const u_char *, int);
void print_udp_packet(const u_char *, int);
void print_icmp_packet(const u_char *, int);
void PrintData(const u_char *, int);

struct sockaddr_in source, dest;
int tcp = 0, udp = 0, icmp = 0, others = 0, igmp = 0, total = 0, i, j;

int main() {
    pcap_if_t *alldevsp, *device;
    pcap_t *handle; // Handle of the device that shall be sniffed

    char errbuf[100], *devname, devs[100][100];
    int count = 1, n;

    // get the list of available devices
    printf("Finding available devices ... ");
    if (pcap_findalldevs(&alldevsp, errbuf)) {
        printf("Error finding devices : %s\n", errbuf);
        exit(1);
    }
    printf("Done\n");

    // Print the available devices
    printf("\nAvailable Devices are:\n");
    for (device = alldevsp; device != NULL; device = device->next) {
        printf("%d. %s - %s\n", count, device->name, device->description);
        if (device->name != NULL) {
            strcpy(devs[count], device->name);
        }
        count++;
    }

    // Ask user which device to sniff
    printf("Enter the number of the device you want to sniff: ");
    scanf("%d", &n);
    devname = devs[n];

    // Open the device for sniffing
    printf("Opening device %s for sniffing ... ", devname);
    handle = pcap_open_live(devname, 65535, 1, 0, errbuf);

    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", devname, errbuf);
        exit(1);
    }
    printf("Done\n");

    // Put the device in sniff loop
    pcap_loop(handle, 0, process_packet, NULL);
    return 0;   
}

void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer) {
    int size = header->len;

    // Get the IP Header part of this packet, excluding the ethernet header
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    ++total;
    switch (iph->protocol) { // Check the Protocol and do accordingly...
        case 1:  // ICMP Protocol
            ++icmp;
            print_icmp_packet(buffer, size);
            break;
        case 2:  // IGMP Protocol
            ++igmp;
            break;
        case 6:  // TCP Protocol
            ++tcp;
            print_tcp_packet(buffer, size);
            break;
        case 17: // UDP Protocol
            ++udp;
            print_udp_packet(buffer, size);
            break;
        default: // Some Other Protocol like ARP etc.
            ++others;
            break;
    }
    printf("TCP: %d   UDP: %d   ICMP: %d   IGMP: %d   Others: %d   Total: %d\r", tcp, udp, icmp, igmp, others, total);
}

void print_ethernet_header(const u_char *Buffer, int Size) {
    struct ethhdr *eth = (struct ethhdr *)Buffer;

    printf("\n");
    printf("Ethernet Header\n");
    printf("   |-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    printf("   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("   |-Protocol            : %u\n", (unsigned short)eth->h_proto);
}

void print_ip_header(const u_char *Buffer, int Size) {
    print_ethernet_header(Buffer, Size);

    unsigned short iphdrlen;
    struct iphdr *iph = (struct iphdr *)(Buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;

    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;

    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;

    printf("\n");
    printf("IP Header\n");
    printf("   |-IP Version        : %d\n", (unsigned int)iph->version);
    printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n", (unsigned int)iph->ihl, ((unsigned int)(iph->ihl)) * 4);
    printf("   |-Type Of Service   : %d\n", (unsigned int)iph->tos);
    printf("   |-IP Total Length   : %d Bytes(Size of Packet)\n", ntohs(iph->tot_len));
    printf("   |-Identification    : %d\n", ntohs(iph->id));
    printf("   |-TTL      : %d\n", (unsigned int)iph->ttl);
    printf("   |-Protocol : %d\n", (unsigned int)iph->protocol);
    printf("   |-Checksum : %d\n", ntohs(iph->check));
    printf("   |-Source IP        : %s\n", inet_ntoa(source.sin_addr));
    printf("   |-Destination IP   : %s\n", inet_ntoa(dest.sin_addr));
}

void print_tcp_packet(const u_char *Buffer, int Size) {
    unsigned short iphdrlen;
    struct iphdr *iph = (struct iphdr *)(Buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;

    struct tcphdr *tcph = (struct tcphdr *)(Buffer + iphdrlen + sizeof(struct ethhdr));

    int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff * 4;

    printf("\n\n***********************TCP Packet*************************\n");
    print_ip_header(Buffer, Size);

    printf("\n");
    printf("TCP Header\n");
    printf("   |-Source Port      : %u\n", ntohs(tcph->source));
    printf("   |-Destination Port : %u\n", ntohs(tcph->dest));
    printf("   |-Sequence Number  : %u\n", ntohl(tcph->seq));
    printf("   |-Acknowledge Number : %u\n", ntohl(tcph->ack_seq));
    printf("   |-Header Length    : %d DWORDS or %d BYTES\n", (unsigned int)tcph->doff, (unsigned int)tcph->doff * 4);
    printf("   |-Urgent Flag      : %d\n", (unsigned int)tcph->urg);
    printf("   |-Acknowledgement Flag : %d\n", (unsigned int)tcph->ack);
    printf("   |-Push Flag        : %d\n", (unsigned int)tcph->psh);
    printf("   |-Reset Flag       : %d\n", (unsigned int)tcph->rst);
    printf("   |-Synchronise Flag : %d\n", (unsigned int)tcph->syn);
    printf("   |-Finish Flag      : %d\n", (unsigned int)tcph->fin);
    printf("   |-Window           : %d\n", ntohs(tcph->window));
    printf("   |-Checksum         : %d\n", ntohs(tcph->check));
    printf("   |-Urgent Pointer   : %d\n", tcph->urg_ptr);
    printf("\n");
    printf("                        DATA Dump                         ");
    printf("\n");

    printf("IP Header\n");
    PrintData(Buffer, iphdrlen);

    printf("TCP Header\n");
    PrintData(Buffer + iphdrlen, tcph->doff * 4);

    printf("Data Payload\n");
    PrintData(Buffer + header_size, Size - header_size);

    printf("\n###########################################################");
}

void print_udp_packet(const u_char *Buffer, int Size) {
    unsigned short iphdrlen;
    struct iphdr *iph = (struct iphdr *)(Buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;

    struct udphdr *udph = (struct udphdr *)(Buffer + iphdrlen + sizeof(struct ethhdr));

    int header_size = sizeof(struct ethhdr) + iphdrlen + sizeof udph;

    printf("\n\n***********************UDP Packet*************************\n");
    print_ip_header(Buffer, Size);

    printf("\nUDP Header\n");
    printf("   |-Source Port      : %d\n", ntohs(udph->source));
    printf("   |-Destination Port : %d\n", ntohs(udph->dest));
    printf("   |-UDP Length       : %d\n", ntohs(udph->len));
    printf("   |-UDP Checksum     : %d\n", ntohs(udph->check));

    printf("\n");
    printf("IP Header\n");
    PrintData(Buffer, iphdrlen);

    printf("UDP Header\n");
    PrintData(Buffer + iphdrlen, sizeof udph);

    printf("Data Payload\n");

    // Move the pointer ahead and reduce the size of string
    PrintData(Buffer + header_size, Size - header_size);

    printf("\n###########################################################");
}

void print_icmp_packet(const u_char *Buffer, int Size) {
    unsigned short iphdrlen;
    struct iphdr *iph = (struct iphdr *)(Buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;

    struct icmphdr *icmph = (struct icmphdr *)(Buffer + iphdrlen + sizeof(struct ethhdr));

    int header_size = sizeof(struct ethhdr) + iphdrlen + sizeof icmph;

    printf("\n\n***********************ICMP Packet*************************\n");
    print_ip_header(Buffer, Size);

    printf("\n");
    printf("ICMP Header\n");
    printf("   |-Type : %d", (unsigned int)(icmph->type));

    if ((unsigned int)(icmph->type) == 11) {
        printf("  (TTL Expired)\n");
    } else if ((unsigned int)(icmph->type) == ICMP_ECHOREPLY) {
        printf("  (ICMP Echo Reply)\n");
    }

    printf("   |-Code : %d\n", (unsigned int)(icmph->code));
    printf("   |-Checksum : %d\n", ntohs(icmph->checksum));
    // printf("   |-ID       : %d\n", ntohs(icmph->id));
    // printf("   |-Sequence : %d\n", ntohs(icmph->sequence));
    printf("\n");

    printf("IP Header\n");
    PrintData(Buffer, iphdrlen);

    printf("ICMP Header\n");
    PrintData(Buffer + iphdrlen, sizeof icmph);

    printf("Data Payload\n");

    // Move the pointer ahead and reduce the size of string
    PrintData(Buffer + header_size, (Size - header_size));

    printf("\n###########################################################");
}

void PrintData(const u_char *data, int Size) {
    int i, j;
    for (i = 0; i < Size; i++) {
        if (i != 0 && i % 16 == 0) { // if one line of hex printing is complete...
            printf("         ");
            for (j = i - 16; j < i; j++) {
                if (data[j] >= 32 && data[j] <= 128)
                    printf("%c", (unsigned char)data[j]); // if it's a number or alphabet
                else
                    printf("."); // otherwise print a dot
            }
            printf("\n");
        }

        if (i % 16 == 0) printf("   ");
        printf(" %02X", (unsigned int)data[i]);

        if (i == Size - 1) { // print the last spaces
            for (j = 0; j < 15 - i % 16; j++) {
                printf("   "); // extra spaces
            }

            printf("         ");

            for (j = i - i % 16; j <= i; j++) {
                if (data[j] >= 32 && data[j] <= 128) {
                    printf("%c", (unsigned char)data[j]);
                } else {
                    printf(".");
                }
            }

            printf("\n");
        }
    }
}
