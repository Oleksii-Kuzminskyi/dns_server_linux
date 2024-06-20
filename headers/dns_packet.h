#ifndef DNS_PACKET_H_
#define DNS_PACKET_H_

#include <string>

#define DATAGRAM_SIZE 512
#define HEADER_LEN	  12
#define TID_LEN		  2
#define TYPE_LEN	  2
#define CLASS_LEN	  2

#define OFFSET_AT 0xc0

#define A_TYPE			 0x00,0x01
#define CNAME_TYPE		 0x00,0x05
#define NAMESERVERS_TYPE 0x00,0x02
#define MAILSERVERS_TYPE 0x00,0x0f

#define IN_CLASS		 0x00,0x01

//1 bit
#define MASK_QR_RESPONSE	0b1000'0000
#define MASK_QR_REQUEST		0b0000'0000

//4 bit
#define MASK_OPCODE_INVERSE 0b0100'0000
#define MASK_OPCODE_STATUS  0b0010'0000
#define MASK_OPCODE_STD     0b0000'0000

//1 bit
#define MASK_AA_ISAUTH		0b0000'0100
#define MASK_AA_NOAUTH		0b0000'0000

//1 bit
#define MASK_TC_ISTRUNC		0b0000'0010
#define MASK_TC_NOTRUNC		0b0000'0000

//1 bit
#define MASK_RD_DESIRE		0b0000'0001
#define MASK_RD_NONEED		0b0000'0000

//1 bit
#define MASK_RA_AVAIL		0b1000'0000
#define MASK_RA_NOAVAIL		0b0000'0000

//3 bit
#define MASK_RESERVED		0b0000'0000

//4 bit
#define MASK_RCODE_NOERR	0b0000'0000
#define MASK_RCODE_FORMAT	0b0000'0001 
#define MASK_RCODE_SERVERR	0b0000'0010 
#define MASK_RCODE_NAMERR	0b0000'0011 
#define MASK_RCODE_NOIMPL	0b0000'0100 
#define MASK_RCODE_REFUSED	0b0000'0101 

typedef unsigned char uint8_t;

/**
 * @brief Retreives IP adddress by given domain name
 * 
 * This is an exception-free wrapper for static 'Poco::Net::DNS::resolveOne' method.
 * 
 * @param name requested name for resolving
 * @param ip destination buffer where resolved ip will be stored
 * 
 * @return 1 - success, 0 - failure
 */
extern bool get_ip_from_domain_name(const std::string& name, std::string& ip);




/**
 * @brief extracts requested domain name from the given datagram
 * 
 * @param datagram a buffer, containig DNS request datagram
 * 
 * IMPORTANT: the function assumes that the buffer is valid
 * and it's size is sufficient. Does NOT check the buffer's size.
 * It is recommended to provide a 512-byte chuck of stack memory.
 * 
 * @return a string representation of a domain name, or empty string 
 *		   if the provided DNS datgram is incorrect
 * 
 * IMPORTANT: the function may also return incorrect name,
 *			  as it relies on the correctness of DNS request datagram
 * 
 */
extern std::string extract_query_name(const uint8_t* datagram);


/**
 * @brief builds a response, based on provided request, resolved IP
 *		  address and response code mask
 * 
 * @param resbuf a result DNS response buffer
 * @param reqbuf a request DNS buffer, needed to retreive some information
 * @param ip a resolved ip address
 * @param RCODE_MASK a response code, indicating on appearing some errors (server errors, format errors, etc.)
 * 
 * @return the number of built bytes
 * 
 * IMPORTANT: the function assumes that the buffers resbuf and reqbuf are valid
 * and their size is sufficient. Does NOT check the buffers' size.
 * It is recommended to provide a 512-byte chuck of stack memory.
 * Also parameter ip is NOT used if RCODE_MASK is not MASK_RCODE_NOERR (according to DNS doc)
 */
extern int build_response(uint8_t* resbuf, const uint8_t* reqbuf,
						  const std::string& ip, const uint8_t RCODE_MASK = MASK_RCODE_NOERR);


#endif
