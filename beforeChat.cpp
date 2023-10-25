#define _CRT_SECURE_NO_WARNINGS 

#include <iostream>
#include <mysql/jdbc.h>
#include <string>
#include <sstream>
#include <ctime>
#include <Windows.h>

using namespace std;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1122"; // 데이터베이스 접속 비밀번호

//색상
enum color {
    BLACK = 0,
    DARK_BLUE,
    DARK_RED = 4,
    GRAY = 7,
    BLUE = 9,
    GREEN,
    SKYBLUE,
    RED,
    YELLOW = 14,
    WHITE
};


void textcolor(int foreground, int background);


// 이전 DM 조회 (전체일자 조회)
void getMyDM(string myId) {
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
    string sql = "SELECT memberID, chatContent, chatDateTime FROM chat WHERE receiverID ='" + myId + "' and DM = 1";
    res = stmt->executeQuery(sql);


    // 이전 DM 출력
    cout << endl;
    cout << "▽   이전 DM   ▽" << endl << endl;
    if (res) {
        string msg;
        string stream1, stream2, stream3, stream4, stream5;

        while (res->next()) {
            msg = res->getString("chatContent");
            std::stringstream ss(msg);  // 문자열을 스트림화

            ss >> stream1; // 첫 번째 단어
            ss >> stream2; // 두 번째 단어
            ss >> stream3; // 세 번째 단어
            ss >> stream4; // 네 번째 단어

            if (stream3 == "/D")
            {
                int eraseLength = 0;
                eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + 3;
                msg.erase(0, eraseLength);

                cout << "[" << res->getString("chatDateTime") << "] ";
                textcolor(BLACK, YELLOW);
                cout << stream1 << "의 귓속말 :";
                textcolor(GRAY, BLACK);
                cout << msg << endl;
            }
        }
        delete res;
        delete con;
        cout << endl << endl;

    }
    else {
        cout << "이전 DM이 없습니다." << endl;
    }
}


// 이전 대화내용 조회 (당일건만 조회)
void getBeforeChat(string myId) {
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
    string sql;
    sql = "SELECT memberID, chatContent, chatDateTime, DM, receiverID FROM chat WHERE DATE_FORMAT(chatDateTime, '%Y-%m-%d') = CURDATE() AND DM = 0";
    sql += " UNION DISTINCT SELECT memberID, chatContent, chatDateTime, DM, receiverID FROM chat WHERE DATE_FORMAT(chatDateTime, '%Y-%m-%d') = CURDATE() AND DM = 1 AND receiverID = '" + myId + "'";
    sql += " ORDER BY chatDateTime";
    res = stmt->executeQuery(sql);

    // 이전 대화 출력
    string msg, chatTime = "";
    string stream1, stream2, stream3, stream4, stream5;

    time_t timer;
    struct tm* t;
    timer = time(NULL); // 1970년 1월 1일 0시 0분 0초부터 시작하여 현재까지의 초
    t = localtime(&timer); // 포맷팅을 위해 구조체에 넣기


    cout << "\n   ▽ [";
    cout << t->tm_year + 1900 << "년 " << t->tm_mon + 1 << "월 " << t->tm_mday << "일 ] ";
    cout << "대화 내용 ▽" << endl;

    while (res->next()) {
        msg = res->getString("chatContent");
        chatTime = res->getString("chatDateTime");
        chatTime = chatTime.substr(11,5);
        std::stringstream ss(msg);  // 문자열을 스트림화

        ss >> stream1; // 첫 번째 단어
        ss >> stream2; // 두 번째 단어
        ss >> stream3; // 세 번째 단어
        ss >> stream4; // 네 번째 단어
        ss >> stream5; // 다섯 번째 단어

        if (stream3 == "/D")
        {
            int eraseLength = 0;
            eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + 3;
            msg.erase(0, eraseLength);

            cout << "[" << chatTime << "] ";
            textcolor(BLACK, YELLOW);
            cout << stream1 << "의 귓속말";
            textcolor(GRAY, BLACK);
            cout <<" :"<< msg << endl;
        }
        else if (stream3 == "/T" || stream3 == "/t") // 송신자 : /T 그룹이름 수신자 메세지
        {
            if (stream3 == "/T" && stream5 == myId)
            {
                int eraseLength = 0;
                eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + size(stream5) + 4;
                msg.erase(0, eraseLength);
                if (stream4 == "red")
                {
                    textcolor(RED, BLACK);
                }
                else if (stream4 == "green")
                {
                    textcolor(GREEN, BLACK);
                }
                else if (stream4 == "blue")
                {
                    textcolor(BLUE, BLACK);
                }
                else if (stream4 == "yellow")
                {
                    textcolor(YELLOW, BLACK);
                }
                cout << "[" << chatTime << "] ";
                cout << stream1 << "의 그룹 메세지 :" << msg << endl;
                textcolor(GRAY, BLACK);
            }

        }
        else // 명령어가 없을 때
        {
            cout << "[" << chatTime << "] ";
            cout << msg << endl;
        }
        chatTime = "";
    }

    cout << "\n   △ [";
    cout << t->tm_year + 1900 << "년 " << t->tm_mon + 1 << "월 " << t->tm_mday << "일 ] ";
    cout << "대화 내용 △" << endl;

    cout << endl;
    cout << endl;
    textcolor(DARK_BLUE, WHITE);
    cout << "################################################################################################";
    textcolor(GRAY, BLACK);
    cout << endl;
    textcolor(DARK_BLUE, WHITE);
    cout << "#                                                                                              #";
    textcolor(GRAY, BLACK);
    cout << endl;
    textcolor(DARK_BLUE, WHITE);
    cout << "#                                 §    채 팅 방    입 장    §                                  #";
    textcolor(GRAY, BLACK);
    cout << endl;
    textcolor(DARK_BLUE, WHITE);
    cout << "#   ※ 기타 기능 사용하기 ( '/d' : DM, '/f' : 친구 신청, ‘/t’ : 팀채팅, '/q' : 채팅방 퇴장)※    #";
    textcolor(GRAY, BLACK);
    cout << endl;
    textcolor(DARK_BLUE, WHITE);
    cout << "#                                                                                              #";
    textcolor(GRAY, BLACK);
    cout << endl;
    textcolor(DARK_BLUE, WHITE);
    cout << "################################################################################################";
    textcolor(GRAY, BLACK);
    cout << endl << endl;

    delete res;
    delete con;
}