#include <iostream>
#include <mysql/jdbc.h>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1122"; // 데이터베이스 접속 비밀번호



// 참가자 목록 출력 (전체)
vector<vector<string>> getPtcpt(string myId) {
    int i = 0;
    string no, friendYN = "N";
    vector<vector<string>> pList;
    vector<string> p1;
    vector<string> fList;

    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;
    sql::Statement* pStmt;
    sql::Statement* fStmt;
    sql::ResultSet* pRes;
    sql::ResultSet* fRes;

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

    // 참가자 목록 select.
    pStmt = con->createStatement();
    pRes = pStmt->executeQuery("SELECT memberID FROM participant WHERE NOT memberID ='" + myId + "'");
    delete pStmt;


    // 참가자 ID pList에 넣기.
    while (pRes->next()) {
        no = to_string(i + 1);
        p1.push_back(no);                           // 참가자 번호
        p1.push_back(pRes->getString("memberID"));  // 참가자 ID

        pList.push_back(p1);
        i++;
        p1.clear();
    }


    // 친구 목록 select.
    fStmt = con->createStatement();
    string sql = "SELECT friendList FROM member WHERE memberID ='" + myId + "'";
    fRes = fStmt->executeQuery(sql);
    delete fStmt;

    // 친구목록 행으로 나열.
    while (fRes->next()) {
        istringstream ss1(fRes->getString("friendList"));
        string buffer;
        while (getline(ss1, buffer, ',')) {
            fList.push_back(buffer);
        }
    }

    // 친구여부 판별하여 friendYN 값 넣기.
    for (int i = 0; i < pList.size(); i++) {
        for (int j = 0; j < fList.size(); j++) {
            if (pList[i][1] == fList[j]) {
                friendYN = "Y";
            }
        }
        pList.at(i).push_back(friendYN);   // 참가자와 로그인ID와 친구여부(Y/N)
        friendYN = "N";
    }
    delete fRes;
    delete pRes;
    delete con;


    // 참가자 목록 출력.
    cout << "\n ▷ 참가자 목록 ◁" << endl;
    for (int j = 0; j < pList.size(); j++) {
        cout << pList.at(j).at(0) << ". ";
        cout << pList.at(j).at(1);
        cout << " - 친구여부 : " << pList.at(j).at(2) << endl;
    }
    return pList;
}


vector<string> useSpeaker(string myId) {
    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;
    sql::Statement* stmt;
    sql::ResultSet* res;
    vector<string> groupInfo;
    string groupName;

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

    // 같은 그룹 ID 조회
    stmt = con->createStatement();
    res = stmt->executeQuery("SELECT memberID, groupName FROM member WHERE groupName = (SELECT groupName FROM member WHERE memberID ='" + myId + "')");
    delete stmt;

    while (res->next()) {
        groupName = (res->getString("groupName"));
        if (groupInfo.empty()) {
            groupInfo.push_back(groupName);
        }
        groupInfo.push_back(res->getString("memberID"));
    }

    return groupInfo;
}