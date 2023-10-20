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
#include <stdlib.h>
#include <stdio.h>



#define MAX_SIZE 1024
#define USE_BATCH

using namespace std;



//색상 선언
enum {
    BLACK = 0,
    GRAY = 7,
    BLUE = 9,
    GREEN,
    RED = 12,
    VIOLET,
    YELLOW,
    WHITE
};



const string server = "tcp://127.0.0.1:3306";                           // 데이터베이스 주소
const string username = "root";                                         // 데이터베이스 사용자
const string password = "1122";                                         // 데이터베이스 접속 비밀번호
SOCKET client_sock;
string my_nick, friendSend, friendAccept;
int current_state = 0;                                                  // 1 : 친구신청 진행중인 상태일 때, 0 : 아무 상태도 아님
int updateSelect;



// participant.cpp
vector<vector<string>> getPtcpt(string myId);                           // 채팅 참가자 목록 출력 (전체)
vector<string> useSpeaker(string myId);                                 // 확성기 사용 (같은 그룹내 ID 조회)
string loginCheck(string myId);                                         // 로그인 중복체크 (참가자 중 조회)

// check_update.cpp
void update(string myId, int updateSelect, string updateContents);      // DB 업데이트
string checkCondition(int conditionSelect);                             // 각 항목별로 조건 판별 후 string으로 반환

// beforeChat.cpp
void getMyDM(string myId);                                              // 이전 DM 조회 (전체일자 조회)
void getBeforeChat(string myId);                                        // 이전 전체 대화 내용 조회 (당일건만 조회)

// -chattingFunc.cpp
string inputDM(string myId);                                            // DM 기능 입력부
void outputDM(string stream1, string stream2, string stream3, string stream4, string msg, string myId);                      // DM 기능 출력부
string inputFriend(string myId);                                        // 친구추가 기능 입력부
tuple<string, string, int> outputFriend(string stream1, string stream3, string stream4, string myId);                        // 친구추가 기능 출력부
string inputSpeaker(string myId, SOCKET client_sock);                   // 확성기 기능 입력부
void outputSpeaker(string stream1, string stream2, string stream3, string stream4, string stream5, string msg, string myId); // 친구추가 기능 출력부

// - chattingProject.cpp
void textcolor(int foreground, int background);                         // 콘솔 텍스트 색상 변경해주는 함수
string getFriend(string sender, string accepter);                       // 신청자 친구 목록 받고, 목록에 수락자 추가 한 string 반환
void insertMemberInfo(string id, string pw, string name, string phone); // db에 회원가입 정보 삽입
string makeAllID();                                                     // db에서 모든 id 받아와서 string으로 붙이고 반환
void inputMembership();                                                 // 회원가입 조건 확인
void myPage(string myId);                                               // 내 정보 조회
void updateMemberInfo(string myId);                                     // 내 정보 수정
void getMyfriendInfo(string myId);                                      // 친구 정보 조회
void inputLogin(string inputId, string inputPw);                        // 로그인 조건 확인
void successLogin(string inputId);                                      // 로그인 성공 후 기능 선택페이지
int chat_recv();                                                        // 채팅 받아옴
void client(string myId);                                               // 받아온 채팅 출력, 채팅 보냄






// 채팅 프로그램 - 메인.
int main(int argc, char* argv[])
{  
    int select;

    while(true)
    {
        system("cls");
        cout << "▽▽▽▽▽▽▽▽▽▽▽" << endl;
        cout << "  1. 로그인 " << endl;
        cout << "  2. 회원가입 " << endl;
        cout << "△△△△△△△△△△△" << endl;
#ifdef USE_BATCH
        select = atoi(argv[1]);
#else
        cin >> select;
        system("cls");
#endif
        if (select == 1) 
        {
            // 로그인
#ifdef USE_BATCH
            inputLogin(argv[2], argv[3]);
#else
            inputLogin("", "");
#endif
            break;
        }
        else if (select == 2) 
        {
            // 회원가입
            inputMembership();
        }
        else 
        {
            cout << "\n잘못 입력하셨습니다." << endl;
        }
    }
}





// 받아온 채팅 출력, 채팅 보냄
void client(string myId)
{
    WSADATA wsa;

    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);
    vector<vector<string>> pList;

    if (!code) {
        my_nick = myId;

        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

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

        // 이전 대화내용 출력
        getBeforeChat(myId);
        cout << endl;
        textcolor(VIOLET, WHITE);
        cout << "#########################################################################";
        textcolor(GRAY, BLACK);
        cout << endl;
        textcolor(VIOLET, WHITE);
        cout << "#                                                                       #";
        textcolor(GRAY, BLACK);
        cout << endl;
        textcolor(VIOLET, WHITE);
        cout << "#                    §    채 팅 방    입 장    §                        #";
        textcolor(GRAY, BLACK);
        cout << endl;
        textcolor(VIOLET, WHITE);
        cout << "# ※ 기타 기능 사용하기 ( '/d' : DM, '/f' : 친구 신청, ‘/s’ : 확성기 )※  #";
        textcolor(GRAY, BLACK);
        cout << endl;
        textcolor(VIOLET, WHITE);
        cout << "#                                                                       #";
        textcolor(GRAY, BLACK);
        cout << endl;
        textcolor(VIOLET, WHITE);
        cout << "#########################################################################";
        textcolor(GRAY, BLACK);
        cout << endl << endl;

        while (1)
        {
            string text;
            std::getline(cin, text);
            cout << "text : " << text << ", current : " << current_state << endl;
            // 친구신청 진행중인 상태일 때
            if (current_state == 1) 
            {
                while (true)
                {
                    if (text == "Y")
                    {
                        cout << friendSend << "의 친구신청을 수락했습니다." << endl;
                        // 전역변수 : friendSend = 친구 신청자, friendAccept = 친구 수락자
                        text = "/D " + friendSend + " " + friendAccept + "가 친구신청을 수락했습니다.";
                        update(myId, 5, getFriend(friendSend, friendAccept));
                        update(myId, 5, getFriend(friendAccept, friendSend));
                        break;
                    }
                    else if (text == "N")
                    {
                        cout << friendSend << "의 친구신청을 거절했습니다." << endl;
                        text = "/D " + friendSend + " " + friendAccept + "가 친구신청을 거절했습니다.";
                        break;
                    }
                    else
                    {
                        cout << "잘못입력하셨습니다." << endl;
                        cout << "ID '" << friendSend << "'이(가) 친구 요청을 보냈습니다. 수락하시겠습니까?(Y, N) :" << endl;
                        std::getline(cin, text);
                    }
                }
                current_state = 0;
            }
            else if (current_state == 0)// 아무 상태도 아닐 때
            {
                //  명령어 전처리 과정
                if (text == "/d")
                {
                    text = inputDM(myId);
                }
                else if (text == "/f")
                {
                    text = inputFriend(myId);
                }
                else if (text == "/s")
                {
                    cout << "텍스트 : " << endl;
                    text = inputSpeaker(myId, client_sock);
                }
            }
            if (!text.empty()) {
                const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
                send(client_sock, buffer, strlen(buffer), 0); // 보내기
            }
        }
        th2.join();
        closesocket(client_sock);
    }
    WSACleanup();
}


// 채팅 받아옴
int chat_recv() {
    char buf[MAX_SIZE] = { };
    string msg;
    while (1)
    {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) 
        {
            msg = buf;
            cout << "buf :" << buf << endl;
            std::stringstream ss(msg);  // 문자열을 스트림화
            string stream1, stream2, stream3, stream4, stream5;
            // 스트림을 통해, 문자열을 공백 분리해 변수에 할당.
            ss >> stream1; // 첫 번째 단어
            ss >> stream2; // 두 번째 단어
            ss >> stream3; // 세 번째 단어
            ss >> stream4; // 네 번째 단어
            ss >> stream5; // 다섯 번째 단어

            cout << "1 : " << stream1 << endl;
            cout << "2 : " << stream2 << endl;
            cout << "3 : " << stream3 << endl;
            cout << "4 : " << stream4 << endl;
            cout << "5 : " << stream5 << endl;

            if (stream3 == "/D" || stream3 == "/d") // DM (송신자 : /F 수신자 메세지)
            {
                outputDM(stream1, stream2, stream3, stream4, msg, my_nick);
            }
            else if (stream3 == "/F" || stream3 == "/f") // 친구신청 (송신자 : /F 수신자 메세지)
            {
                tie(friendSend, friendAccept, current_state) = outputFriend(stream1, stream3, stream4, my_nick);
                cout << "friendSend, friendAccept, current_state : " << friendSend  << ", " << friendAccept << ", " << current_state << endl;
            }
            else if (stream3 == "/S" || stream3 == "/s") // 팀채팅 (송신자 : /S 그룹이름 수신자 메세지)
            {
                outputSpeaker(stream1, stream2, stream3, stream4, stream5, msg, my_nick);
            }
            else // 명령어가 없을 때
            {
                
                if (stream1 != my_nick)
                {
                    cout << msg << endl;
                }
            }
        }
        else
        {
            cout << "Server Off" << endl;
            return -1;
        }
    }
}


// 신청자 친구 목록 받고, 목록에 수락자 추가 한 string 반환
string getFriend(string sender, string accepter)
{
    string query, friendList;

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

    con->setSchema("chattingproject");

    stmt = con->createStatement();
    query = "SELECT friendList FROM member WHERE memberID = '" + sender + "'";
    res = stmt->executeQuery(query); // 신청자 친구 목록 받고
    if (res->next())
        friendList = res->getString("friendList") + "," + accepter; // 목록에 수락자 추가하고

    delete res;
    delete stmt;
    delete con;

    return friendList;
}


// db에 회원가입 정보 삽입
void insertMemberInfo(string id, string pw, string name, string phone)
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
    pstmt = con->prepareStatement("INSERT INTO member(memberID, passWord, name, phoneNumber, groupName, friendList) VALUES(?,?,?,?,?,?)");

    pstmt->setString(1, id);
    pstmt->setString(2, pw);
    pstmt->setString(3, name);
    pstmt->setString(4, phone);
    pstmt->setString(5, groupName);
    pstmt->setString(6, "");
    pstmt->execute();

    cout << "\n☆★☆ 회원가입 완료 ☆★☆" << endl;
    cout << "☆회원님은 " << groupName << "그룹입니다.☆" << endl;
    cout << endl;

    // MySQL Connector/C++ 정리
    delete pstmt;
    delete con;
}


// db에서 모든 id 받아와서 string으로 붙이고 반환
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
    string id, pw, name, phone;
    id = checkCondition(1);
    pw = checkCondition(2);
    name = checkCondition(3);
    phone = checkCondition(4);
    insertMemberInfo(id, pw, name, phone);
}


// 내 정보 수정
void updateMemberInfo(string myId)
{
    while (true)
    {
        system("cls");
        cout << "\n▽▽▽▽▽▽▽▽▽▽▽▽▽▽" << endl;
        cout << "  1. 초기 화면으로 돌아가기 " << endl;
        cout << "  2. 비밀번호 수정 " << endl;
        cout << "  3. 이름 수정 " << endl;
        cout << "  4. 전화번호 수정 " << endl;
        cout << "△△△△△△△△△△△△△△" << endl;
        cin >> updateSelect;

        if (updateSelect < 1 && updateSelect > 4)
        {
            cout << "잘못 입력하셨습니다." << endl;
        }
        else
        {

            if (updateSelect == 1)
            {
                successLogin(myId);
            }
            else
            {
                update(myId, updateSelect, checkCondition(updateSelect));
                system("cls");
                myPage(myId);
            }
            break;
        }
    }
}


// 내 정보 조회
void myPage(string myId) {
    string action, pw, chckPW;
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

    // db 한글 저장을 위한 셋팅 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();
    string sql = "SELECT memberID,passWord, name, phoneNumber, groupName, friendList FROM member WHERE memberID ='" + myId + "'";
    res = stmt->executeQuery(sql);

    while (res->next()) {
        pw = res->getString("passWord");
        cout << "\n  ◇◆◇  내 정보 조회  ◆◇◆  " << endl;
        cout << "   ID : " << res->getString("memberID") << endl;
        cout << "   PW : " << pw << endl;
        cout << "   NAME : " << res->getString("name") << endl;
        cout << "   PHONENUMBER : " << res->getString("phoneNumber") << endl;
        cout << "   GROUP : " << res->getString("groupName") << endl;
        cout << "   FRIEND : " << res->getString("friendList") << endl;
        cout << "  ◇◆◇◆◇◆◇◆◇◆◇◆◇◆◇◆  " << endl;
    }


    while (true)
    {
        cout << "\n  정보 수정 : Y, 뒤로 가기 : N" << endl;
        cin >> action;
        if (action == "Y")
        {
            system("cls");
            cout << "비밀번호를 입력하세요 : ";
            cin >> chckPW;
            if (chckPW == pw)
            {
                updateMemberInfo(myId);
                break;
            }
            else
            {
                cout << "비밀번호가 일치하지 않습니다." << endl;
            }
        }
        else if (action == "N")
        {
            successLogin(myId);
            break;
        }
        else
        {
            cout << "잘못 입력하셨습니다." << endl;
        }
    }
    delete res;
    delete con;
}


// 친구 정보 조회
void getMyfriendInfo(string myId) {
    string friendId;
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

    // db 한글 저장을 위한 셋팅 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();
    string sql = "SELECT friendList FROM member WHERE memberID ='" + myId + "'";
    res = stmt->executeQuery(sql);

    while (res->next()) {
        cout << "친구 List : " << res->getString("friendList") << endl;
    }

    delete res;
    delete con;

    cout << "확인하고 싶은 친구의 id를 입력하세요.";
    cin >> friendId;
    myPage(friendId);
}


// 로그인 조건 확인
void inputLogin(string inputId, string inputPw) {

    //string inputId, inputPw;
    bool login = false, idYN = false, pwYN = false;


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
    if (inputId.empty()) {
        cout << "\n\nID를 입력해주세요.(영어+숫자, 20자 이내) : ";
        cin >> inputId;
    }

    if (inputPw.empty()) {
        cout << "\n비밀번호를 입력해주세요.(숫자, 6자 이내) : ";
        cin >> inputPw;
    }

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
        //로그인 성공
        my_nick = inputId;
        successLogin(inputId);
    }
    else {
        //로그인 실패. - 다시 로그인 받기.
        if (!idYN) {
            cout << "없는 ID입니다. ID를 확인해주세요." << endl;
        }
        else if (!pwYN) {
            cout << "비밀번호가 일치하지 않습니다. 비밀번호를 확인해주세요." << endl;
        }
        inputId.clear();
        inputPw.clear();
        inputLogin(inputId, inputPw);
    }

}


// 로그인 성공 후 기능 선택페이지
void successLogin(string myId) {
    int select;
    string action;
    string loginYN = "";
    system("cls");

    cout << "\n▽▽▽▽▽▽▽▽▽▽▽▽▽▽" << endl;
    cout << "  1. 채팅방 입장 " << endl;
    cout << "  2. 채팅방 참가자 조회 " << endl;
    cout << "  3. 내 정보 조회 " << endl;
    cout << "  4. 친구정보 조회 " << endl;
    cout << "  5. 이전 DM 보기 " << endl;
    cout << "△△△△△△△△△△△△△△" << endl;

    cin >> select;

    if (select == 1)
    {
        // 로그인 중복체크.
        loginYN = loginCheck(myId);

        if (loginYN == "Y") {
            cout << "해당 ID는 이미 로그인 중으로 채팅방에 참여하실 수 없습니다. " << endl;
        }
        else if (loginYN == "N") {
            // 채팅방 입장
            system("cls");
            client(myId);
        }
    }
    else if (select == 2)
    {
        // 채팅방 참가자 조회
        system("cls");
        getPtcpt(myId);
    }
    else if (select == 3)
    {
        // 회원정보 조회
        system("cls");
        myPage(myId);
    }
    else if (select == 4)
    {
        // 친구정보 조회
        system("cls");
        getMyfriendInfo(myId);

    }
    else if (select == 5)
    {
        // 이전 DM 조회
        system("cls");
        getMyDM(myId);
    }

    while (1) {
        cout << "\n이전으로 가기 (Y)" << endl;
        cin >> action;
        if (action == "Y") {
            successLogin(myId);
            break;
        }
        else {
            cout << "잘못 입력하였습니다. 다시 입력해주세요." << endl;
        }
    }
}


// 콘솔 텍스트 색상 변경해주는 함수
void textcolor(int foreground, int background)
{
    int color = foreground + background * 16;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}