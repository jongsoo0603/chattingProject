#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <string>
// 채팅 프로그램 - 메인.

using namespace std;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "0000"; // 데이터베이스 접속 비밀번호

void join_membership(string id, string pw, string name, string phone)
{
    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
    sql::Connection* con;
    sql::Statement* stmt;
    sql::PreparedStatement* pstmt;

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    // 데이터베이스 선택
    con->setSchema("chattingproject");

    // db 한글 저장을 위한 셋팅 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // 데이터베이스 쿼리 실행
    // INSERT
    pstmt = con->prepareStatement("INSERT INTO member(memberID, passWord, name, phoneNumber) VALUES(?,?,?,?)");

    pstmt->setString(1, id);
    pstmt->setString(2, pw);
    pstmt->setString(3, name);
    pstmt->setString(4, phone);
    pstmt->execute();
    cout << "One row inserted." << endl;

    // MySQL Connector/C++ 정리
    delete pstmt;
    delete con;
}

string makeAllID()
{
    string allID;

    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
    sql::Connection* con;
    sql::Statement* stmt;
    sql::ResultSet* res;

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    // 데이터베이스 선택
    con->setSchema("chattingproject");

    stmt = con->createStatement();
    res = stmt->executeQuery("SELECT * FROM member");

    // ID 다 가져다 붙이기
    while (res->next()) 
    {
        allID += res->getString("memberID") + " ";
    }

    // MySQL Connector/C++ 정리
    delete res;
    delete stmt;
    delete con;

    return allID;
}


int main()
{  
    int select, alphaCheck = 0, repeatedCheck = 0;
    string id, name, phone, pw, allID = makeAllID();
    bool isalphaYN = false, pwCheck = false, phoneCheck = false;

    while(true)
    {
        cout << "1. 로그인 2. 회원가입 : ";
        cin >> select;

        if (select != 1 && select != 2)
        {
            cout << "\n잘못 입력하셨습니다." << endl;
        }

        else if (select == 1) // 로그인 해서 채팅 입장
        {
            
        }

        else if (select == 2) // 회원가입
        {
            while (true)
            {
                cout << "사용할 ID를 입력해 주세요(영어+숫자, 20자 이내) : ";
                cin >> id;
                // cout << id << endl;
                for (int i = 0; i < size(id); i++) // 영어, 숫자 만 있는지 체크
                {
                    if (isalnum(id[i]) == 0)
                    {
                        isalphaYN = true;
                    }
                }

                if (isalphaYN) 
                {
                    cout << "아이디는 영어와 숫자 만 입력이 가능합니다." << endl;
                    isalphaYN = false;
                }
                else 
                {
                    if (allID.find(id) != string::npos) // id 중복 체크
                    {
                        cout << "중복된 ID가 있습니다." << endl;
                    }
                    else
                    {
                        while (pwCheck == false)
                        {
                            cout << "사용할 PW를 입력해 주세요(숫자 6자리 만) : ";
                            cin >> pw;
                            cout << "입력값 : " << pw << endl;
                            if (size(pw) != 6)
                            {
                                cout << "사이즈오류" << endl;
                                pwCheck = true;
                            }
                            else
                            {
                                for (int i = 0; i < 6; i++)
                                {
                                    if (isdigit(pw[i]) == 0)
                                    {
                                        cout << "숫자아님오류" << endl;
                                        pwCheck = true;
                                    }
                                }
                            }
                            if (pwCheck)
                            {
                                cout << "비밀번호는 숫자 6자리 만 입력이 가능합니다." << endl;
                                pwCheck = false;
                            }
                            else
                            {
                                pwCheck = true;
                            }
                        }
                        
                        while (true)
                        {
                            cout << "이름을 입력해 주세요(10자 이하) : ";
                            cin >> name;
                            if (size(name) < 11)
                            {
                                break;
                            }
                            else
                            {
                                cout << "이름은 10자 이하여야 합니다." << endl;
                            }

                        }
                        

                        while (phoneCheck == false)
                        {
                            cout << "전화번호를 입력해 주세요(슬래쉬제외 11자리) : ";
                            cin >> phone;

                            if (size(phone) != 11) // 11자리 체크
                            {
                                cout << "11자리 오류" << endl;
                                phoneCheck = true;
                            }
                            else // 11자리면
                            {
                                for (int i = 0; i < size(phone); i++)
                                {
                                    if (isdigit(phone[i]) == 0) // 숫자 체크
                                    {
                                        cout << "숫자아님오류" << endl;
                                        phoneCheck = true;
                                    }
                                }
                            }
                            
                            if (phoneCheck == false)
                            {
                                break;
                            }
                            else
                            {
                                cout << "전화번호는 11개의 숫자이어야 합니다." << endl;
                                phoneCheck = false;
                            }
                        }
                        
                        join_membership(id, pw, name, phone);
                        break;   
                    }
                }
               
            }
        }
    }

}