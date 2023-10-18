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
#include <windows.h>

#define MAX_SIZE 1024
#define USE_BATCH

//색상
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

// 콘솔 텍스트 색상 변경해주는 함수
void setColor(unsigned short text) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), text);
}


using namespace std;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1122"; // 데이터베이스 접속 비밀번호
int current_state = 0;
int updateSelect;
string friendSend, friendAccept;

SOCKET client_sock;
string my_nick;


void successLogin(string inputId);
vector<vector<string>> getPtcpt(string myId);
vector<string> useSpeaker(string myId);
void myPage(string myId);
void update(string myId, int updateSelect, string updateContents); // DB 업데이트
string checkCondition(int conditionSelect); // 각 항목별로 조건 판별 후 string으로 반환
string loginCheck(string myId);
void getMyDM(string myId);
void getBeforeChat(string myId);



// 신청자 친구 목록 받고, 목록에 수락자 추가 한 string 반환
string getFriend(string sender, string accepter)
{
    string query, frienList;

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
    if(res->next())
        frienList = res->getString("friendList") + "," + accepter; // 목록에 수락자 추가하고

    delete res;
    delete stmt;
    delete con;

    return frienList;
}


// 채팅 받아옴
int chat_recv() {
    char buf[MAX_SIZE] = { };
    string msg;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
            msg = buf;
            // cout << "buf :" << buf << endl;
            std::stringstream ss(msg);  // 문자열을 스트림화
            string stream1, stream2, stream3, stream4, stream5;
            // 스트림을 통해, 문자열을 공백 분리해 변수에 할당. 보낸 사람의 이름만 user에 저장됨.
            ss >> stream1; // 첫 번째 단어
            ss >> stream2; // 두 번째 단어
            ss >> stream3; // 세 번째 단어
            ss >> stream4; // 네 번째 단어
            ss >> stream5; // 다섯 번째 단어

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
            else if (stream3 == "/F" || stream3 == "/f") // 친구신청
            {
                if (stream3 == "/F" && stream4 == my_nick)
                {
                    friendSend = stream1;
                    friendAccept = stream4;
                    cout << "ID '" << stream1 << "'이(가) 친구 요청을 보냈습니다. 수락하시겠습니까?(Y, N) :" << endl;
                    current_state = 1;
                }
            }
            else if (stream3 == "/S" || stream3 == "/s") // 송신자 : /S 그룹이름 수신자 메세지
            {
                if (stream3 == "/S" && stream5 == my_nick)
                {
                    int eraseLength = 0;
                    eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + size(stream5) + 4;
                    msg.erase(0, eraseLength);
                    if (stream4 == "red")
                    {
                        setColor(RED);
                    }
                    else if (stream4 == "green")
                    {
                        setColor(GREEN);
                    }
                    else if (stream4 == "blue")
                    {
                        setColor(BLUE);
                    }
                    else if (stream4 == "yellow")
                    {
                        setColor(YELLOW);
                    }
                    cout << stream1 << "의 그룹 메세지 :" << msg << endl;
                    setColor(WHITE);
                }
                
            }
            else // 명령어가 없을 때
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

        // 이전 대화내용 출력
        getBeforeChat(myId);

        cout << "        §    채 팅 방    입 장    §     " << endl;
        cout << "※ 기타 기능 사용하기 ( '/d' : DM, '/f' : 친구 신청, ‘/s’ : 확성기 ) \n" << endl;

        while (1) 
        {
            string text;
            std::getline(cin, text);

            if (current_state == 1) // 친구신청 진행중인 상태일 때
            {
                while (true)
                {
                    if (text == "Y")
                    {
                        cout << friendSend << "의 친구신청을 수락했습니다." << endl;
                        // 여기에 친구 DB삽입
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
            else if(current_state == 0)// 아무 상태도 아닐 때
            {
                //  명령어 전처리 과정
                if (text == "/d")
                {
                    int end = 0;
                    string newFrdNum, message;
                    pList = getPtcpt(myId);
                    while (end == 0)
                    {
                        cout << "DM 할 사람 id 입력 : ";
                        cin >> newFrdNum;
                        vector<string> newFriend;
                        for (int i = 1; i < pList.size() + 1; i++)
                        {
                            newFriend = pList[i - 1];
                            if (newFrdNum == newFriend[1])
                            {
                                if (newFriend[2] == "N")
                                {
                                    cout << "친구가 아닌 참가자에게는 DM 할 수 없습니다." << endl;
                                    end = 1;
                                    break;
                                }
                                else if (newFriend[2] == "Y")
                                {
                                    cout << "보낼 메세지 입력 : ";
                                    getline(cin, message);
                                    getline(cin, message);
                                    text = "/D " + newFriend[1] + " " + message; // 송신자 : /D 수신자 메세지
                                    end = 1;
                                    break;
                                }
                            }
                        }
                        if (end == 0)
                        {
                            cout << "\n            * 입력 오류 *" << endl;
                            cout << "### 참가자 목록의 id를 입력하세요. ###" << endl;
                        }
                    }
                }
                else if (text == "/f")
                {
                    int end = 0;
                    string newFrdNum;
                    pList = getPtcpt(myId);
                    while (end == 0)
                    {
                        cout << "친구신청 할 사람 id 입력 : ";
                        cin >> newFrdNum;
                        vector<string> newFriend;
                        for (int i = 1; i < pList.size() + 1; i++)
                        {
                            newFriend = pList[i - 1];
                            if (newFrdNum == newFriend[1])
                            {
                                if (newFriend[2] == "N")
                                {
                                    text = "/F " + newFriend[1]; // 송신자 : /F 수신자
                                    end = 1;
                                    break;
                                }
                                else if (newFriend[2] == "Y")
                                {
                                    cout << "이미 친구입니다." << endl;
                                    end = 1;
                                    break;
                                }
                            }
                        }
                        if (end == 0)
                        {
                            cout << "\n            * 입력 오류 *" << endl;
                            cout << "### 참가자 목록의 id를 입력하세요. ###" << endl;
                        }
                    }
                    
                }
                else if (text == "/s")
                {
                    vector<string> groupInfo;
                    string groupName, message;
                    int gSize = groupInfo.size();
                    groupInfo = useSpeaker(myId);

                    groupName = groupInfo[0];
                    cout << "[" << groupName << "]group에게 보낼 메세지 입력 : ";
                    // getline(cin, message);
                    getline(cin, message);

                    for (int i = 1; i < groupInfo.size(); i++)
                    {
                        if (groupInfo.at(i) != my_nick)
                        {
                            text = "/S " + groupName + " " + groupInfo.at(i) + " " + message; // 송신자 : /S 그룹이름 수신자 메세지

                            const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
                            send(client_sock, buffer, strlen(buffer), 0); // 보내기
                        }
                        text = "";
                    }
                }
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
    cout << "☆회원님은 "<< groupName <<"그룹입니다.☆" << endl;
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
                myPage(myId);
            }
            break;
        }
    }
    
}

// 회원 정보 조회
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
        cout << "친구 List : "<< res->getString("friendList") << endl;
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
        cout << "\nID를 입력해주세요.(영어+숫자, 20자 이내) : ";
        cin >> inputId;
    }

    if (inputPw.empty()) {
        cout << "비밀번호를 입력해주세요.(숫자, 6자 이내) : ";
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
        inputLogin(inputId, inputPw);
    }

}


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



// 채팅 프로그램 - 메인.

int main(int argc, char* argv[])
{  
    int select;

    while(true)
    {
        cout << "▽▽▽▽▽▽▽▽▽▽▽" << endl;
        cout << "  1. 로그인 " << endl;
        cout << "  2. 회원가입 " << endl;
        cout << "△△△△△△△△△△△" << endl;
#ifdef USE_BATCH
        select = atoi(argv[1]);
#else
        cin >> select;
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
