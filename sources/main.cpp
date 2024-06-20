#include "main.h"
#include "dns_packet.h"
#include "sqlite_utils.h"

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/DatagramSocket.h>


static bool setup_check(int argc, char* argv[], const char * db_name);
static bool db_refresh_need(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	using namespace Poco::Net;
	const char* DBNAME = "DNS.db";

	if (!setup_check(argc, argv, DBNAME))
	{
		return -1;
	}

	sqlite::database db(DBNAME);
	if (!create_table(db))
	{
		return -1;
	}
	if (db_refresh_need(argc, argv))
	{
		refresh_db(db);
		DBG("\nRefresh completed..");
	}

	DatagramSocket server(SocketAddress(argv[1]), true); //true - reuse address
	uint8_t DNS_datagram_in[DATAGRAM_SIZE + 1];
	uint8_t DNS_datagram_out[DATAGRAM_SIZE + 1];
	
	while (true)
	{
		SocketAddress client_address;
		memset(DNS_datagram_in, 0x00, sizeof(DNS_datagram_in));
		int received_bytes = 0;
		try
		{
			received_bytes = server.receiveFrom(DNS_datagram_in, DATAGRAM_SIZE, client_address);
		}
		catch (const std::exception & ex)
		{
			DBG(ex.what());
			continue;
		}
		DBG("\n\n\nIncoming message from " << client_address.toString());
		DBG("\nreceived bytes: " << received_bytes);
		DBG(Hexdump(DNS_datagram_in, received_bytes));

		// parse datagram for requested name
		std::string request_domain_name = extract_query_name(DNS_datagram_in);
		if (request_domain_name.empty())
		{
			DBG("\nCould not extract query name");
			continue; //to think: return format error?
		}
		DBG("\nRequested name: " << request_domain_name);

		std::string ip;
		uint8_t RESPONSE_CODE_MASK = MASK_RCODE_NOERR;

		if (name_present_in_db(db, request_domain_name))
		{
			if (!get_ip_by_name_in_db(db, request_domain_name, ip))
			{
				if (get_ip_from_domain_name(request_domain_name, ip))
				{
					add_record_in_db(db, request_domain_name, ip);
					// if fail to add - not client's problem
				}
				else
				{
					RESPONSE_CODE_MASK = MASK_RCODE_NAMERR;
				}
			}
		}
		else
		{
			if (get_ip_from_domain_name(request_domain_name, ip))
			{
				add_record_in_db(db, request_domain_name, ip);
				// if fail to add - not client's problem
			}
			else
			{
				RESPONSE_CODE_MASK = MASK_RCODE_NAMERR;
			}
		}
		DBG("\nResolved ip: " << ip);

		// building response
		memset(DNS_datagram_out, 0x00, sizeof(DNS_datagram_out));
		int bytes_build = build_response(DNS_datagram_out, DNS_datagram_in, ip, RESPONSE_CODE_MASK);
		DBG("\nBuilded response (" << std::dec << bytes_build << " bytes): ");
		DBG(Hexdump(DNS_datagram_out, bytes_build));

		//sending response back to client
		int send_bytes = 0;
		try
		{
			send_bytes = server.sendTo(DNS_datagram_out, bytes_build, client_address);
		}
		catch (const std::exception & ex)
		{
			DBG(ex.what());
		}
		DBG("\nBytes sent: " << std::dec << send_bytes);
	}

	return 0;
}

static bool setup_check(int argc, char* argv[], const char* db_name)
{
	if (argc < 2)
	{
		DBG("Too few arguments");
		return false;
	}

	try
	{
		Poco::Net::SocketAddress guard_address(argv[1]); //example: argv[1] = "192.168.1.34:53"
		Poco::Net::DatagramSocket guard_socket(guard_address, true);
		sqlite::database guard_database(db_name);
	}
	catch (const std::exception &ex)
	{
		DBG(ex.what());
		return false;
	}
	catch (...)
	{
		DBG("Unknown exception");
		return false;
	}



	return true;
}

static bool db_refresh_need(int argc, char* argv[])
{
	if (argc < 3)
	{
		DBG("\nDatabase refresh not needed");
		return false;
	}
	if (std::string(argv[2]) != "refresh")
	{
		DBG("\nIncorrect 2nd argument, type \'refresh\' instead");
		return false;
	}
	DBG("\nRefresh database needed");

	return true;
}