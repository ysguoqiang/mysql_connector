#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <vector>
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

class DataBuf : public std::streambuf
{
public:
    DataBuf(char* d, size_t s)
    {
        setg(d, d, d+s);
    }
};

int main(int argc, const char **argv)
{
	cout << "Mysql Connector/C++ Test" << endl;

	try {
		sql::mysql::MySQL_Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		driver = sql::mysql::get_mysql_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "123456");
		stmt = con->createStatement();
		stmt->execute("USE test");
		stmt->execute("DROP TABLE IF EXISTS test_a");
		stmt->execute("CREATE TABLE test_a(id INT primary key, label CHAR(1))");
		stmt->execute("INSERT INTO test_a(id, label) VALUES (1, 'a')");

		stmt->execute("DROP TABLE IF EXISTS test_b");
		stmt->execute("CREATE TABLE test_b(id INT primary key, b BLOB)");

		//写入blob字段
		sql::PreparedStatement* pstmt;
		pstmt = con->prepareStatement("INSERT INTO test_b VALUES(?, ?)");

		int a = 1;
		char buf[64] = "I am a good boy!";
		//memcpy(buf,&a,sizeof(a));
		//memcpy(buf + sizeof(a),&a,sizeof(a));
		//memcpy(buf + 2 * sizeof(a),&a,sizeof(a));
		DataBuf buffer(buf, strlen(buf));

		std::istream s(&buffer);
		pstmt->setInt(1, a);
		pstmt->setBlob(2, &s);
		pstmt->execute();
		delete pstmt;

		//读取blob字段
		sql::ResultSet* result = stmt->executeQuery("select * from test_b");
		if (result) {
			while (result->next()) {
				//方法一：使用getString
				//vector<char> vec;
				//string str = result->getString("b");
				//vec.insert(vec.end(), str.begin(), str.end());

				//方法二：使用getBlob
				istream *pis= result->getBlob("b");
				pis->seekg(0, ios::end);
				int sz = pis->tellg();
				pis->seekg(ios::beg);
				vector<char> vec;
				vec.resize(sz);
				pis->read(&vec[0], sz);

				cout << "print blob:";
				for (vector<char>::iterator iter = vec.begin(); iter != vec.end(); ++iter) {
					cout << (*iter);
				}
				cout << endl;
			}
		}

		delete stmt;
		delete con;
	} catch (sql::SQLException &e) {
		/*
		The MySQL Connector/C++ throws three different exceptions:
		1.sql::MethodNotImplementedException (derived from sql::SQLException)
		2.sql::InvalidArgumentException (derived from sql::SQLException)
		3.sql::SQLException (derived from std::runtime_error)
		*/
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << "__FUNCTION__" << ") on line " << __LINE__ << endl;
		/* Use what() (derived from std::runtime_error) to fetch the error message */
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return -1;
	}

	cout << "Done" << endl;
	return 0;
}
