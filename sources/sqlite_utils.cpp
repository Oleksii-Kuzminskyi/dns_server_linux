#include "main.h"
#include "sqlite_utils.h"
#include "dns_packet.h"

extern bool create_table(sqlite::database& db)
{
	try
	{
		db << "create table if not exists NAME_IP ("
			"		NAME text primary key not null,"
			"		IP text not null"
			");";
		return true;
	}
	catch (const std::exception& ex)
	{
		DBG(ex.what());
	}
	catch (...)
	{
		DBG("Unknown exception");
	}

	return false;
}

extern bool name_present_in_db(sqlite::database& db, const std::string& name)
{
	int name_present = 0;
	try
	{
		db << "select count(*) from NAME_IP where NAME = ?" << name >> name_present;
		if (name_present > 0)
		{
			DBG("\nPresent in database");
		}
		else
		{
			DBG("\nNot in database");
		}
		return static_cast<bool>(name_present);
	}
	catch (const std::exception& ex)
	{
		DBG(ex.what());
	}


	return static_cast<bool>(name_present);
}

extern bool get_ip_by_name_in_db(sqlite::database& db, const std::string& name, std::string& ip)
{
	try
	{
		db << "select IP from NAME_IP where NAME = ?" << name >> ip;
		return true;
	}
	catch (const std::exception& ex)
	{
		DBG(ex.what());
	}
	return false;
}

extern bool add_record_in_db(sqlite::database& db, const std::string& name, const std::string& ip)
{
	DBG("\nAdding a record to database");
	try
	{
		db << "insert into NAME_IP (NAME, IP) values (?, ?);" << name << ip;
		return true;
	}
	catch (const std::exception& ex)
	{
		DBG("\nColud not add record");
	}
	return false;
}

extern bool refresh_db(sqlite::database& db)
{
	int rows = 0;
	try
	{
		db << "select count(*) from NAME_IP" >> rows;
	}
	catch (const std::exception & ex)
	{
		DBG(ex.what());
		return false;
	}

	for (int i = 0; i < rows; i++)
	{
		try
		{
			std::string table_name;
			std::string old_ip;
			db << "select NAME, IP from NAME_IP order by rowid limit 1 offset ?" << i >> std::tie(table_name, old_ip);

			std::string fresh_ip;
			if (!get_ip_from_domain_name(table_name, fresh_ip))
			{
				continue;
			}
			
			if (fresh_ip != old_ip)
			{
				// row enumeration in SQLite starts from 1
				db << "update NAME_IP set IP = ? where rowid = ?" << fresh_ip << (i + 1);
				DBG("\nRow: " << (i + 1) << ". " << table_name << ": " << old_ip << " ---> " << fresh_ip);
			}

		}
		catch (const std::exception & ex)
		{
			DBG(ex.what());
			continue;
		}
	}

	return true;
}