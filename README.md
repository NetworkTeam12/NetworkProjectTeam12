# NetworkProjectTeam12
# 파일 구성 방법
1.
- client.cc, client.h, streamer.cc streamer.h : /src/applications/model/ directory 안에 넣어주시고
- helper.cc, helper.h : /src/applictions/helper/ directory 안에 넣어주시고
- mainfile.cc 는 /scratch/에 넣어주시고

2.
- /src/application/wscript 에서 module.source = [ 안에 'model/client.cc', 'model/server.cc', 'helper/helper.cc'] 추가해주시고
- headers.source = [ 안에는  'model/client.h', 'model/server.h', 'helper/helper.h'] 추가해주시면 됩니다

3.
- ns-3.29$ ./waf clean
- ns-3.29$ ./waf configure --build-profile=debug
- ns-3.29$ ./waf
- ns-3.29$ ./waf scratch/mainfile
- 하시면 됩니다 



# 옆에 본인 이름 적힌 항목 채워주시면 됩니다. 제출 시 지울 예정

## 1. Introduction 장지호
Project Subject: **Reliability & Congestion Control**

NS3를 통해 Reliability & Congestion Control을 직접 구현하고, 다양한 Network Environment & Topology 시나리오에 대해 시뮬레이션을 진행한다.

<br>

## 2. Environment 장지호
- OS: Ubuntu 18.04 LTS  
- Simulator: NS3.29 


<br>

## 3. Application Explanation
### 1) Introduction 장지호
Reliability & Congestion Control을 구현하기 위해 client application과 server application에 대한 코드와 헤더 파일을 각각 작성했고, 추가로 application 동작에 사용되는 helper 코드도 함께 작성했다. 또한 우리가 구현한 application을 기반으로 다양한 시나리오를 시뮬레이션할 수 있는 main.cc 코드를 구축해놓았다.

### 2) Code Tree 장지호
    ns3.29
    ├─ src
    │  └─ application
    │     ├─ model
    │     │  ├─ client.cc
    │     │  ├─ client.h
    │     │  ├─ server.cc
    │     │  └─ server.h
    │     │
    │     └─ helper
    │        ├─ helper.cc
    │        └─ helper.h
    │ 
    └─ scratch
	    └─ main.cc


### 3) Streaming Logic 조하영 이규민
#### A. Streamer
- Attribute : Remote, Port, PacketSize, StreamingFPS, LossEnable, LossRate, PacketNIP
- Additional Member Variable : m_socket, m_sendEvent, m_frameN, m_seqN
- Functions Flow
	1. Streamer (), ~ Streamer (), GetTypeId ()
	2. StartApplication ()
		- Udp Socket을 통해 Client와 bind 실행. 
		- Socket을 통해 Packet을 Receive 시 HandleRead()가 불리도록 Callback 연결
  		- 연결된 Client에게 Frame(Packets)을 보내기 위해 Send() 호출 
	3. Send ()
		- LossEnable이 켜져 있을 때, 랜덤 확률을 통해 Packet Loss 발생
		- 한 frame에 있는 packet들의 갯수만큼 Client에게 전송
		- Send() 함수를 정해진 FPS에 맞게 실행되도록 scheduling
	4. HandleRead (Ptr<Socket> socket)
		- Packet을 받을 때 불리는 Callback function
		- Packet Header제거 후, Seqence number에 맞는 packet을 resend하기 위해 Resend(seqN) 호출
	5. ReSend (uint32_t seqN)
		- 주어진 SeqN에 맞는 Packet을 생성
		- Client에게 재 전송 
	6. StopApplication ()
		- Socket 닫고 Callback 함수 초기화하여 Application 종료

#### B. Frame
- Member Variable : m_packets[1000]
- Functions : Frame (), ~Frame ();
	
#### C. Client
- Attribute : Remote, Port, PacketSize, BufferSize, LossEnable, LossRate, PacketNIP, PauseSize, ResumeSize
- Additional Member Variable : m_socket, m_local, m_consumEvent, m_frameN, m_seqN, m_frameBuffer, m_consumeN
- Functions Flow
	1. Client (), ~ Client (), GetTypeId () 
	2. StartApplication ()
		- Udp Socket을 통해 요청들어오는 Server와 bind 실행 
		- Socket을 통해 Packet을 Receive 시 HandleRead()가 불리도록 Callback 연결
		- FrameConsumer() 함수를 정해진 Event time에 맞게 실행되도록 scheduling
	3. HandleRead (Ptr<Socket> socket)
		- Packet을 받을 때 불리는 Callback function
		- Packet Header제거 후, Frame number, Sequence number 확인 후 PutFrameBuffer() 실행
	4. PutFrameBuffer (uint32_t frameN, uint32_t seqN)
		- 넘겨 받은 Frame number, Sequence number에 따라 packet을 Frame buffer에 추가하는 함수
		- 만약 Frame buffer 안에 해당하는 Frame이 이미 들어가 있다면, 그 Frame 안에 넘겨 받은 Seqence number에 따라 packet 추가
		- 만약 Frame buffer 안에 해당하는 Frame이 없다면, 해당 Frame을 만든 후 그 안에 넘겨 받은 Seqence number에 따라 packet 추가
		- 만약 Frame buffer가 가득 찼다면 Frame에 packet 추가 안 함
	5. FrameConsumer (void)
		- Frame buffer에 Frame이 있다면, 소비해야 하는 consume number에 해당하는 Frame 찾기
		- 소비해야 하는 Frame이 없다면, Frame 전체 packet을 Streamer에게 재요청
		- 소비해야 하는 Frame에 없는 Packet이 있다면, 해당 sequence를 packet에 붙인 후 Streamer에게 Send
		- 소비해야 하는 Frame에 모든 Packet이 있다면, Frame buffer에 Frame 제거
		- FrameConsumer() 함수를 정해진 Event time에 맞게 실행되도록 scheduling
	6. StopApplication ()
		- Socket 닫고 Callback 함수 초기화하여 Application 종료


### 4) Reliability of UDP 조하영
- Client가 FrameConsumer (void)에서 Frame을 소비할 때, 만약 없는 Frame/Packets이 있다면 Streamer에게 재요청한다. 
- 재요청하는 방법은 빠진 Packet의 Seqeunce number을 packet header에 추가한 후, Streamer에게 보내는 것이다.
- Streamer가 Client에게 이 Packet을 받으면 HandleRead()가 Callback으로 불리게 된다.
- HandleRead()에서 받은 Packet의 Header를 제거하여 Sequence number를 확인한 후, 다시 Packet을 보낸다. (이때는 Loss Enable 적용 안 함)
	
### 5) Congestion Control of UDP 이규민


### 6) Main Function & Network Topology 이원규, 강남구



<br>

## 4. Running the Application
### 1) DockerHub Link 장지호
결과물 docker hub link 올리기 
*메인 작성 후

### 2) Running the App 이원규 강남구 
```
$ ./waf scratch TeamProject 
```
등등 


### 3) Result 이원규 강남구 
결과물 -> 그래프


### 4) Video Link 장지호
최종 발표 비디오


<br>

## 5. Team Members & Roles
### 1) SubTeam 1.
- Streamer.h, Streamer.cc, Client.h, Client.cc, README.md
- 조하영 : Streaming Code의 전체적인 틀, Reliability of UDP, README.md 작성
- 이규민 : Streaming Code의 전체적인 틀, Congestion control of UDP, README.md 작성

### 2) SubTeam 2.
- StreamingHelper.cc StreamingHelper.h, README.md
- 이경돈 : Streaming Helper Code 전체, README.md 작성

### 3) SubTeam 3.
- Project.cc, README.md
- 이원규, 강남구 : Project Code 전체(Network topology 구상), 다양한 네트워크 환경과 노드 특성을 가지고 실험, README.md 작성

### 4) SubTeam 4.
- README.md, .ppt
- 장지호 : README.md 작성 및 , Presentation 제작, Team 발표

<br>

## 6. Timeline
- 5/6 : Project Meeting -> Project 주제 회의 (전체)
- 5/12 : Project Meeting -> Project 주제 확정 (전체)
- 5/16 : SubProject Meeting (SubTeam 1 + 이원규)
- 5/17~5/19 : Development of SubTeam 1
- 5/20 : Development of SubTeam 2
- 5/21 : Issue 해결 (이규민)
- 5/22 : Main Function & Topology Meeting of SubTeam 3
- 5/23 : Issue 해결 (조하영)
- 5/24 : Issue 해결 (이규민)
- 5/23~5/31 : Development of SubTeam 3
- 5/29~5/31 : Development of SubTeam 4
- 6/1 : Feedback & Submission 
