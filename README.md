# NetworkProjectTeam12

## 1. Introduction 장지호
- repository 소개
- 팀플 과제 설명 등등 

<br>

## 2. Environment 장지호
환경 소개 : ns-3 버전, docker 사용 등등


<br>

## 3. Team Members & Roles
### 1) SubTeam 1.
- Streamer.h, Streamer.cc, Client.h, Client.cc
- 조하영 : Streaming Code의 전체적인 틀, Reliability of UDP, README.md 작성
- 이규민 : Streaming Code의 전체적인 틀, Congestion control of UDP, README.md 작성

### 2) SubTeam 2.
- StreamingHelper.cc StreamingHelper.h
- 이경돈 : Streaming Helper Code 전체, README.md 작성

### 3) SubTeam 3.
- Project.cc
- 이원규, 강남구 : Project Code 전체(Network topology 구상), 다양한 네트워크 환경과 노드 특성을 가지고 실험, README.md 작성

### 4) SubTeam 4.
- 장지호 : README.md 작성, Presentation 제작, Team 발표

<br>

## 4. Application Explanation
### 1) Introduction 이규민


### 2) Streaming Logic
#### A. Streamer
- Attribute : Remote, Port, PacketSize, StreamingFPS, LossEnable, LossRate, PacketNIP
- Additional Member Variable : m_socket, m_sendEvent, m_frameN, m_seqN
- Function Flow
	1. Streamer (), GetTypeId () 
	
	2. StartApplication ()
  		- Udp Socket을 통해 Client와 bind 실행. 
  		  - 이때, Server Socket은 m_socket, Client의 Address, Port는 Remote, Por
  		- Socket을 통해 Packet을 Receive 시 HandleRead()가 불리도록 Callback 연결
  		- 연결된 Client에게 Frame(Packets)을 보내기 위해 Send() 호출 
  
	3. Send ()
			- LossEnable이 켜져 있을 때, 랜덤 확률을 통해 Packet Loss 발생
				- 이때, Packet Loss rate는 LossRate
			- 한 frame에 있는 packet들의 갯수만큼 Client에게 전송
				- 이때, 한 frame에 있는 packet들의 갯수는 PacketNIP
			- Send() 함수를 정해진 FPS에 맞게 실행되도록 scheduling
				- 이때, 정해진 FPS는 StreamingFPS이며, Scheduling event는 m_sendEvent



#### B. Client


### 3) Reliability of UDP
조하영

### 4) Congestion Control of UDP 이규민


### 5) Main Function & Network Topology 이원규, 강남구



<br>

## 5. Running the Application
### 1) DockerHub Link 장지호
결과물 docker hub link 올리기 


### 2) Running the App 이원규 강남구 
```
$ ./waf scratch TeamProject 
```
등등 


### 3) Result 이원규 강남구 
결과물 -> 그래프


### 4) Video Link 장지호
최종 발표 비디오



