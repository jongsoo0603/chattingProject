# 채팅 프로그램

---

# 👆🏻프로젝트 개요

<aside>
✅ 콘솔 창에서 여러 사람들과 대화할 수 있는 간단한 채팅 프로그램입니다.

- 주요 기능
    - 전체 채팅
    - 친구 추가(A 신청 → B 수락 → A, B 확인)
    - Direct Message(친구 간)
    - Group Message(그룹 간)
    - 회원가입 / 로그인(DB 활용)
- Window의 TCP 프로토콜인 소켓 통신 사용
- 채팅방은 1개의 서버와 1명 당 1개의 Thread로 운영
- MySQL로 DB 관리
</aside>

- 언어
    - C++
    - SQL

- 개발 환경
    - Windows
    - Visual Studio
    - MySQL
        - MySQL Workbench
        - MySQL C++ Connector
    - Git
        - Git Bash
        - Git Hub

- 개발 기간
    - 2023/10/10 ~ 2023/10/24

- 회고
    - 경험
        - Git 협업 → 하나의 Repository에서 각자의 Branch로 만 협업 진행
        - C++  ←→  DB 연동
        - 기능 별 파일 분리
    - 보완 사항
        - Class, Sturct 사용 → 객체지향적 개발
        - 채팅방 입장 전 Server와 Client 간 통신 문제 해결
            - Server에서 만 DB 접근 → DB 보안 향상

---

# 📺 시연 영상

[https://youtu.be/RCxbHagOcmU?si=b-Bus8Zuo3ZE7kxV](https://youtu.be/RCxbHagOcmU?si=b-Bus8Zuo3ZE7kxV)

---

# 📃 발표 PPT

[https://docs.google.com/presentation/d/1RdeuP0C05-BWHUzGlX5enSWm1gmgxYr8/edit?usp=share_link&ouid=114926827325419426334&rtpof=true&sd=true](https://docs.google.com/presentation/d/1RdeuP0C05-BWHUzGlX5enSWm1gmgxYr8/edit?usp=share_link&ouid=114926827325419426334&rtpof=true&sd=true)

---

# 🌍 깃허브

Fork 한 본인 Repository

[GitHub - jongsoo0603/chattingProject](https://github.com/jongsoo0603/chattingProject/tree/main)

- 협업 원본 Repository
    
    [GitHub - dayoungkwon-dev/chattingProject](https://github.com/dayoungkwon-dev/chattingProject)
    

---

# 📖 주요 코드

- C++에서 DB 연결 후 조건에 맞는 Data 출력
    
    ```cpp
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
    ```
    
- 채팅 받아와서 후처리 후 출력
    
    ```cpp
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
    ```
    
- 채팅방 연결 유지, 채팅 입력 시 전처리 후 서버에 전송
    
    ```cpp
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
    ```
