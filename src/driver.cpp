#pragma warning(disable : 5045)
#pragma warning(disable : 4820)
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>

#include <memdb.h>

using namespace memdb;

// Gobals
std::default_random_engine gen;
constexpr int NUM_ROWS = 1000000;

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
    std::uniform_int_distribution<size_t> distr1(5, size);
    std::uniform_int_distribution<int> distr2(97, 122);
    std::uniform_int_distribution<int> distr3(1000, 9999);

    //size = distr1(gen);

    std::string s;
    for (size_t i = 0; i < size; ++i)
    {
        s.append(std::string(1, (char)distr2(gen)));
    }
    //s += std::to_string(distr3(gen));

    return s;
}

void scenario1()
{
    const std::string query =
        "create table users "
        "({key, autoincrement} id: int32, "
        "{key} login: string[16], "
        "is_admin: bool = false, "
        "code: bytes[4])";

    std::cout << "Scenario #1... " << std::endl << std::endl;
    std::cout << "Executing query:" << std::endl;
    std::cout << "    " << query << std::endl << std::endl;

    try
    {
        Database db;
        ResultSet rs = db.execute(query);
        if (!rs.is_ok())
        {
            throw std::runtime_error(rs.get_error());
        }        

        std::cout << "Populating table with " << NUM_ROWS << " rows... ";
        std::cout.flush();
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        for (size_t i = 0; i < NUM_ROWS; ++i)
        {
            while (true)
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

                ResultSet rs2 = db.execute(query.str());
                if (rs2.is_ok())
                {
                    //std::cout << std::endl << login << "... ";
                    break;
                }
                else
                {
                    // continue if random login is not unique
                    //std::cout << std::endl << "Recreating non-unique login "  << login  << "... ";
                }
            }
        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << time_ms << " ms" << std::endl
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
    }
    catch (std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void scenario2()
{
    const std::string query =
        "create table users "
        "({key, autoincrement} id: int32, "
        "login: string[16], "
        "is_admin: bool = false, "
        "code: bytes[4])";

    std::cout << "Scenario #2... " << std::endl << std::endl;
    std::cout << "Executing query:" << std::endl;
    std::cout << "    " << query << std::endl << std::endl;
    
    try
    {
        Database db;
        ResultSet rs = db.execute(query);
        if (!rs.is_ok())
        {
            throw std::runtime_error(rs.get_error());
        }        

        std::cout << "Populating table with " << NUM_ROWS << " rows... ";
        std::cout.flush();
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        for (size_t i = 0; i < NUM_ROWS; ++i)
        {
            while (true)
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

                ResultSet rs2 = db.execute(query.str());
                if (rs2.is_ok())
                {
                    //std::cout << std::endl << login << "... ";
                    break;
                }
                else
                {
                    // continue if random login is not unique
                    //std::cout << std::endl << "Recreating non-unique login "  << login  << "... ";
                }
            }
        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << time_ms << " ms" << std::endl
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
    }
    catch (std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void scenario3()
{
    const std::string query =
        "create table users "
        "({key, autoincrement} id: int32, "
        "{unique} login: string[16], "
        "is_admin: bool = false, "
        "code: bytes[4])";

    std::cout << "Scenario #3... " << std::endl << std::endl;
    std::cout << "Executing query:" << std::endl;
    std::cout << "    " << query << std::endl << std::endl;

    try
    {
        Database db;
        ResultSet rs = db.execute(query);
        if (!rs.is_ok())
        {
            throw std::runtime_error(rs.get_error());
        }        

        std::cout << "Populating table with " << NUM_ROWS << " rows... ";
        std::cout.flush();
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        for (size_t i = 0; i < NUM_ROWS; ++i)
        {
            while (true)
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

                ResultSet rs2 = db.execute(query.str());
                if (rs2.is_ok())
                {
                    //std::cout << std::endl << login << "... ";
                    break;
                }
                else
                {
                    // continue if random login is not unique
                    //std::cout << std::endl << "Recreating non-unique login "  << login  << "... ";
                }
            }
        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << time_ms << " ms" << std::endl
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
    }
    catch (std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void scenario4()
{
    std::cout << "Scenario #4... " << std::endl << std::endl;
    try
    {
        Database db;        

        std::cout << "Loading from file... ";
        std::cout.flush();
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        std::ifstream in("db.bin", std::ios::binary);
        db.load_from_file(in);
        in.close();
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << time_ms << " ms" << std::endl
            << std::endl;

        db.info(std::cout);  
               

        std::cout << "Selecting all... ";
        std::cout.flush();
        ResultSet rs3 = db.select_all("users");
        std::cout << rs3.get_time() << " ms (" << rs3.get_row_count() << " rows)" << std::endl
            << std::endl;

        std::cout << "First 5 rows:" << std::endl;
        std::cout 
            << std::setw(8) << std::left << rs3.get_columns()[0]
            << std::setw(16) << std::left << rs3.get_columns()[1]
            << std::setw(9) << std::left << rs3.get_columns()[2]
            << rs3.get_columns()[3] << std::endl;

        int count = 0;
        for (const auto& row : rs3)
        {
            int id = row.get<int32_t>("id");
            std::string login = row.get<std::string>("login");
            bool is_admin = row.get<bool>("is_admin");
            std::string code = to_string(row.get<Bytes>("code"));
            std::cout << std::setw(8) << std::left << id
                << std::setw(16) << std::left << login
                << std::setw(9) << std::left << is_admin
                << code << std::endl;
            if (++count == 5)
                break;
        }
        
        std::string query = "select id, login from users where login >= \"a\" && login < \"b\" && code < 0xffffffff";

        std::cout << std::endl << "Selecting by conditions using query:" << std::endl;
        std::cout << "    " << query << std::endl << std::endl;

        std::cout << "Selecting without index... ";
        std::cout.flush();        
        ResultSet rs4 = db.execute(query);
        std::cout << rs4.get_time() << " ms (" << rs4.get_row_count() << " rows)" << std::endl
            << std::endl;       

        if (!rs4.is_ok())
        {
            throw std::runtime_error(rs4.get_error());
        }

        std::cout << "First 5 rows:" << std::endl;

        std::cout << std::setw(8) << std::left << rs4.get_columns()[0]
            << std::setw(16) << std::left << rs4.get_columns()[1]
            << std::endl;

        count = 0;
        for (const auto& row : rs4)
        {
            int id = row.get<int32_t>("id");
            std::string login = row.get<std::string>("login");
            std::cout << std::setw(8) << std::left << id
                << std::setw(16) << std::left << login
                << std::endl;
            if (++count == 5)
                break;
        }

        std::cout << std::endl << "Creating ordered index on the first column... ";
        std::cout.flush();
        ResultSet rs1 = db.execute("create ordered index on users by id");
        // ResultSet rs1 = db.create_ordered_index("users", { "id" });        
        std::cout << rs1.get_time() << " ms" << std::endl
            << std::endl;
        if (!rs1.is_ok())
        {
            std::cout << "Warning: " << rs1.get_error() << std::endl
                << std::endl;
        }

        std::cout << "Creating ordered index on the second column... ";
        std::cout.flush();
        ResultSet rs2 = db.execute("create ordered index on users by login");
        // ResultSet rs2 = db.create_ordered_index("users", { "login" });        
        std::cout << rs2.get_time() << " ms" << std::endl
            << std::endl;
        if (!rs2.is_ok())
        {
            std::cout << "Warning: " << rs2.get_error() << std::endl
                << std::endl;
        }

        std::cout << "Selecting using index... ";
        std::cout.flush();
        ResultSet rs5 = db.execute(query);
        std::cout << rs5.get_time() << " ms (" << rs5.get_row_count() << " rows)" << std::endl
            << std::endl;

        if (!rs5.is_ok())
        {
            throw std::runtime_error(rs4.get_error());
        }

        std::cout << "First 5 rows:" << std::endl;
        std::cout << std::setw(8) << std::left << rs5.get_columns()[0]
            << std::setw(16) << std::left << rs5.get_columns()[1]
            << std::endl;

        count = 0;
        for (const auto& row : rs5)
        {
            int id = row.get<int32_t>("id");
            std::string login = row.get<std::string>("login");            
            std::cout << std::setw(8) << std::left << id
                << std::setw(16) << std::left << login
                << std::endl;
            if (++count == 5)
                break;
        }
    }
    catch (std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

int main()
{
    scenario1();
    scenario2();
    //scenario3(); // too slow
    scenario4();

    return 0;
}