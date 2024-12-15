#include <iostream>
#include <string>
#include <fstream>

#include <memdb.h>


using namespace memdb;

int main()
{
	Database db;

	std::string query1 = "create table users ({autoincrement} id :\n"
		"int32, login: string[32])";	


	db.execute(query1);

	for (int i = 0; i < 10; ++i)
	{
		std::string login = "user_" + std::to_string(i + 1);
		std::string query2 = "insert (login = \"" + login + "\") to users";
		db.execute(query2);
	}
	
	std::string query3 = "select id, login from users where true";
	auto result = db.execute(query3);
	if (result.is_ok()) {
		for (auto& row : result) {
			int id = row.get<int>("id");
			std::string login = row.get<std::string>("login");
			std::cout << id << "\t" << login << std::endl;
		}
	}
	else {
		std::cerr << "Error: " << result.get_error() << "\n";
	}
	
	db.save_to_file(std::ofstream("small_db.bin", std::ios::binary));

	return 0;
}