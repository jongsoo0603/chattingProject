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
#include <conio.h> //getch() 포함 헤더


#define UP 72
#define DOWN 80
#define ENTER 13
#define MAX_SIZE 1024
//#define USE_BATCH


using namespace std;



//색상 선언
enum {
    BLACK,
    DARK_BLUE,
    DARK_GREEN,
    DARK_SKYBLUE,
    DARK_RED,
    DARK_VOILET,
    DAKR_YELLOW,
    GRAY,
    DARK_GRAY,
    BLUE,
    GREEN,
    SKYBLUE,
    RED,
    VIOLET,
    YELLOW,
    WHITE,
};



const string server = "tcp://127.0.0.1:3306";                           // 데이터베이스 주소 "tcp://127.0.0.1:3306"
const string username = "root";                                         // 데이터베이스 사용자
const string password = "1122";                                         // 데이터베이스 접속 비밀번호
SOCKET client_sock;
string my_nick, friendSend, friendAccept;
int current_state = 0;                                                  // 1 : 친구신청 진행중인 상태일 때, 0 : 아무 상태도 아님
int updateSelect;



// - participant.cpp
vector<vector<string>> getPtcpt(string myId);                               // 채팅 참가자 목록 출력 (전체)
vector<string> useSpeaker(string myId);                                     // 확성기 사용 (같은 그룹내 ID 조회)
string loginCheck(string myId);                                             // 로그인 중복체크 (참가자 중 조회)

// - check_update.cpp
void update(string myId, int updateSelect, string updateContents);          // DB 업데이트
string checkCondition(int conditionSelect);                                 // 각 항목별로 조건 판별 후 string으로 반환

// beforeChat.cpp
void getMyDM(string myId);                                                  // 이전 DM 조회 (전체일자 조회)
void getBeforeChat(string myId);                                            // 이전 전체 대화 내용 조회 (당일건만 조회)

// - chattingFunc.cpp
string inputDM(string myId);                                                                                                 // DM 기능 입력부
void outputDM(string stream1, string stream2, string stream3, string stream4, string msg, string myId);                      // DM 기능 출력부
string inputFriend(string myId);                                                                                             // 친구추가 기능 입력부
tuple<string, string, int> outputFriend(string stream1, string stream3, string stream4, string myId);                        // 친구추가 기능 출력부
void inputSpeaker(string myId, SOCKET client_sock);                                                                          // 팀 채팅 기능 입력부
void outputSpeaker(string stream1, string stream2, string stream3, string stream4, string stream5, string msg, string myId); // 팀 채팅 기능 출력부

// - chattingProject.cpp
void textcolor(int foreground, int background);                             // 콘솔 텍스트 색상 변경해주는 함수
string getFriend(string sender, string accepter);                           // 신청자 친구 목록 받고, 목록에 수락자 추가 한 string 반환
string insertMemberInfo(string id, string pw, string name, string phone);   // db에 회원가입 정보 삽입
string makeAllID();                                                         // db에서 모든 id 받아와서 string으로 붙이고 반환
void inputMembership();                                                     // 회원가입 조건 확인
void myPage(string myId, string type);                                      // 내 정보 조회
void updateMemberInfo(string myId);                                         // 내 정보 수정
void getMyfriendInfo(string myId);                                          // 친구 정보 조회
void inputLogin(string inputId, string inputPw);                            // 로그인 조건 확인
void successLogin(string inputId);                                          // 로그인 성공 후 기능 선택페이지
int chat_recv();                                                            // 채팅 받아옴
void client(string myId);                                                   // 받아온 채팅 출력, 채팅 보냄
void gotoxy(int x, int y);                                                  // 커서 이동






// 채팅 프로그램 - 메인.
int main(int argc, char* argv[])
{  
    int select, x = 39, y = 17, input;

    while(true)
    {
        system("cls");
        cout << endl;
        textcolor(YELLOW, BLACK);
        cout << "▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽" << endl;
        cout << endl << endl << endl << endl;
        cout << "           #             #     #   ###          #####  #                #          #   #        " << endl;
        cout << "           #     ####   ###   #   #            #       #               ####        #   #        " << endl;
        cout << "           #    ######   #        ###         ##       ####    #####    #          #   #        " << endl;
        cout << "           #    #        #           #         #       #   #  #    ##   #                       " << endl;
        cout << "           ####  ####    ##       ###           #####  #   #   #### #   ###        #   #        " << endl;
        cout << endl << endl << endl << endl << endl << endl;
        textcolor(GREEN, BLACK);
        cout << "                                       ▷ 로그인                                              " << endl;
        textcolor(SKYBLUE, BLACK);
        cout << "\n                                       ▷ 회원가입                                            " << endl;
        textcolor(YELLOW, BLACK);
        cout << endl << endl;
        cout << "△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△△" << endl;
        textcolor(GRAY, BLACK);

        gotoxy(x, y);
        textcolor(BLACK, WHITE);
        cout << "▶";
        textcolor(GRAY, BLACK);
        if (_kbhit())
        {
            input = _getch();
            if (input == ENTER && x == 39 && (y == 17 || y == 19))
            {
                break;
            }
            else
            {
                switch (input)
                {
                case UP: 
                    {
                        if (y - 1 > 16)
                        {
                            y -= 2;
                        }
                        break;
                    }
                case DOWN: 
                    {
                        if (y + 1 < 20)
                        {
                            y += 2;
                        }
                        break;
                    }
                }
            }
            
        }
        gotoxy(x, y);
        Sleep(50);
    }

#ifdef USE_BATCH
        select = atoi(argv[1]);
#else
        system("cls");
#endif
        if (x == 39 && y == 17) 
        {
            // 로그인
#ifdef USE_BATCH
            inputLogin(argv[2], argv[3]);
#else
            inputLogin("", "");
#endif
            // break;
        }
        else if (x == 39 && y == 19)
        {
            // 회원가입
            inputMembership();
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

                // 이전 대화내용 출력
                getBeforeChat(myId);
                
                send(client_sock, my_nick.c_str(), my_nick.length(), 0); // 연결에 성공하면 client 가 입력한 닉네임을 서버로 전송
                break;
            }
            cout << "Connecting..." << endl;
        }

        // 전체 채팅 받아서 출력
        std::thread th2(chat_recv);


        while (1)
        {
            string text;
            std::getline(cin, text);
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
                        update(friendSend, 5, getFriend(friendSend, friendAccept));
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
                else if (text == "/t")
                {
                    inputSpeaker(myId, client_sock);
                    text = "";
                }
                else if (text == "/q")
                {
                    const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
                    send(client_sock, buffer, strlen(buffer), 0); // 보내기
                    break;
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

    successLogin(my_nick);
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
            //cout << "buf :" << buf << endl;
            std::stringstream ss(msg);  // 문자열을 스트림화
            string stream1, stream2, stream3, stream4, stream5;
            // 스트림을 통해, 문자열을 공백 분리해 변수에 할당.
            ss >> stream1; // 첫 번째 단어
            ss >> stream2; // 두 번째 단어
            ss >> stream3; // 세 번째 단어
            ss >> stream4; // 네 번째 단어
            ss >> stream5; // 다섯 번째 단어

            

            if (stream3 == "/D" || stream3 == "/d") // DM (송신자 : /F 수신자 메세지)
            {
                outputDM(stream1, stream2, stream3, stream4, msg, my_nick);
            }
            else if (stream3 == "/F" || stream3 == "/f") // 친구신청 (송신자 : /F 수신자 메세지)
            {
                tie(friendSend, friendAccept, current_state) = outputFriend(stream1, stream3, stream4, my_nick);
            }
            else if (stream3 == "/T" || stream3 == "/t") // 팀채팅 (송신자 : /T 그룹이름 수신자 메세지)
            {
                outputSpeaker(stream1, stream2, stream3, stream4, stream5, msg, my_nick);
            }
            else // 명령어가 없을 때
            {
                
                if (stream1 != my_nick)
                {
                    cout << "                                                            " << msg << endl;
                }
            }
        }
        else
        {
            cout << "Server Off" << endl;
            break;
            // return -1;
        }
    }
    //successLogin(my_nick);
    cout << "chat_recv end" << endl;
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
string insertMemberInfo(string id, string pw, string name, string phone)
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

    // MySQL Connector/C++ 정리
    delete pstmt;
    delete con;

    return groupName;
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
    textcolor(SKYBLUE, BLACK);
    cout << endl << endl;
    cout << "     ●●●●       ●●●       ●●●     ●    ●                         " << endl;
    cout << "       ●       ●   ●       ●      ● ●  ●                         " << endl;
    cout << "       ●       ●   ●       ●      ●  ● ●                         " << endl;
    cout << "     ●●         ●●●       ●●●     ●    ●                         " << endl << endl << endl;
    cout << " ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽                   " << endl << endl;
    cout << endl << endl;
    textcolor(GRAY, BLACK);

    string id, pw, name, phone, groupName, action;
    id = checkCondition(1);
    pw = checkCondition(2);
    name = checkCondition(3);
    phone = checkCondition(4);
    groupName = insertMemberInfo(id, pw, name, phone);

    textcolor(SKYBLUE, BLACK);
    cout << "\n   ☆   ★   ☆   ★   ☆   ★   ☆   ★   ☆   ★   ☆   ★" << endl;
    cout << "\n   ☆               회원가입   완료             ★" << endl;
    cout << "\n   ☆   ★   ☆   ★   ☆   ★   ☆   ★   ☆   ★   ☆   ★" << endl << endl;
    cout << "        ☞ 회원님은 ";
    if (groupName == "red")
    {
        textcolor(RED, BLACK);
    }
    else if (groupName == "green")
    {
        textcolor(GREEN, BLACK);
    }
    else if (groupName == "blue")
    {
        textcolor(BLUE, BLACK);
    }
    else if (groupName == "yellow")
    {
        textcolor(YELLOW, BLACK);
    }
    cout << groupName;
    cout << " 그룹입니다.  " << endl << endl << endl;
    textcolor(GRAY, BLACK);

    while (true) {
        cout << "\n로그인하러 가기(L)" << endl;
        cin >> action;
        cin.ignore();
        if (action == "L") {
            system("cls");
            inputLogin("", "");
            break;
        }
        else {
            cout << "잘못입력하였습니다. " << endl;
        }
    }
}


// 내 정보 수정
void updateMemberInfo(string myId)
{
    int  x = 23, y = 5, input;
    while (true)
    {
        system("cls");
        cout << endl << endl << endl;
        cout << "                   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽          " << endl << endl;
        cout << "                       ▷ 초기 화면으로 돌아가기                   " << endl << endl;
        cout << "                       ▷ 비밀번호 수정                            " << endl << endl;
        cout << "                       ▷ 이름 수정                                " << endl << endl;
        cout << "                       ▷ 전화번호 수정                            " << endl << endl;

        gotoxy(x, y);
        textcolor(BLACK, WHITE);
        cout << "▶";
        textcolor(GRAY, BLACK);
        if (_kbhit())
        {
            input = _getch();
            if (input == ENTER && x == 23 && (y == 5 || y == 7 || y == 9 || y == 11))
            {
                break;
            }
            else
            {
                switch (input)
                {
                case UP:
                {
                    if (y - 1 > 4)
                    {
                        y -= 2;
                    }
                    break;
                }
                case DOWN:
                {
                    if (y + 1 < 12)
                    {
                        y += 2;
                    }
                    break;
                }
                }
            }

        }
        gotoxy(x, y);
        Sleep(50);
    }

    if (x == 23 && y == 5)
    {
        successLogin(myId);
    }
    else if(x == 23 && (y == 7 || y == 9 || y == 11))
    {
        if (y == 7) updateSelect = 2;
        else if (y == 9) updateSelect = 3;
        else if (y == 11) updateSelect = 4;
        system("cls");
        update(myId, updateSelect, checkCondition(updateSelect));
        system("cls");
        myPage(myId, "myInfo");
    }
 }


// 내 정보 조회
void myPage(string myId, string type) {
    string action, pw, chckPW, friendList;
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


    // 정보 조회 ( type : 내정보, 친구정보 분리)
    if (type == "myInfo") {
        while (res->next()) {
            pw = res->getString("passWord");
            friendList = res->getString("friendList");
            cout << endl << endl << endl << endl << endl;
            cout << "                               ▶▶▶   내정보 조회   ◀◀◀                "      << endl << endl << endl;
            cout << "                               ▷ ID : " << res->getString("memberID")             << endl << endl;
            cout << "                               ▷ PW : " << pw                                     << endl << endl;
            cout << "                               ▷ NAME : " << res->getString("name")               << endl << endl;
            cout << "                               ▷ PHONENUMBER : " << res->getString("phoneNumber") << endl << endl;
            cout << "                               ▷ GROUP : " << res->getString("groupName")         << endl << endl;
            cout << "                               ▷ FRIEND : " << friendList.erase(0,1)              << endl << endl << endl;
            textcolor(GRAY, BLACK);
        }

        while (true)
        {
            cout << "\n                              ";
            textcolor(WHITE, DARK_BLUE);
            cout << "정보 수정 : Y, ";
            textcolor(GRAY, BLACK);
            cout << "  ";
            textcolor(WHITE, DARK_RED);
            cout << "뒤로 가기 : N " << endl;
            textcolor(GRAY, BLACK);

            cin >> action;
            cin.ignore();
            if (action == "Y")
            {
                system("cls");
                cout << "비밀번호를 입력하세요 : ";
                getline(cin, chckPW);
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
    }
    else if (type == "friendInfo") {
        while (res->next()) {
            friendList = res->getString("friendList");
            cout << endl << endl;
            cout << "                               ▶▶▶   친구 정보 조회   ◀◀◀                "    << endl << endl << endl;
            cout << "                               ▷ ID : " << res->getString("memberID")              << endl << endl;
            cout << "                               ▷ NAME : " << res->getString("name")                << endl << endl;
            cout << "                               ▷ PHONENUMBER : " << res->getString("phoneNumber")  << endl << endl;
            cout << "                               ▷ GROUP : " << res->getString("groupName")          << endl << endl;
            cout << "                               ▷ FRIEND : " << friendList.erase(0, 1)              << endl << endl << endl;
            textcolor(GRAY, BLACK);
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
    string friendList = "";

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

    // 친구 목록 있을때만 친구정보 조회 가능.
    while (res->next()) {
        friendList = res->getString("friendList");
        if (friendList != "") {
            friendList.erase(0, 1);
            cout << "친구 List : [ " << friendList << " ]" << endl << endl;
            cout << "확인하고 싶은 친구의 id를 입력하세요. : ";
            cin >> friendId;
            cin.ignore();
            myPage(friendId, "friendInfo");
        }
        else {
            cout << endl << endl << endl;
            cout << "\n            ♪                                                                        ♬ " << endl;
            cout << "                   ♬                                                          ♩♪        " << endl;
            cout << "                            (  ◑ ▽ ◐)     (◑ ▽ ◐ )    ( ◑ ▽ ◐ )                   " << endl;
            cout << "                                                                                           " << endl;
            cout << "                                                                                           " << endl << endl;
            cout << "                           친구 리스트가 없어서 조회할 친구 정보가 없습니다.               " << endl;
            cout << "                               대화에 참여해 친구를 추가해보세요 !! :)                     " << endl;
        }
    }

    delete res;
    delete con;
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

    textcolor(GREEN, BLACK);
    cout << endl << endl;

    cout << "                   ●         ● ●        ●●●     ●●●     ●    ●                         " << endl;
    cout << "                   ●        ●   ●     ●          ●      ● ●  ●                         " << endl;
    cout << "                   ●        ●   ●     ●  ●●      ●      ●  ● ●                         " << endl;
    cout << "                   ●●●       ● ●       ●● ●     ●●●     ●    ●                         " << endl << endl << endl;
    cout << "                   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽                         " << endl;
    cout << endl << endl << endl;

    textcolor(GRAY, BLACK);

    // id,비번 입력받기.
    if (inputId.empty()) {
        cout << "                   ID를 입력해주세요.(영어+숫자, 20자 이내) : ";
        cin >> inputId;
        cin.ignore();
    }

    if (inputPw.empty()) {
        cout << "\n                   비밀번호를 입력해주세요.(숫자, 6자 이내) : ";
        cin >> inputPw;
        cin.ignore();
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
        textcolor(RED, BLACK);
        if (!idYN) {
            cout << "\n                   없는 ID입니다. ID를 확인해주세요." << endl;
        }
        else if (!pwYN) {
            cout << "\n                   비밀번호가 일치하지 않습니다. 비밀번호를 확인해주세요." << endl;
        }
        textcolor(GRAY, BLACK);
        cout << endl << endl << endl;
        inputId.clear();
        inputPw.clear();
        inputLogin(inputId, inputPw);
    }

}


// 로그인 성공 후 기능 선택페이지
void successLogin(string myId) {
    string loginYN = "", action;
    int x = 31, y = 12, input;

    while (true)
    {
        system("cls");
        textcolor(GREEN, BLACK);
        cout << endl << endl;
        cout << "                   ●   ●   ●●●●●   ●       ●         ●●●●        " << endl;
        cout << "                   ●   ●   ●       ●       ●        ●    ●        " << endl;
        cout << "                   ●●●●●   ●●●●●   ●       ●        ●    ●       " << endl;
        cout << "                   ●   ●   ●       ●       ●        ●    ●     " << endl;
        cout << "                   ●   ●   ●●●●●   ●●●●●   ●●●●●     ●●●●                 " << endl << endl << endl;
        cout << "                   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽   ▽                           " << endl << endl << endl;
        textcolor(GRAY, BLACK);
        cout << "                               ▷  채팅방 참가                                             " << endl << endl;
        cout << "                               ▷  채팅방 참가자 조회                                      " << endl << endl;
        cout << "                               ▷  내 정보 조회                                            " << endl << endl;
        cout << "                               ▷  친구정보 조회                                           " << endl << endl;
        cout << "                               ▷  이전 DM 조회                                            " << endl << endl;

        gotoxy(x, y);
        textcolor(BLACK, WHITE);
        cout << "▶";
        textcolor(GRAY, BLACK);

        if (_kbhit())
        {
            input = _getch();
            if (input == ENTER && x == 31 && (y == 12 || y == 14 || y == 16 || y == 18 || y == 20))
            {
                break;
            }
            else
            {
                switch (input)
                {
                case UP:
                {
                    if (y - 1 > 11)
                    {
                        y -= 2;
                    }
                    break;
                }
                case DOWN:
                {
                    if (y + 1 < 21)
                    {
                        y += 2;
                    }
                    break;
                }
                }
            }

        }
        gotoxy(x, y);
        Sleep(50);
    }
    if (x == 31 && y == 12)
    {
        // 로그인 중복체크.
        loginYN = loginCheck(myId);

        if (loginYN == "Y") {
            cout << "해당 ID는 이미 채팅방에 참가했기 때문에 채팅방에 참여하실 수 없습니다. " << endl;
        }
        else if (loginYN == "F")
        {
            cout << "채팅방 최대 인원을 초과했기 때문에 채팅방에 참여하실 수 없습니다." << endl;
        }
        else if (loginYN == "N") {
            // 채팅방 입장
            system("cls");
            client(myId);
        }
    }
    else if (x == 31 && y == 14)
    {
        // 채팅방 참가자 조회
        system("cls");
        getPtcpt(myId);
    }
    else if (x == 31 && y == 16)
    {
        // 내 정보 조회
        system("cls");
        myPage(myId, "myInfo");
    }
    else if (x == 31 && y == 18)
    {
        // 친구정보 조회
        system("cls");
        getMyfriendInfo(myId);

    }
    else if (x == 31 && y == 20)
    {
        // 이전 DM 조회
        system("cls");
        getMyDM(myId);
    }

    while (true) {
        cout << "\n이전으로 가기 (N)" << endl;
        cin >> action;
        cin.ignore();
        if (action == "N") {
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

// 커서 이동 함수
void gotoxy(int x, int y) {
    COORD posXY = { x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), posXY);
}