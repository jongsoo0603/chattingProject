#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <WinSock2.h> //Winsock 헤더파일 include. WSADATA 들어있음.
#include <WS2tcpip.h>
#include <sstream>
#include <thread>
#include <ctime>
#include <random>
#include <windows.h>

#define MAX_SIZE 1024


using namespace std;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1122"; // 데이터베이스 접속 비밀번호

SOCKET client_sock;
string my_nick;


// 참가자 목록 출력
void getPtcpt() {
    int i = 0;

    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver;
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

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();
    res = stmt->executeQuery("SELECT memberID FROM participant");
    
    // 참가자 목록 출력.
    cout << " ▷ 참가자 목록 ◁" << endl;
    while (res->next()) {
        i++;
        cout << i << ". " << res->getString("memberID") << endl;
    }
    delete res;
    delete con;
}

// 채팅 받아옴
int chat_recv() {
    char buf[MAX_SIZE] = { };
    string msg;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
            msg = buf;
            std::stringstream ss(msg);  // 문자열을 스트림화
            string stream1, stream2, stream3, stream4;
            // 스트림을 통해, 문자열을 공백 분리해 변수에 할당. 보낸 사람의 이름만 user에 저장됨.
            ss >> stream1; // 첫 번째 단어
            ss >> stream2; // 두 번째 단어
            ss >> stream3; // 세 번째 단어
            ss >> stream4; // 네 번째 단어


            if (stream3 == "/D" || stream3 == "/d") // DM
            {
                int eraseLength = 0;
                eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + 3;
                msg.erase(0, eraseLength);
                if (stream3 == "/D"  && stream4 == my_nick)
                {
                    cout << stream1 << "의 귓속말 :" << msg << endl;
                }
                
            }
            else // 아무것도 안붙였을 때
            {
                if (stream1 != my_nick) 
                {
                    cout << msg << endl;
                }
            }
        }
        else {
            cout << "Server Off" << endl;
            return -1;
        }
    }
}

// 받아온 채팅 출력, 채팅 보냄
void client(string inputId)
{
    WSADATA wsa;

    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        my_nick = inputId;

        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // 

        // 연결할 서버 정보 설정 부분
        SOCKADDR_IN client_addr = {};
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(7777);
        InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr); // 연결할 서버의 ip주소

        while (1) {
            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) { // 위에 설정한 정보에 해당하는 server로 연결!
                cout << "Server Connect" << endl;
                send(client_sock, my_nick.c_str(), my_nick.length(), 0); // 연결에 성공하면 client 가 입력한 닉네임을 서버로 전송
                break;
            }
            cout << "Connecting..." << endl;
        }

        // 전체 채팅 받아서 출력
        std::thread th2(chat_recv); 

        while (1) {
            string text, reciever, message;
            std::getline(cin, text);

            // 보내기 전에 여기서 전처리 해야됨
            if (text == "/d")
            {
                cout << "채팅방에 있는 친구목록: 이따가 친구 목록 여기에 출력" << endl;
                cout << "DM할 친구 id 입력 : ";
                cin >> reciever;
                cout << "보낼 메세지 입력 : ";
                getline(cin, message);
                getline(cin, message);
                text = "/D " + reciever + " " + message;
            }
            else if (text == "/g")
            {

            }

            const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
            send(client_sock, buffer, strlen(buffer), 0); // 보내기
        }
        th2.join();
        closesocket(client_sock);
    }

    WSACleanup();
}

// db에 회원가입 정보 삽입
void insertDB(string id, string pw, string name, string phone)
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

    // 회원가입 시 그룹 랜덤 배정
    string groupName;
    int rNum;
    srand(time(NULL));
    rNum = (rand() % 4 + 1);
    if (rNum == 1) {
        groupName = "red";
    }
    else if (rNum == 2) {
        groupName = "green";
    }
    else if (rNum == 3) {
        groupName = "blue";
    }
    else if (rNum == 4) {
        groupName = "yellow";
    }

    // 데이터베이스 쿼리 실행
    // INSERT
    pstmt = con->prepareStatement("INSERT INTO member(memberID, passWord, name, phoneNumber, groupName) VALUES(?,?,?,?,?)");

    pstmt->setString(1, id);
    pstmt->setString(2, pw);
    pstmt->setString(3, name);
    pstmt->setString(4, phone);
    pstmt->setString(5, groupName);
    pstmt->execute();

    cout << "\n☆★☆ 회원가입 완료 ☆★☆" << endl;
    cout << "☆회원님은 "<< groupName <<"그룹입니다.☆" << endl;
    cout << endl;

    // MySQL Connector/C++ 정리
    delete pstmt;
    delete con;
} 

// db에서 모든 id 받아와서 string으로 붙이기
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

// 회원가입 조건 확인
void inputMembership()
{
    string id, name, phone, pw, allID = makeAllID();
    bool isalphaYN = false, pwCheck = false, phoneCheck = false;

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
                    if (size(pw) != 6)
                    {
                        pwCheck = true;
                    }
                    else
                    {
                        for (int i = 0; i < 6; i++)
                        {
                            if (isdigit(pw[i]) == 0)
                            {
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

                insertDB(id, pw, name, phone);
                break;
            }
        }

    }
}


// 회원 정보 조회
void myPage(string id) {
    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver;
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

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();
    string sql = "SELECT memberID,passWord, name, phoneNumber, groupName, friendList FROM member WHERE memberID ='" + id +"'";
    res = stmt->executeQuery(sql);

    //pstmt->setString(5, groupName);
    //pstmt->execute();
    while (res->next()) {
        cout << "  ◇◆◇  회원 정보 조회  ◆◇◆  " << endl;
        cout << "   ID : " << res->getString("memberID") << endl;
        cout << "   NAME : " << res->getString("name") << endl;
        cout << "   PHONENUMBER : " << res->getString("phoneNumber") << endl;
        cout << "   GROUP : " << res->getString("groupName") << endl;
        cout << "   FRIEND : " << res->getString("friendList") << endl;
        cout << "  ◇◆◇◆◇◆◇◆◇◆◇◆◇◆◇◆  " << endl;
    }

    delete res;
    delete con;
}


// 로그인 조건 확인
void inputLogin() {
    string inputId, inputPw;
    bool login = false, idYN = false, pwYN = false;;

    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver;
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

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();
    res = stmt->executeQuery("SELECT memberID,passWord FROM member");


    // id,비번 입력받기.
    cout << "\nID를 입력해주세요.(영어+숫자, 20자 이내) : ";
    cin >> inputId;
    cout << "비밀번호를 입력해주세요.(숫자, 6자 이내) : ";
    cin >> inputPw;

    // id, 비번 확인.
    while (res->next()) {
        if (res->getString("memberID") == inputId) {
            //해당 id 존재.
            idYN = true;
            if (res->getString("passWord") == inputPw) {
                pwYN = true;
                login = true;
            }
        }
    }
    delete res;
    delete con;

    if (login) {
        int select;
        //로그인 성공. 
        cout << "\n▽▽▽▽▽▽▽▽▽▽▽▽▽▽" << endl;
        cout << "  1. 채팅방 입장 " << endl;
        cout << "  2. 채팅방 참가자 조회 " << endl;
        cout << "  3. 회원정보 조회 " << endl;
        cout << "  4. 친구정보 조회 " << endl;
        cout << "△△△△△△△△△△△△△△" << endl;

        cin >> select;

        if (select == 1)
        {
            // 채팅방 입장
            client(inputId);
        }
        else if (select == 2)
        {
            // 채팅방 참가자 조회
            getPtcpt();
        }
        else if (select == 3)
        {
            // 회원정보 조회
            myPage(inputId);
        }
        else if (select == 4)
        {
            // 친구정보 조회
            
        }
    }
    else {
        //로그인 실패. - 다시 로그인 받기.
        if (!idYN) {
            cout << "없는 ID입니다. ID를 확인해주세요." << endl;
        }
        else if (!pwYN) {
            cout << "비밀번호가 일치하지 않습니다. 비밀번호를 확인해주세요." << endl;
        }
        inputLogin();
    }

}



// 채팅 프로그램 - 메인.
int main()
{  
    int select;

    while(true)
    {
        cout << "▽▽▽▽▽▽▽▽▽▽▽▽▽▽" << endl;
        cout << "  1. 로그인 " << endl;
        cout << "  2. 회원가입 " << endl;
        cout << "  3. 그룹별 랭킹 조회 " << endl;
        cout << "△△△△△△△△△△△△△△" << endl;

        cin >> select;

        if (select == 1) 
        {
            // 로그인
            inputLogin();
            break;
        }
        else if (select == 2) 
        {
            // 회원가입
            inputMembership();
        }
        else if (select == 3) 
        {
            // 그룹별 랭킹 조회
            inputMembership();
        }
        else 
        {
            cout << "\n잘못 입력하셨습니다." << endl;
        }
    }
}
