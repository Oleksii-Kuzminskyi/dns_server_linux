#include "main.h"
#include "dns_packet.h"

#include <Poco/Net/IPAddress.h>
#include <Poco/Net/DNS.h>

extern bool get_ip_from_domain_name(const std::string& name, std::string& ip)
{
	using namespace Poco::Net;
	try
	{
		IPAddress ip_addr = DNS::resolveOne(name);
		ip = ip_addr.toString();
		return true;
	}
	catch (const std::exception & ex)
	{
		DBG("\nCould not resolve name:" << name << ", reason: " << ex.what());
	}
	return false;
}

extern std::string extract_query_name(const uint8_t* datagram)
{

	int string_part_size = 0;

	// size and data alternate.
	// 
	// example (HEX):
	// 06  67  6f  6f  67  6c  65  03  63  6f  6d  00    
	// |   |   |   |   |   |   |   |   |   |   |   |
	// |  'g' 'o' 'o' 'g' 'l' 'e'  |  'c' 'o' 'm'  |
	// |					       |			   |
	// <string_part_size>	<string_part_size>   <end>

	std::string result;
	const uint8_t* temp = datagram;
	temp += HEADER_LEN;

	for (; temp && *temp != 0x00; temp += string_part_size)
	{

		string_part_size = *temp; //obtain number of characters to read
		++temp;					  //now temp points to data

		for (int i = 0; i < string_part_size; i++)
		{
			result.push_back(temp[i]);
		}
		result.push_back('.');
	}
	result.pop_back();	       // 'google.com.' --> 'google.com'
	temp = nullptr;

	return result;
}

static int build_header(uint8_t* & resbuf, const uint8_t* & reqbuf, uint8_t RCODE_MASK)
{
	int bytes_build = 0;

	// Transaction ID (2 bytes)
	memcpy(resbuf, reqbuf, TID_LEN);
#ifdef MAKE_DEBUG
	uint8_t* header_start = resbuf;
#endif
	DBG("\nTID:");
	DBG(Hexdump(resbuf, TID_LEN));

	bytes_build += TID_LEN;
	resbuf += TID_LEN;
	reqbuf += TID_LEN;


	// Flags (2 bytes)
		//------1st byte
	*resbuf = *reqbuf;
	*resbuf &= ~MASK_QR_REQUEST;
	*resbuf |= (MASK_QR_RESPONSE | MASK_AA_ISAUTH);
	DBG("\nFlag 1 byte:");
	DBG(Hexdump(resbuf, 1));
	++bytes_build;
	++resbuf;
	++reqbuf;

		//------2nd byte
	*resbuf = (MASK_RA_NOAVAIL | MASK_RESERVED | RCODE_MASK);
	DBG("\nFlag 2 byte:");
	DBG(Hexdump(resbuf, 1));
	++bytes_build;
	++resbuf;
	++reqbuf;

	// QDCOUNT (2 bytes), ANCOUNT (2 bytes), NSCOUNT (2 bytes), ARCOUNT (2 bytes)
	uint8_t header_tail[] = {
		0x00, 0x01,			//QDCOUNT
		0x00, 0x00,			//ANCOUNT
		0x00, 0x00,			//NSCOUNT - zero for simplicity
		0x00, 0x00,			//ARCOUNT - zero for simplicity
	};
	if (RCODE_MASK == MASK_RCODE_NOERR)
	{
		//othwerwise the packet is considered by Wireshark as malformed
		header_tail[3] = 0x01;
	}

	memcpy(resbuf, header_tail, sizeof(header_tail));
	DBG("\nQDCOUNT (2 bytes), ANCOUNT (2 bytes), NSCOUNT (2 bytes), ARCOUNT (2 bytes)");
	DBG(Hexdump(resbuf, sizeof(header_tail)));
	bytes_build += sizeof(header_tail);
	resbuf += sizeof(header_tail);
	reqbuf += sizeof(header_tail);

	DBG("\n==================HEADER========================");
#ifdef MAKE_DEBUG
	std::cout << Hexdump(header_start, bytes_build) << std::endl;
	header_start = nullptr;
#endif

	return bytes_build;
}

static int build_question(uint8_t* & resbuf, const uint8_t* & reqbuf)
{
	int bytes_build = 0;

	// NAME (var bytes, but terminates with 0x00)
	int request_name_size = strlen(static_cast<const char*>(static_cast<const void*>(reqbuf)));
	memcpy(resbuf, reqbuf, request_name_size + 1);
#ifdef MAKE_DEBUG
	uint8_t* question_start = resbuf;
#endif
	DBG("\nNAME:");
	DBG(Hexdump(resbuf, request_name_size + 1));
	bytes_build += request_name_size + 1;
	resbuf += request_name_size + 1;
	reqbuf += request_name_size + 1;

	// TYPE (2 bytes), CLASS (2 bytes)
	memcpy(resbuf, reqbuf, TYPE_LEN + CLASS_LEN);
	DBG("\nTYPE (2 bytes), CLASS (2 bytes):");
	DBG(Hexdump(resbuf, TYPE_LEN + CLASS_LEN));

	bytes_build += TYPE_LEN + CLASS_LEN;
	resbuf += TYPE_LEN + CLASS_LEN;
	reqbuf += TYPE_LEN + CLASS_LEN;

	DBG("\n==================QUESTION========================");
#ifdef MAKE_DEBUG
	std::cout << Hexdump(question_start, bytes_build) << std::endl;
	question_start = nullptr;
#endif

	return bytes_build;
}

static int build_answer(uint8_t* & resbuf, const std::string& ip)
{
	int bytes_build = 0;
	uint8_t answer[16] = {
	OFFSET_AT, HEADER_LEN,	  // NAME, but with packet compression
		   A_TYPE,			  // TYPE
		   IN_CLASS,		  // CLASS
	 0x00, 0x01, 0x51, 0x80,  // TTL = 0x15180 = 86400s = 24h
		  0x00, 0x04,		  // RD Length
	 0x00, 0x00, 0x00, 0x00,  // RDATA
	};

	Poco::Net::IPAddress::RawIPv4 ip_array = Poco::Net::IPAddress(ip).toV4Bytes();
	for (size_t i = 0; i < ip_array.size(); i++)
	{
		answer[i + 12] = ip_array[i];
	}
	memcpy(resbuf, answer, 16);
	DBG("\n==================ANSWER========================");
	DBG(Hexdump(resbuf, 16));
	bytes_build += 16;


	return bytes_build;
}

extern int build_response(uint8_t* resbuf, const uint8_t* reqbuf,
						  const std::string& ip, const uint8_t RCODE_MASK)
{
	DBG("\nStart building response..");

	uint8_t* res_ptr = resbuf;
	const uint8_t* req_ptr = reqbuf;
	int bytes_build_total = 0;

	bytes_build_total += build_header(res_ptr, req_ptr, RCODE_MASK);
	bytes_build_total += build_question(res_ptr, req_ptr);
	if (RCODE_MASK == MASK_RCODE_NOERR)
	{
		bytes_build_total += build_answer(res_ptr, ip);
	}
	// AUTHORITY and ADDITIONAL fields are not used in this project

	res_ptr = nullptr;
	req_ptr = nullptr;

	return bytes_build_total;
}
