# Overview
This project is an implementation of the DNS (Domain Name System) protocol. Written using C++ Poco library and C++ SQLite wrapper.

The key goals of the project are:
- relatively readable code, using RAII
- safety, by errors and exceptions handling
- universal interface, by writing in functional style

Though the target platform is Linux, the project initially was written and tested in Visual Studio 2019, so expected few
errors when porting it on Windows.

# Requirements
- C++14 compiler
- PocoNet module ([Poco](https://pocoproject.org/) 1.12.4)
- [SQLite C++ Wrapper](https://github.com/SqliteModernCpp/sqlite_modern_cpp) 3.2
- sqlite3 3.45.0 C-library


# Building
Use `chmod +x *.sh` once at start, then `./build_debug.sh`, `./build_release.sh` or `./clean.sh` to build debug version, build release version or 
clean bin directory respectively.

# Usage
- Navigate the folder bin
- Type `sudo ./dns_server <host>:<port>`, where:
     * host - IP address of your machine
     * port - port, that you are going to use
- By default DNS uses 53 port so providing `sudo` is needed
- Correct example: `sudo ./dns_server 192.168.1.35:53`
- You can add 3d parameter `refresh` to refresh database on startup (eg. `sudo ./dns_server 192.168.1.35:53 refresh`)

# Special thanks for
- providing documentation - [fffaraz](https://github.com/fffaraz/Faraz-DNS-Server)
- detailed explanation about the DNS protocol - [howCode](https://www.youtube.com/watch?v=HdrPWGZ3NRo&list=PLBOh8f9FoHHhvO5e5HF_6mYvtZegobYX2)
- providing hexdump library for logging DNS packets - [zmb3](https://github.com/zmb3/hexdump/tree/master)



 