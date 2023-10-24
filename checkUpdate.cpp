#include <iostream>
#include <string>
#include <mysql/jdbc.h>

using namespace std;

string getFriend(string sender, string accepter);

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1122"; // 데이터베이스 접속 비밀번호


sql::mysql::MySQL_Driver* driver;
sql::Connection* con;
sql::Statement* stmt;
sql::ResultSet* res;

string makeAllID();


// conditionSelect(id = 1, pw = 2, name = 3, phone = 4)
string checkCondition(int conditionSelect) 
{
    string id, name, phone, pw;
    bool isalphaCheck = true, pwCheck = true, phoneCheck = true, nameCheck = true;
    switch (conditionSelect)
    {
    case 1:
        while (true)
        {
            cout << "\n사용할 ID를 입력해 주세요(영어와 숫자 만, 20자 이내) : ";
            cin >> id;
            cin.ignore();
            for (int i = 0; i < size(id); i++) // 영어, 숫자 만 있는지 체크
            {
                if (isalnum(id[i]) == 0)
                {
                    isalphaCheck = false;
                }
            }

            if (isalphaCheck == false)
            {
                cout << "아이디는 영어와 숫자 만 입력이 가능합니다." << endl;
                isalphaCheck = true;
            }
            else
            {
                if (makeAllID().find(id) != string::npos) // id 중복 체크
                {
                    cout << "중복된 ID가 있습니다." << endl;
                }
                else
                {
                    return id;
                    break;
                }
            }
        }
        break;
    case 2:
        while (true)
        {
            cout << "\n사용할 PW를 입력해 주세요(숫자 6자리 만) : ";
            cin >> pw;
            cin.ignore();
            if (size(pw) != 6)
            {
                pwCheck = false;
            }
            else
            {
                for (int i = 0; i < 6; i++)
                {
                    if (isdigit(pw[i]) == 0)
                    {
                        pwCheck = false;
                    }
                }
            }
            if (pwCheck == false)
            {
                cout << "비밀번호는 숫자 6자리 만 입력이 가능합니다." << endl;
                pwCheck = true;
            }
            else 
            {
                return pw;
                break;
            }
        }
        break;
    case 3:
        while (true)
        {
            cout << "\n이름을 입력해 주세요(10자 이하) : ";
            cin >> name;
            cin.ignore();
            if (size(name) < 1 || size(name) > 10)
            {
                cout << "이름은 10자 이하여야 합니다." << endl;
                nameCheck = false;
            }
            else
            {
                for (int i = 0; i < size(name); i++) // 영어, 한글 만 있는지 체크
                {
                    if (isalpha(name[i]) == 0) // 영어도 아니고
                    {
                        if((name[i] & 0x80) != 0x80) // 한글도 아니면
                        { 
                            nameCheck = false;
                        }
                    }
                }
            }
            if (nameCheck == false)
            {
                cout << "영어나 한글 문자 이외의 문자가 포함되어 있습니다." << endl;
                nameCheck = true;
            }
            else
            {
                return name;
                break;
            }
        }
        break;
    case 4:
        while (true)
        {
            cout << "\n전화번호를 입력해 주세요(슬래쉬제외 11자리) : ";
            cin >> phone;
            cin.ignore();
            if (size(phone) != 11) // 11자리 체크
            {
                phoneCheck = false;
            }
            else // 11자리면
            {
                for (int i = 0; i < size(phone); i++)
                {
                    if (isdigit(phone[i]) == 0) // 숫자 체크
                    {
                        phoneCheck = false;
                    }
                }
            }

            if (phoneCheck == false)
            {
                cout << "전화번호는 11개의 숫자이어야 합니다." << endl;
                phoneCheck = true;
            }
            else
            {
                return phone;
                break;
            }
        }
        break;
    default:
        break;
    }
    system("cls");
}


// updateSelect(pw = 2, name = 3, phone = 4, friend = 5)
void update(string myId, int updateSelect, string updateContents) 
{
    string updateItem;
    

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);

        // db 한글 저장을 위한 셋팅 
        stmt = con->createStatement();
        stmt->execute("set names euckr");
        if (stmt) { delete stmt; stmt = nullptr; }

        con->setSchema("chattingproject");

        stmt = con->createStatement();
    
        if (updateSelect == 2) updateItem = "password";
        else if (updateSelect == 3) updateItem = "name";
        else if (updateSelect == 4) updateItem = "phoneNumber";
        else if (updateSelect == 5)
        {
            updateItem = "friendList";
        }
    
        string query = "UPDATE member SET " + updateItem + " = '" + updateContents + "' WHERE (memberID = '" + myId + "')"; // DB에 삽입
        stmt->execute(query);
    }
    catch (sql::SQLException& e) {
        delete stmt;
        delete con;
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    delete stmt;
    delete con;
}




