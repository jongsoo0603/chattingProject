#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

using namespace std;

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
vector<vector<string>> getPtcpt(string myId);
vector<string> useSpeaker(string myId);

// DM 기능 입력부
string inputDM(string myId)
{
    vector<vector<string>> pList;
    int end = 0;
    string newFrdNum, message;
    pList = getPtcpt(myId);
    while (end == 0)
    {
        std::cout << "DM 할 사람 id 입력 : ";
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
                    return "";
                    break;
                }
                else if (newFriend[2] == "Y")
                {
                    cout << "보낼 메세지 입력 : ";
                    getline(cin, message);
                    getline(cin, message);
                    return  "/D " + newFriend[1] + " " + message; // 송신자 : /D 수신자 메세지
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


// DM 기능 출력부
void outputDM(string stream1, string stream2, string stream3, string stream4, string msg, string myId)
{
    if (stream3 == "/D" && stream4 == myId)
    {
        int eraseLength = 0;
        eraseLength = size(stream1) + size(stream2) + size(stream3) + size(stream4) + 3;
        msg.erase(0, eraseLength);
        cout << "                                                            ";
        textcolor(BLACK, YELLOW); 
        cout << stream1 << "의 귓속말";
        textcolor(GRAY, BLACK);
        cout << " :" << msg << endl;
    }
}


// 친구추가 기능 입력부
string inputFriend(string myId)
{
    vector<vector<string>> pList;
    int end = 0;
    string newFrdNum;
    pList = getPtcpt(myId);
    while (end == 0)
    {
        cout << "친구신청 할 사람 id 입력 : ";
        cin >> newFrdNum;
        string temp;
        getline(cin, temp);
        vector<string> newFriend;
        for (int i = 1; i < pList.size() + 1; i++)
        {
            newFriend = pList[i - 1];
            if (newFrdNum == newFriend[1])
            {
                if (newFriend[2] == "N")
                {
                    return "/F " + newFriend[1]; // 송신자 : /F 수신자
                    end = 1;
                    break;
                }
                else if (newFriend[2] == "Y")
                {
                    cout << "이미 친구입니다." << endl;
                    end = 1;
                    return "";
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


// 친구추가 기능 출력부
tuple<string, string, int> outputFriend(string stream1, string stream3, string stream4, string myId)
{
    if (stream3 == "/F" && stream4 == myId)
    {
        cout << "                                                            " << "ID '" << stream1 << "'이(가) 친구 요청을 보냈습니다. 수락하시겠습니까?(Y, N)\n :";
        return { stream1 , stream4, 1 };
    }
    return { "" , "", 0 };
}


// 확성기 기능 입력부
void inputSpeaker(string myId, SOCKET client_sock)
{
    vector<string> groupInfo;
    string groupName, message, text;
    int gSize = groupInfo.size();
    groupInfo = useSpeaker(myId);

    groupName = groupInfo[0];
    cout << "[ ";
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
    textcolor(GRAY, BLACK);
    cout << " ] 팀에게 보낼 메세지 입력 : ";
    getline(cin, message);

    for (int i = 1; i < groupInfo.size(); i++)
    {
        text = "/T " + groupName + " " + groupInfo.at(i) + " " + message; // 송신자 : /T 그룹이름 수신자 메세지
        const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
        send(client_sock, buffer, strlen(buffer), 0); // 보내기
    }
}


// 확성기 기능 출력부 (송신자 : /T 그룹이름 수신자 메세지)
void outputSpeaker(string stream1, string stream2, string stream3, string stream4, string stream5, string msg, string myId)
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
        cout << "                                                            " << stream1 << "의 그룹 메세지 :" << msg << endl;
        textcolor(GRAY, BLACK);
    }
}