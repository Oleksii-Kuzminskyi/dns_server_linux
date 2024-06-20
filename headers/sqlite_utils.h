#ifndef SQLITE_UTILS_H_
#define SQLITE_UTILS_H_

#include <string>
#include "sqlite_modern_cpp.h"


/*********************************************************************
 * @brief These functions are exception-free SQL-query wrappers.
 *		  They work with the following table structure:
 *		  
 *								TABLE: NAME_IP
 *	 ---------------------------------------------------------------------
 *	|		NAME (TEXT, PRIMRAY KEY, NOT NULL) |	IP (TEXT, NOT NULL)	  |
 *	 ---------------------------------------------------------------------
 * 
 *********************************************************************/

extern bool create_table(sqlite::database& db);
extern bool name_present_in_db(sqlite::database& db, const std::string& name);
extern bool get_ip_by_name_in_db(sqlite::database& db, const std::string& name, std::string& ip);
extern bool add_record_in_db(sqlite::database& db, const std::string& name, const std::string& ip);
extern bool refresh_db(sqlite::database& db);



#endif
