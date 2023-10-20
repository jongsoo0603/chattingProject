#define _CRT_SECURE_NO_WARNINGS 

#include <iostream>
#include <mysql/jdbc.h>
#include <string>
#include <sstream>
#include <ctime>
#include <Windows.h>

using namespace std;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "1122"; // �����ͺ��̽� ���� ��й�ȣ

//����
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
    WHITE
};


void textcolor(int foreground, int background);


// ���� DM ��ȸ (��ü���� ��ȸ)
void getMyDM(string myId) {
    // MySQL Connector/C++ �ʱ�ȭ
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

    // �����ͺ��̽� ����
    con->setSchema("chattingproject");

    // db �ѱ� ������ ���� ���� 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // �����ͺ��̽� ���� ����
    stmt = con->createStatement();
    string sql = "SELECT memberID, chatContent, chatDateTime FROM chat WHERE receiverID ='" + myId + "' and DM = 1";
    res = stmt->executeQuery(sql);


    // ���� DM ���
    cout << "�� ���� DM ��" << endl;
    if (res) {
        string msg;
        string stream1, stream2, stream3, stream4, stream5;

        while (res->next()) {
            msg = res->getString("chatContent");
            std::stringstream ss(msg);  // ���ڿ��� ��Ʈ��ȭ

            ss >> stream1; // ù ��° �ܾ�
            ss >> stream2; // �� ��° �ܾ�
            ss >> stream3; // �� ��° �ܾ�
            ss >> stream4; // �� ��° �ܾ�

            if (stream3 == "/D")
            {
                int eraseLength = 0;
                eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + 3;
                msg.erase(0, eraseLength);

                cout << "[" << res->getString("chatDateTime") << " ] ";
                cout << stream1 << "�� �ӼӸ� :" << msg << endl;
            }
        }
        delete res;
        delete con;


    }
    else {
        cout << "���� DM�� �����ϴ�." << endl;
    }
}


// ���� ��ȭ���� ��ȸ (���ϰǸ� ��ȸ)
void getBeforeChat(string myId) {
    // MySQL Connector/C++ �ʱ�ȭ
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

    // �����ͺ��̽� ����
    con->setSchema("chattingproject");

    // db �ѱ� ������ ���� ���� 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // �����ͺ��̽� ���� ����
    stmt = con->createStatement();
    string sql;
    sql = "SELECT memberID, chatContent, chatDateTime, DM, receiverID FROM chat WHERE DATE_FORMAT(chatDateTime, '%Y-%m-%d') = CURDATE() AND DM = 0";
    sql += " UNION DISTINCT SELECT memberID, chatContent, chatDateTime, DM, receiverID FROM chat WHERE DATE_FORMAT(chatDateTime, '%Y-%m-%d') = CURDATE() AND DM = 1 AND receiverID = '" + myId + "'";
    res = stmt->executeQuery(sql);

    // ���� ��ȭ ���
    string msg, chatTime = "";
    string stream1, stream2, stream3, stream4, stream5;

    time_t timer;
    struct tm* t;
    timer = time(NULL); // 1970�� 1�� 1�� 0�� 0�� 0�ʺ��� �����Ͽ� ��������� ��
    t = localtime(&timer); // �������� ���� ����ü�� �ֱ�


    cout << "\n   �� [";
    cout << t->tm_year + 1900 << "�� " << t->tm_mon + 1 << "�� " << t->tm_mday << "�� ] ";
    cout << "��ȭ ���� ��" << endl;

    while (res->next()) {
        msg = res->getString("chatContent");
        chatTime = res->getString("chatDateTime");
        chatTime = chatTime.substr(11,5);
        std::stringstream ss(msg);  // ���ڿ��� ��Ʈ��ȭ

        ss >> stream1; // ù ��° �ܾ�
        ss >> stream2; // �� ��° �ܾ�
        ss >> stream3; // �� ��° �ܾ�
        ss >> stream4; // �� ��° �ܾ�
        ss >> stream5; // �ټ� ��° �ܾ�

        if (stream3 == "/D")
        {
            int eraseLength = 0;
            eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + 3;
            msg.erase(0, eraseLength);

            cout << "[" << chatTime << "] ";
            textcolor(BLACK, YELLOW);
            cout << stream1 << "�� �ӼӸ�";
            textcolor(GRAY, BLACK);
            cout <<" :"<< msg << endl;
        }
        else if (stream3 == "/S" || stream3 == "/s") // �۽��� : /S �׷��̸� ������ �޼���
        {
            if (stream3 == "/S" && stream5 == myId)
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
                cout << stream1 << "�� �׷� �޼��� :" << msg << endl;
                textcolor(GRAY, BLACK);
            }

        }
        else // ���ɾ ���� ��
        {
            cout << "[" << chatTime << "] ";
            cout << msg << endl;
        }
        chatTime = "";
    }
    cout << endl;

    delete res;
    delete con;
}