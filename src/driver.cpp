#pragma warning(disable : 5045)
#pragma warning(disable : 4820)
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>

#include <memdb.h>

using namespace memdb;

std::default_random_engine gen;

Bytes random_bytes(size_t size)
{
    std::uniform_int_distribution<int> distr(0, UINT8_MAX);

    Bytes bytes;
    for (size_t i = 0; i < size; ++i)
    {
        bytes.push_back((uint8_t)distr(gen));
    }
    return bytes;
}

std::string random_string(size_t size)
{
    std::uniform_int_distribution<size_t> distr1(1, size);
    std::uniform_int_distribution<int> distr2(97, 122);

    size = distr1(gen);

    std::string s;
    for (size_t i = 0; i < size; ++i)
    {
        s.append(std::string(1, (char)distr2(gen)));
    }
    return s;
}

int main()
{
    try
    {
        Database db;
        ResultSet rs = db.execute("create table users ({autoincrement} id: int32, login: string[16], is_admin: bool = false, code: bytes[4])");
        if (!rs.is_ok())
        {
            throw std::runtime_error(rs.get_error());
        }
        // Эквивалентно вызову метода create_table(...):
        /*std::vector<Column> columns =
        {
            Column(Type::INT, "id"),
            Column(Type::STRING, "login", 16),
            Column(Type::BOOL, "is_admin"),
            Column(Type::BYTES, "code", 4)
        };
        columns[0].is_auto = true;
        db.create_table("users", columns);
        );*/

        std::cout << "Populating table... ";
        std::cout.flush();
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        for (size_t i = 0; i < 1000000; ++i)
        {
            std::string login = random_string(16 - 1);
            Bytes code = random_bytes(4);

            std::ostringstream query;
            query << "insert (";
            if (i % 2)
            {
                // use form 1
                query << ","; // autoincrement
                query << "\"" << login << "\",";
                if (i % 3)
                    query << ","; // to use default
                else
                    query << "true" << ",";
                query << to_string(code) << ")";
                query << " to users";
            }
            else
            {
                // use form 2
                query << "login=\"" << login << "\",";
                if (i % 3)
                    query << "is_admin = true" << ",";
                query << "code=" << to_string(code) << ")";
                query << " to users";
            }

            ResultSet rs = db.execute(query.str());
            if (!rs.is_ok())
            {
                std::ostringstream oss;
                oss << std::endl;
                oss << oss.str() << std::endl;
                oss << rs.get_error();
                throw std::runtime_error(oss.str());
            }

            // Эквивалентно вызову метода insert(...),
            // который будет работать гораздо быстрей в
            // данном случае, так как не требует разбора
            // текстового запроса:
            /*db.insert("users",
                {
                    Value(),
                    Value(login),
                    i % 3 ? Value() : Value(true),
                    Value(code)
                }
            );*/
        }
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        int64_t time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << time_ms << " ms" << std::endl
                  << std::endl;

        std::cout << "Creating ordered index on the first column... ";
        std::cout.flush();
        ResultSet rs1 = db.execute("create ordered index on users by id");
        // ResultSet rs1 = db.create_ordered_index("users", { "id" });
        if (!rs1.is_ok())
        {
            throw std::runtime_error(rs1.get_error());
        }
        std::cout << rs1.get_time() << " ms" << std::endl
                  << std::endl;

        std::cout << "Creating ordered index on the second column... ";
        std::cout.flush();
        ResultSet rs2 = db.execute("create ordered index on users by login");
        // ResultSet rs2 = db.create_ordered_index("users", { "login" });
        if (!rs2.is_ok())
        {
            throw std::runtime_error(rs2.get_error());
        }
        std::cout << rs2.get_time() << " ms" << std::endl
                  << std::endl;

        std::cout << "Saving to file... ";
        std::cout.flush();
        t1 = std::chrono::steady_clock::now();
        std::ofstream out("db.bin", std::ios::binary);
        db.save_to_file(out);
        out.close();
        t2 = std::chrono::steady_clock::now();
        time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << time_ms << " ms" << std::endl
                  << std::endl;

        std::cout << "Loading from file... ";
        std::cout.flush();
        t1 = std::chrono::steady_clock::now();
        std::ifstream in("db.bin", std::ios::binary);
        db.load_from_file(in);
        in.close();
        t2 = std::chrono::steady_clock::now();
        time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << time_ms << " ms" << std::endl
                  << std::endl;

        db.info(std::cout);

        std::cout << "Selecting all... ";
        std::cout.flush();
        ResultSet rs3 = db.select_all("users");
        std::cout << rs3.get_time() << " ms (" << rs3.get_row_count() << " rows)" << std::endl
                  << std::endl;

        Condition cond1(Value(10000), CondOp::GT);
        Condition cond2(Value(20000), CondOp::LT);
        Condition cond3(Value(true), CondOp::EQ);
        Condition cond4(Value("abc"), CondOp::GE);
        Condition cond5(Value("def"), CondOp::LE);
        Condition cond6(Value(bytes_from_hex_string("0x12345678")), CondOp::GE);
        Condition cond7(Value(bytes_from_hex_string("0x87654321")), CondOp::LE);

        std::cout << "Selecting by conditions... ";
        std::cout.flush();
        ResultSet rs4 = db.select("users", {{cond1, 0}, {cond2, 0}, {cond3, 2}, {cond4, 1}, {cond5, 1}, {cond6, 3}, {cond7, 3}});
        std::cout << rs4.get_time() << " ms (" << rs4.get_row_count() << " rows)" << std::endl
                  << std::endl;

        std::cout << std::setw(8) << std::left << rs4.get_columns()[0]
                  << std::setw(16) << std::left << rs4.get_columns()[1]
                  << std::setw(9) << std::left << rs4.get_columns()[2]
                  << rs4.get_columns()[3] << std::endl;

        for (const auto &row : rs4)
        {
            int id = row.get<int32_t>("id");
            std::string login = row.get<std::string>("login");
            bool is_admin = row.get<bool>("is_admin");
            std::string code = to_string(row.get<Bytes>("code"));
            std::cout << std::setw(8) << std::left << id
                      << std::setw(16) << std::left << login
                      << std::setw(9) << std::left << is_admin
                      << code << std::endl;
        }
    }
    catch (std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}