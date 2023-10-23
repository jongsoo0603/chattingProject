#pragma comment(lib, "ws2_32.lib") //명시적인 라이브러리의 링크. 윈속 라이브러리 참조

#include <WinSock2.h>
#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <mysql/jdbc.h>
#include <sstream>


#define MAX_SIZE 1024
#define MAX_CLIENT 10

using std::cout;
using std::cin;
using std::endl;
using std::string;

struct SOCKET_INFO { // 연결된 소켓 정보에 대한 틀 생성
    SOCKET sck;
    string user;
};

std::vector<SOCKET_INFO> sck_list; // 연결된 클라이언트 소켓들을 저장할 배열 선언.
SOCKET_INFO server_sock; // 서버 소켓에 대한 정보를 저장할 변수 선언.
int client_count = 0; // 현재 접속해 있는 클라이언트를 count 할 변수 선언.
std::vector<string> pctList = {};

void server_init(); // socket 초기화 함수. socket(), bind(), listen() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void add_client(); // 소켓에 연결을 시도하는 client를 추가(accept)하는 함수. client accept() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void send_msg(const char* msg); // send() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void recv_msg(int idx); // recv() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void del_client(int idx); // 소켓에 연결되어 있는 client를 제거하는 함수. closesocket() 실행됨. 자세한 내용은 함수 구현부에서 확인.
void insertPtcpt();
void insertMsgInfo(string msg);

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1122"; // 데이터베이스 접속 비밀번호


int main() {
    WSADATA wsa;

    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        server_init();

        std::thread th1[MAX_CLIENT];
        for (int i = 0; i < MAX_CLIENT; i++) {
            // 인원 수 만큼 thread 생성해서 각각의 클라이언트가 동시에 소통할 수 있도록 함.
            th1[i] = std::thread(add_client);
        }
        //std::thread th1(add_client); // 이렇게 하면 하나의 client만 받아짐...

        while (1) { // 무한 반복문을 사용하여 서버가 계속해서 채팅 보낼 수 있는 상태를 만들어 줌. 반복문을 사용하지 않으면 한 번만 보낼 수 있음.
            string text, msg = "";

            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = server_sock.user + " : " + buf;
            send_msg(msg.c_str());
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            th1[i].join();
            //join : 해당하는 thread 들이 실행을 종료하면 리턴하는 함수.
            //join 함수가 없으면 main 함수가 먼저 종료되어서 thread가 소멸하게 됨.
            //thread 작업이 끝날 때까지 main 함수가 끝나지 않도록 해줌.
        }
        //th1.join();

        closesocket(server_sock.sck);
    }
    else {
        cout << "프로그램 종료. (Error code : " << code << ")";
    }

    WSACleanup();

    return 0;
}

void server_init() {
    server_sock.sck = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Internet의 Stream 방식으로 소켓 생성
   // SOCKET_INFO의 소켓 객체에 socket 함수 반환값(디스크립터 저장)
   // 인터넷 주소체계, 연결지향, TCP 프로토콜 쓸 것. 

    SOCKADDR_IN server_addr = {}; // 소켓 주소 설정 변수
    // 인터넷 소켓 주소체계 server_addr

    server_addr.sin_family = AF_INET; // 소켓은 Internet 타입 
    server_addr.sin_port = htons(7777); // 서버 포트 설정
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 서버이기 때문에 local 설정한다. 
    //Any인 경우는 호스트를 127.0.0.1로 잡아도 되고 localhost로 잡아도 되고 양쪽 다 허용하게 할 수 있따. 그것이 INADDR_ANY이다.
    //ip 주소를 저장할 땐 server_addr.sin_addr.s_addr -- 정해진 모양?

    bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr)); // 설정된 소켓 정보를 소켓에 바인딩한다.
    listen(server_sock.sck, SOMAXCONN); // 소켓을 대기 상태로 기다린다.
    server_sock.user = "server";

    cout << "Server On" << endl;
    insertPtcpt();
}

void add_client() {
    SOCKADDR_IN addr = {};
    int addrsize = sizeof(addr);
    char buf[MAX_SIZE] = { };

    ZeroMemory(&addr, addrsize); // addr의 메모리 영역을 0으로 초기화

    SOCKET_INFO new_client = {};

    new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);
    recv(new_client.sck, buf, MAX_SIZE, 0);
    // Winsock2의 recv 함수. client가 보낸 닉네임을 받음.
    new_client.user = string(buf);

    string msg = "[공지] " + new_client.user + " 님이 입장했습니다.";
    pctList.push_back(new_client.user);
    insertPtcpt();

    cout << msg << endl;
    sck_list.push_back(new_client); // client 정보를 답는 sck_list 배열에 새로운 client 추가

    std::thread th(recv_msg, client_count);
    // 다른 사람들로부터 오는 메시지를 계속해서 받을 수 있는 상태로 만들어 두기.

    client_count++; // client 수 증가.
    // cout << "[공지] 현재 접속자 수 : " << client_count << "명" << endl;
    send_msg(msg.c_str()); // c_str : string 타입을 const chqr* 타입으로 바꿔줌.

    th.join();
}

void send_msg(const char* msg) {
    for (int i = 0; i < client_count; i++) { // 접속해 있는 모든 client에게 메시지 전송
        send(sck_list[i].sck, msg, MAX_SIZE, 0);
    }

    // 메세지 insert DB
    insertMsgInfo(msg);
}

void recv_msg(int idx) {
    char buf[MAX_SIZE] = { };
    string msg = "";
    int pIdx = 0;

    //cout << sck_list[idx].user << endl;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        
        int x = 0;
        x = recv(sck_list[idx].sck, buf, MAX_SIZE, 0);
        msg = sck_list[idx].user + " : " + buf;

        if (msg == sck_list[idx].user + " : /q" || x < 1)
        {
            msg = "[공지] " + sck_list[idx].user + " 님이 퇴장했습니다.";
            pIdx = std::find(pctList.begin(), pctList.end(), sck_list[idx].user) - pctList.begin();
            pctList.erase(pctList.begin() + pIdx);
            insertPtcpt();

            cout << msg << endl;
            send_msg(msg.c_str());
            del_client(idx); // 클라이언트 삭제
            return;
        }
        else
        {
            cout << msg << endl;
            send_msg(msg.c_str());
        }
        //if (recv(sck_list[idx].sck, buf, MAX_SIZE, 0) > 0) { // 오류가 발생하지 않으면 recv는 수신된 바이트 수를 반환. 0보다 크다는 것은 메시지가 왔다는 것.
        //    msg = sck_list[idx].user + " : " + buf;
        //    cout << msg << endl;
        //    send_msg(msg.c_str());
        //}
        //else { //그렇지 않을 경우 퇴장에 대한 신호로 생각하여 퇴장 메시지 전송
        //    msg = "[공지] " + sck_list[idx].user + " 님이 퇴장했습니다.";
        //    pIdx = std::find(pctList.begin(), pctList.end(), sck_list[idx].user) - pctList.begin();
        //    pctList.erase(pctList.begin() + pIdx);
        //    insertPtcpt();

        //    cout << msg << endl;
        //    send_msg(msg.c_str());
        //    del_client(idx); // 클라이언트 삭제
        //    return;
        //}
    }
}

void del_client(int idx) {
    closesocket(sck_list[idx].sck);
    //sck_list.erase(sck_list.begin() + idx); // 배열에서 클라이언트를 삭제하게 될 경우 index가 달라지면서 런타임 오류 발생....ㅎ
    // client_count--;
}


// 참가자 DB insert
void insertPtcpt()
{
    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
    sql::Connection* con;

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

    // 참가자 목록 삭제 (중복방지)
    sql::PreparedStatement* delPstmt;
    delPstmt = con->prepareStatement("TRUNCATE TABLE participant");
    delPstmt->execute();
    delete delPstmt;

    // 참가자 insert
    for (int i = 0; i < pctList.size(); i++) {
        sql::PreparedStatement* pstmt;
        pstmt = con->prepareStatement("INSERT INTO participant(memberID) VALUES(?)");
        pstmt->setString(1, pctList[i]);
        pstmt->execute();
        delete pstmt;
    }

    delete con;
}


void insertMsgInfo(string msg)
{
    std::stringstream ss(msg);  // 문자열을 스트림화
    string stream1 = "", stream2 = "", stream3 = "", stream4 = "";
    string dmYN = "0";
    // 
    // 1: 송신자, 3 : /D , 4 : 수신자, 시간은 현재시간으로 넣기.
    ss >> stream1; // 첫 번째 단어
    ss >> stream2; // 두 번째 단어
    ss >> stream3; // 세 번째 단어
    ss >> stream4; // 네 번째 단어

    // 명령어는 '/D','/S'만 가능. 
    if (stream3 != "/d" && stream3 != "/s" && stream3 != "/f" && stream3 != "/F") {
        if (stream3 == "/D") {
            dmYN = "1";
        }
        else {
            stream4 = "";
        }

        // 공지는 memberID null.
        if (stream1 == "[공지]") {
            stream1 = "";
        }

        // MySQL Connector/C++ 초기화
        sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
        sql::Connection* con;
        sql::PreparedStatement* pstmt;
        sql::Statement* stmt;

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

        // 참가자 insert
        pstmt = con->prepareStatement("INSERT INTO chat(memberID, chatContent, chatDateTime, DM, receiverID) VALUES(?,?,date_format(now(), '%Y-%m-%d %H:%m:%s'), ?,?);");
        pstmt->setString(1, stream1);
        pstmt->setString(2, msg);
        pstmt->setString(3, dmYN);
        pstmt->setString(4, stream4);
        pstmt->execute();

        delete pstmt;
        delete con;
    }
}