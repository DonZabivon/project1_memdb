#include <gtest/gtest.h>
#include "lexer.h"
using namespace memdb;

TEST(LexerTest, TestCreateTable)
{
	std::string s = "create table users ({key, autoincrement} id :\n"
					"int32, {unique} login: string[32], password_hash: bytes[8], is_admin:\n"
					"bool = false)";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 36);

	EXPECT_EQ(lexems[0].type, LexemType::CREATE);
	EXPECT_EQ(lexems[0].value, "create");

	EXPECT_EQ(lexems[1].type, LexemType::TABLE);
	EXPECT_EQ(lexems[1].value, "table");

	EXPECT_EQ(lexems[2].type, LexemType::ID);
	EXPECT_EQ(lexems[2].value, "users");

	EXPECT_EQ(lexems[3].type, LexemType::LPAR);
	EXPECT_EQ(lexems[3].value, "(");

	EXPECT_EQ(lexems[4].type, LexemType::LBRC);
	EXPECT_EQ(lexems[4].value, "{");

	EXPECT_EQ(lexems[5].type, LexemType::KEY);
	EXPECT_EQ(lexems[5].value, "key");

	EXPECT_EQ(lexems[6].type, LexemType::COMMA);
	EXPECT_EQ(lexems[6].value, ",");

	EXPECT_EQ(lexems[7].type, LexemType::AUTO);
	EXPECT_EQ(lexems[7].value, "autoincrement");

	EXPECT_EQ(lexems[8].type, LexemType::RBRC);
	EXPECT_EQ(lexems[8].value, "}");

	EXPECT_EQ(lexems[9].type, LexemType::ID);
	EXPECT_EQ(lexems[9].value, "id");

	EXPECT_EQ(lexems[10].type, LexemType::COLON);
	EXPECT_EQ(lexems[10].value, ":");

	EXPECT_EQ(lexems[11].type, LexemType::INT32);
	EXPECT_EQ(lexems[11].value, "int32");

	EXPECT_EQ(lexems[12].type, LexemType::COMMA);
	EXPECT_EQ(lexems[12].value, ",");

	EXPECT_EQ(lexems[13].type, LexemType::LBRC);
	EXPECT_EQ(lexems[13].value, "{");

	EXPECT_EQ(lexems[14].type, LexemType::UNIQUE);
	EXPECT_EQ(lexems[14].value, "unique");

	EXPECT_EQ(lexems[15].type, LexemType::RBRC);
	EXPECT_EQ(lexems[15].value, "}");

	EXPECT_EQ(lexems[16].type, LexemType::ID);
	EXPECT_EQ(lexems[16].value, "login");

	EXPECT_EQ(lexems[17].type, LexemType::COLON);
	EXPECT_EQ(lexems[17].value, ":");

	EXPECT_EQ(lexems[18].type, LexemType::STRING);
	EXPECT_EQ(lexems[18].value, "string");

	EXPECT_EQ(lexems[19].type, LexemType::LBRK);
	EXPECT_EQ(lexems[19].value, "[");

	EXPECT_EQ(lexems[20].type, LexemType::INT_LIT);
	EXPECT_EQ(lexems[20].value, "32");

	EXPECT_EQ(lexems[21].type, LexemType::RBRK);
	EXPECT_EQ(lexems[21].value, "]");

	EXPECT_EQ(lexems[22].type, LexemType::COMMA);
	EXPECT_EQ(lexems[22].value, ",");

	EXPECT_EQ(lexems[23].type, LexemType::ID);
	EXPECT_EQ(lexems[23].value, "password_hash");

	EXPECT_EQ(lexems[24].type, LexemType::COLON);
	EXPECT_EQ(lexems[24].value, ":");

	EXPECT_EQ(lexems[25].type, LexemType::BYTES);
	EXPECT_EQ(lexems[25].value, "bytes");

	EXPECT_EQ(lexems[26].type, LexemType::LBRK);
	EXPECT_EQ(lexems[26].value, "[");

	EXPECT_EQ(lexems[27].type, LexemType::INT_LIT);
	EXPECT_EQ(lexems[27].value, "8");

	EXPECT_EQ(lexems[28].type, LexemType::RBRK);
	EXPECT_EQ(lexems[28].value, "]");

	EXPECT_EQ(lexems[29].type, LexemType::COMMA);
	EXPECT_EQ(lexems[29].value, ",");

	EXPECT_EQ(lexems[30].type, LexemType::ID);
	EXPECT_EQ(lexems[30].value, "is_admin");

	EXPECT_EQ(lexems[31].type, LexemType::COLON);
	EXPECT_EQ(lexems[31].value, ":");

	EXPECT_EQ(lexems[32].type, LexemType::BOOL);
	EXPECT_EQ(lexems[32].value, "bool");

	EXPECT_EQ(lexems[33].type, LexemType::EQ);
	EXPECT_EQ(lexems[33].value, "=");

	EXPECT_EQ(lexems[34].type, LexemType::BOOL_LIT);
	EXPECT_EQ(lexems[34].value, "false");

	EXPECT_EQ(lexems[35].type, LexemType::RPAR);
	EXPECT_EQ(lexems[35].value, ")");
}

TEST(LexerTest, TestInsert)
{
	std::string s = "insert (login = \"vas\\\"ya\\\"\", password_hash = 0xdeadbeefdeadbeef) to users";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 12);

	EXPECT_EQ(lexems[0].type, LexemType::INSERT);
	EXPECT_EQ(lexems[0].value, "insert");

	EXPECT_EQ(lexems[1].type, LexemType::LPAR);
	EXPECT_EQ(lexems[1].value, "(");

	EXPECT_EQ(lexems[2].type, LexemType::ID);
	EXPECT_EQ(lexems[2].value, "login");

	EXPECT_EQ(lexems[3].type, LexemType::EQ);
	EXPECT_EQ(lexems[3].value, "=");

	EXPECT_EQ(lexems[4].type, LexemType::STR_LIT);
	EXPECT_EQ(lexems[4].value, "vas\\\"ya\\\"");

	EXPECT_EQ(lexems[5].type, LexemType::COMMA);
	EXPECT_EQ(lexems[5].value, ",");

	EXPECT_EQ(lexems[6].type, LexemType::ID);
	EXPECT_EQ(lexems[6].value, "password_hash");

	EXPECT_EQ(lexems[7].type, LexemType::EQ);
	EXPECT_EQ(lexems[7].value, "=");

	EXPECT_EQ(lexems[8].type, LexemType::BT_LIT);
	EXPECT_EQ(lexems[8].value, "0xdeadbeefdeadbeef");

	EXPECT_EQ(lexems[9].type, LexemType::RPAR);
	EXPECT_EQ(lexems[9].value, ")");

	EXPECT_EQ(lexems[10].type, LexemType::TO);
	EXPECT_EQ(lexems[10].value, "to");

	EXPECT_EQ(lexems[11].type, LexemType::ID);
	EXPECT_EQ(lexems[11].value, "users");
}

TEST(LexerTest, TestSelect)
{
	std::string s = "select id, login from users where is_admin || id <= 10";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 12);

	EXPECT_EQ(lexems[0].type, LexemType::SELECT);
	EXPECT_EQ(lexems[0].value, "select");

	EXPECT_EQ(lexems[1].type, LexemType::ID);
	EXPECT_EQ(lexems[1].value, "id");

	EXPECT_EQ(lexems[2].type, LexemType::COMMA);
	EXPECT_EQ(lexems[2].value, ",");

	EXPECT_EQ(lexems[3].type, LexemType::ID);
	EXPECT_EQ(lexems[3].value, "login");

	EXPECT_EQ(lexems[4].type, LexemType::FROM);
	EXPECT_EQ(lexems[4].value, "from");

	EXPECT_EQ(lexems[5].type, LexemType::ID);
	EXPECT_EQ(lexems[5].value, "users");

	EXPECT_EQ(lexems[6].type, LexemType::WHERE);
	EXPECT_EQ(lexems[6].value, "where");

	EXPECT_EQ(lexems[7].type, LexemType::ID);
	EXPECT_EQ(lexems[7].value, "is_admin");

	EXPECT_EQ(lexems[8].type, LexemType::OR);
	EXPECT_EQ(lexems[8].value, "||");

	EXPECT_EQ(lexems[9].type, LexemType::ID);
	EXPECT_EQ(lexems[9].value, "id");

	EXPECT_EQ(lexems[10].type, LexemType::LE);
	EXPECT_EQ(lexems[10].value, "<=");

	EXPECT_EQ(lexems[11].type, LexemType::INT_LIT);
	EXPECT_EQ(lexems[11].value, "10");
}

TEST(LexerTest, TestUpdate)
{
	std::string s = "update users set login = login + \"_deleted\", is_admin = true where\n"
					"password_hash >= 0x00000000ffffffff";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 16);

	EXPECT_EQ(lexems[0].type, LexemType::UPDATE);
	EXPECT_EQ(lexems[0].value, "update");

	EXPECT_EQ(lexems[1].type, LexemType::ID);
	EXPECT_EQ(lexems[1].value, "users");

	EXPECT_EQ(lexems[2].type, LexemType::SET);
	EXPECT_EQ(lexems[2].value, "set");

	EXPECT_EQ(lexems[3].type, LexemType::ID);
	EXPECT_EQ(lexems[3].value, "login");

	EXPECT_EQ(lexems[4].type, LexemType::EQ);
	EXPECT_EQ(lexems[4].value, "=");

	EXPECT_EQ(lexems[5].type, LexemType::ID);
	EXPECT_EQ(lexems[5].value, "login");

	EXPECT_EQ(lexems[6].type, LexemType::PLUS);
	EXPECT_EQ(lexems[6].value, "+");

	EXPECT_EQ(lexems[7].type, LexemType::STR_LIT);
	EXPECT_EQ(lexems[7].value, "_deleted");

	EXPECT_EQ(lexems[8].type, LexemType::COMMA);
	EXPECT_EQ(lexems[8].value, ",");

	EXPECT_EQ(lexems[9].type, LexemType::ID);
	EXPECT_EQ(lexems[9].value, "is_admin");

	EXPECT_EQ(lexems[10].type, LexemType::EQ);
	EXPECT_EQ(lexems[10].value, "=");

	EXPECT_EQ(lexems[11].type, LexemType::BOOL_LIT);
	EXPECT_EQ(lexems[11].value, "true");

	EXPECT_EQ(lexems[12].type, LexemType::WHERE);
	EXPECT_EQ(lexems[12].value, "where");

	EXPECT_EQ(lexems[13].type, LexemType::ID);
	EXPECT_EQ(lexems[13].value, "password_hash");

	EXPECT_EQ(lexems[14].type, LexemType::GE);
	EXPECT_EQ(lexems[14].value, ">=");

	EXPECT_EQ(lexems[15].type, LexemType::BT_LIT);
	EXPECT_EQ(lexems[15].value, "0x00000000ffffffff");
}

TEST(LexerTest, TestDelete)
{
	std::string s = "delete users where |login| % 2 = 0";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 10);

	EXPECT_EQ(lexems[0].type, LexemType::DELETE);
	EXPECT_EQ(lexems[0].value, "delete");

	EXPECT_EQ(lexems[1].type, LexemType::ID);
	EXPECT_EQ(lexems[1].value, "users");

	EXPECT_EQ(lexems[2].type, LexemType::WHERE);
	EXPECT_EQ(lexems[2].value, "where");

	EXPECT_EQ(lexems[3].type, LexemType::PIPE);
	EXPECT_EQ(lexems[3].value, "|");

	EXPECT_EQ(lexems[4].type, LexemType::ID);
	EXPECT_EQ(lexems[4].value, "login");

	EXPECT_EQ(lexems[5].type, LexemType::PIPE);
	EXPECT_EQ(lexems[5].value, "|");

	EXPECT_EQ(lexems[6].type, LexemType::MOD);
	EXPECT_EQ(lexems[6].value, "%");

	EXPECT_EQ(lexems[7].type, LexemType::INT_LIT);
	EXPECT_EQ(lexems[7].value, "2");

	EXPECT_EQ(lexems[8].type, LexemType::EQ);
	EXPECT_EQ(lexems[8].value, "=");

	EXPECT_EQ(lexems[9].type, LexemType::INT_LIT);
	EXPECT_EQ(lexems[9].value, "0");
}

TEST(LexerTest, TestJoin)
{
	std::string s = "select posts.id, users.login, posts.text from users join\n"
					"posts on users.id = posts.user_id where true";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 26);

	EXPECT_EQ(lexems[0].type, LexemType::SELECT);
	EXPECT_EQ(lexems[0].value, "select");

	EXPECT_EQ(lexems[1].type, LexemType::ID);
	EXPECT_EQ(lexems[1].value, "posts");

	EXPECT_EQ(lexems[2].type, LexemType::DOT);
	EXPECT_EQ(lexems[2].value, ".");

	EXPECT_EQ(lexems[3].type, LexemType::ID);
	EXPECT_EQ(lexems[3].value, "id");

	EXPECT_EQ(lexems[4].type, LexemType::COMMA);
	EXPECT_EQ(lexems[4].value, ",");

	EXPECT_EQ(lexems[5].type, LexemType::ID);
	EXPECT_EQ(lexems[5].value, "users");

	EXPECT_EQ(lexems[6].type, LexemType::DOT);
	EXPECT_EQ(lexems[6].value, ".");

	EXPECT_EQ(lexems[7].type, LexemType::ID);
	EXPECT_EQ(lexems[7].value, "login");

	EXPECT_EQ(lexems[8].type, LexemType::COMMA);
	EXPECT_EQ(lexems[8].value, ",");

	EXPECT_EQ(lexems[9].type, LexemType::ID);
	EXPECT_EQ(lexems[9].value, "posts");

	EXPECT_EQ(lexems[10].type, LexemType::DOT);
	EXPECT_EQ(lexems[10].value, ".");

	EXPECT_EQ(lexems[11].type, LexemType::ID);
	EXPECT_EQ(lexems[11].value, "text");

	EXPECT_EQ(lexems[12].type, LexemType::FROM);
	EXPECT_EQ(lexems[12].value, "from");

	EXPECT_EQ(lexems[13].type, LexemType::ID);
	EXPECT_EQ(lexems[13].value, "users");

	EXPECT_EQ(lexems[14].type, LexemType::JOIN);
	EXPECT_EQ(lexems[14].value, "join");

	EXPECT_EQ(lexems[15].type, LexemType::ID);
	EXPECT_EQ(lexems[15].value, "posts");

	EXPECT_EQ(lexems[16].type, LexemType::ON);
	EXPECT_EQ(lexems[16].value, "on");

	EXPECT_EQ(lexems[17].type, LexemType::ID);
	EXPECT_EQ(lexems[17].value, "users");

	EXPECT_EQ(lexems[18].type, LexemType::DOT);
	EXPECT_EQ(lexems[18].value, ".");

	EXPECT_EQ(lexems[19].type, LexemType::ID);
	EXPECT_EQ(lexems[19].value, "id");

	EXPECT_EQ(lexems[20].type, LexemType::EQ);
	EXPECT_EQ(lexems[20].value, "=");

	EXPECT_EQ(lexems[21].type, LexemType::ID);
	EXPECT_EQ(lexems[21].value, "posts");

	EXPECT_EQ(lexems[22].type, LexemType::DOT);
	EXPECT_EQ(lexems[22].value, ".");

	EXPECT_EQ(lexems[23].type, LexemType::ID);
	EXPECT_EQ(lexems[23].value, "user_id");

	EXPECT_EQ(lexems[24].type, LexemType::WHERE);
	EXPECT_EQ(lexems[24].value, "where");

	EXPECT_EQ(lexems[25].type, LexemType::BOOL_LIT);
	EXPECT_EQ(lexems[25].value, "true");
}

TEST(LexerTest, TestOrderedIndex)
{
	std::string s = "create ordered index on users by login";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 7);

	EXPECT_EQ(lexems[0].type, LexemType::CREATE);
	EXPECT_EQ(lexems[0].value, "create");

	EXPECT_EQ(lexems[1].type, LexemType::ORDERED);
	EXPECT_EQ(lexems[1].value, "ordered");

	EXPECT_EQ(lexems[2].type, LexemType::INDEX);
	EXPECT_EQ(lexems[2].value, "index");

	EXPECT_EQ(lexems[3].type, LexemType::ON);
	EXPECT_EQ(lexems[3].value, "on");

	EXPECT_EQ(lexems[4].type, LexemType::ID);
	EXPECT_EQ(lexems[4].value, "users");

	EXPECT_EQ(lexems[5].type, LexemType::BY);
	EXPECT_EQ(lexems[5].value, "by");

	EXPECT_EQ(lexems[6].type, LexemType::ID);
	EXPECT_EQ(lexems[6].value, "login");
}

TEST(LexerTest, TestUnorderedIndex)
{
	std::string s = "Create Unordered Index On users By is_admin";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 7);

	EXPECT_EQ(lexems[0].type, LexemType::CREATE);
	EXPECT_EQ(lexems[0].value, "Create");

	EXPECT_EQ(lexems[1].type, LexemType::UNORDERED);
	EXPECT_EQ(lexems[1].value, "Unordered");

	EXPECT_EQ(lexems[2].type, LexemType::INDEX);
	EXPECT_EQ(lexems[2].value, "Index");

	EXPECT_EQ(lexems[3].type, LexemType::ON);
	EXPECT_EQ(lexems[3].value, "On");

	EXPECT_EQ(lexems[4].type, LexemType::ID);
	EXPECT_EQ(lexems[4].value, "users");

	EXPECT_EQ(lexems[5].type, LexemType::BY);
	EXPECT_EQ(lexems[5].value, "By");

	EXPECT_EQ(lexems[6].type, LexemType::ID);
	EXPECT_EQ(lexems[6].value, "is_admin");
}

TEST(LexerTest, TestAll)
{
	std::string s = "create table unique autoincrement key insert\n"
					"to select from where set update delete join index on by\n"
					"ordered unordered int32 bool string bytes\n"
					"+-*/%=!=<><=>=&&!||^^|(){}[].,:\n"
					"123456789 true false \"s\\tt\\tr\" 0x123456";
	Lexer lexer(s);
	const auto &lexems = lexer.tokenize();

	EXPECT_EQ(lexems.size(), 53);

	EXPECT_EQ(lexems[0].type, LexemType::CREATE);
	EXPECT_EQ(lexems[0].value, "create");

	EXPECT_EQ(lexems[1].type, LexemType::TABLE);
	EXPECT_EQ(lexems[1].value, "table");

	EXPECT_EQ(lexems[2].type, LexemType::UNIQUE);
	EXPECT_EQ(lexems[2].value, "unique");

	EXPECT_EQ(lexems[3].type, LexemType::AUTO);
	EXPECT_EQ(lexems[3].value, "autoincrement");

	EXPECT_EQ(lexems[4].type, LexemType::KEY);
	EXPECT_EQ(lexems[4].value, "key");

	EXPECT_EQ(lexems[5].type, LexemType::INSERT);
	EXPECT_EQ(lexems[5].value, "insert");

	EXPECT_EQ(lexems[6].type, LexemType::TO);
	EXPECT_EQ(lexems[6].value, "to");

	EXPECT_EQ(lexems[7].type, LexemType::SELECT);
	EXPECT_EQ(lexems[7].value, "select");

	EXPECT_EQ(lexems[8].type, LexemType::FROM);
	EXPECT_EQ(lexems[8].value, "from");

	EXPECT_EQ(lexems[9].type, LexemType::WHERE);
	EXPECT_EQ(lexems[9].value, "where");

	EXPECT_EQ(lexems[10].type, LexemType::SET);
	EXPECT_EQ(lexems[10].value, "set");

	EXPECT_EQ(lexems[11].type, LexemType::UPDATE);
	EXPECT_EQ(lexems[11].value, "update");

	EXPECT_EQ(lexems[12].type, LexemType::DELETE);
	EXPECT_EQ(lexems[12].value, "delete");

	EXPECT_EQ(lexems[13].type, LexemType::JOIN);
	EXPECT_EQ(lexems[13].value, "join");

	EXPECT_EQ(lexems[14].type, LexemType::INDEX);
	EXPECT_EQ(lexems[14].value, "index");

	EXPECT_EQ(lexems[15].type, LexemType::ON);
	EXPECT_EQ(lexems[15].value, "on");

	EXPECT_EQ(lexems[16].type, LexemType::BY);
	EXPECT_EQ(lexems[16].value, "by");

	EXPECT_EQ(lexems[17].type, LexemType::ORDERED);
	EXPECT_EQ(lexems[17].value, "ordered");

	EXPECT_EQ(lexems[18].type, LexemType::UNORDERED);
	EXPECT_EQ(lexems[18].value, "unordered");

	EXPECT_EQ(lexems[19].type, LexemType::INT32);
	EXPECT_EQ(lexems[19].value, "int32");

	EXPECT_EQ(lexems[20].type, LexemType::BOOL);
	EXPECT_EQ(lexems[20].value, "bool");

	EXPECT_EQ(lexems[21].type, LexemType::STRING);
	EXPECT_EQ(lexems[21].value, "string");

	EXPECT_EQ(lexems[22].type, LexemType::BYTES);
	EXPECT_EQ(lexems[22].value, "bytes");

	EXPECT_EQ(lexems[23].type, LexemType::PLUS);
	EXPECT_EQ(lexems[23].value, "+");

	EXPECT_EQ(lexems[24].type, LexemType::MINUS);
	EXPECT_EQ(lexems[24].value, "-");

	EXPECT_EQ(lexems[25].type, LexemType::MULT);
	EXPECT_EQ(lexems[25].value, "*");

	EXPECT_EQ(lexems[26].type, LexemType::DIV);
	EXPECT_EQ(lexems[26].value, "/");

	EXPECT_EQ(lexems[27].type, LexemType::MOD);
	EXPECT_EQ(lexems[27].value, "%");

	EXPECT_EQ(lexems[28].type, LexemType::EQ);
	EXPECT_EQ(lexems[28].value, "=");

	EXPECT_EQ(lexems[29].type, LexemType::NE);
	EXPECT_EQ(lexems[29].value, "!=");

	EXPECT_EQ(lexems[30].type, LexemType::LT);
	EXPECT_EQ(lexems[30].value, "<");

	EXPECT_EQ(lexems[31].type, LexemType::GT);
	EXPECT_EQ(lexems[31].value, ">");

	EXPECT_EQ(lexems[32].type, LexemType::LE);
	EXPECT_EQ(lexems[32].value, "<=");

	EXPECT_EQ(lexems[33].type, LexemType::GE);
	EXPECT_EQ(lexems[33].value, ">=");

	EXPECT_EQ(lexems[34].type, LexemType::AND);
	EXPECT_EQ(lexems[34].value, "&&");

	EXPECT_EQ(lexems[35].type, LexemType::NOT);
	EXPECT_EQ(lexems[35].value, "!");

	EXPECT_EQ(lexems[36].type, LexemType::OR);
	EXPECT_EQ(lexems[36].value, "||");

	EXPECT_EQ(lexems[37].type, LexemType::XOR);
	EXPECT_EQ(lexems[37].value, "^^");

	EXPECT_EQ(lexems[38].type, LexemType::PIPE);
	EXPECT_EQ(lexems[38].value, "|");

	EXPECT_EQ(lexems[39].type, LexemType::LPAR);
	EXPECT_EQ(lexems[39].value, "(");

	EXPECT_EQ(lexems[40].type, LexemType::RPAR);
	EXPECT_EQ(lexems[40].value, ")");

	EXPECT_EQ(lexems[41].type, LexemType::LBRC);
	EXPECT_EQ(lexems[41].value, "{");

	EXPECT_EQ(lexems[42].type, LexemType::RBRC);
	EXPECT_EQ(lexems[42].value, "}");

	EXPECT_EQ(lexems[43].type, LexemType::LBRK);
	EXPECT_EQ(lexems[43].value, "[");

	EXPECT_EQ(lexems[44].type, LexemType::RBRK);
	EXPECT_EQ(lexems[44].value, "]");

	EXPECT_EQ(lexems[45].type, LexemType::DOT);
	EXPECT_EQ(lexems[45].value, ".");

	EXPECT_EQ(lexems[46].type, LexemType::COMMA);
	EXPECT_EQ(lexems[46].value, ",");

	EXPECT_EQ(lexems[47].type, LexemType::COLON);
	EXPECT_EQ(lexems[47].value, ":");

	EXPECT_EQ(lexems[48].type, LexemType::INT_LIT);
	EXPECT_EQ(lexems[48].value, "123456789");

	EXPECT_EQ(lexems[49].type, LexemType::BOOL_LIT);
	EXPECT_EQ(lexems[49].value, "true");

	EXPECT_EQ(lexems[50].type, LexemType::BOOL_LIT);
	EXPECT_EQ(lexems[50].value, "false");

	EXPECT_EQ(lexems[51].type, LexemType::STR_LIT);
	EXPECT_EQ(lexems[51].value, "s\\tt\\tr");

	EXPECT_EQ(lexems[52].type, LexemType::BT_LIT);
	EXPECT_EQ(lexems[52].value, "0x123456");
}