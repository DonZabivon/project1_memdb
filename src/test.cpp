#pragma warning(disable :5045)
#pragma warning(disable :4820)
#include <iostream>
#include <random>
#include <chrono>

#include <memdb.h>
#include <lexer.h>
#include <parser.h>

using namespace memdb;

int main()
{
	std::string s = "create table users ({key, autoincrement} id :\n"
		"int32, {unique} login: string[32], password_hash: bytes[8], is_admin:\n"
		"bool = false)";
	Lexer lexer(s);
	const auto& lexems = lexer.tokenize();

	CreateTableParser parser(lexems);

	CreateTableDef def = parser.parse();



	return 0;
}