# NetworkProjectTeam12

## 1. Introduction 장지호
Project Subject: **Reliability & Congestion Control Streaming**
- NS3를 통해 UDP 기반 Reliability & Congestion Control을 직접 구현하고, 
- Streaming Application을 통해
- 다양한 Network Environment & Topology 시나리오에 대해 시뮬레이션을 진행한다.

<br>

## 2. Environment 장지호
- OS : Ubuntu 18.04 LTS
- Simulato r: ns-3.29

<br>

## 3. Application Explanation
### 1) Introduction 장지호
- Reliability & Congestion Control을 구현하기 위해 client application과 server application에 대한 코드와 헤더 파일을 각각 작성했고, 
- 추가로 application 동작에 사용되는 helper 코드도 함께 작성했다. 
- 또한 우리가 구현한 application을 기반으로 다양한 시나리오를 시뮬레이션할 수 있는 main.cc 코드를 구축해놓았다.

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
- Attribute : Remote, Port, PacketSize, StreamingFPS, LossEnable, LossRate, PacketNIP, Mode, threshold
- Additional Member Variable : m_socket, m_sendEvent, m_congestionEvent, m_frameN, m_seqN, m_mode, m_slowstart, m_threshold, m_flag;
- Functions Flow
	1. Streamer (), ~ Streamer (), GetTypeId ()
	2. StartApplication ()
		- Udp Socket을 통해 Client와 bind 실행. 
		- Socket을 통해 Packet을 Receive 시 HandleRead()가 불리도록 Callback 연결
  		- 연결된 Client에게 Frame(Packets)을 보내기 위해 Send() 호출 
	3. Send ()
		- LossEnable이 켜져 있을 때, 랜덤 확률을 통해 Packet Loss 발생
		- 한 frame에 있는 packet들의 갯수 즉, m_packetNIP만큼 Client에게 전송
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
- Member Variable : m_packets[1000], m_send[1000]
- Functions : Frame (), ~Frame ()
	
#### C. Client
- Attribute : Remote, Port, PacketSize, BufferSize, LossEnable, PacketNIP
- Additional Member Variable : m_socket, m_local, m_consumEvent, m_frameN, m_seqN, m_frameBuffer, m_consumeN, m_sendN
- Functions Flow
	1. Client (), ~ Client (), GetTypeId () 
	2. StartApplication ()
		- Udp Socket을 통해 요청들어오는 Server와 bind 실행 
		- Socket을 통해 Packet을 Receive 시 HandleRead()가 불리도록 Callback 연결
	3. HandleRead (Ptr\<Socket> socket)
		- Packet을 받을 때 불리는 Callback function
		- Packet Header제거 후, Frame number, Sequence number 확인 후 PutFrameBuffer() 실행
	4. PutFrameBuffer (uint32_t frameN, uint32_t seqN)
		- 넘겨 받은 Frame number, Sequence number에 따라 packet을 Frame buffer에 추가하는 함수
		- 만약 Frame buffer 안에 해당하는 Frame이 이미 들어가 있다면, 그 Frame 안에 넘겨 받은 Seqence number에 따라 packet 추가
		- 만약 Frame buffer 안에 해당하는 Frame이 없다면, 해당 Frame을 만든 후 그 안에 넘겨 받은 Seqence number에 따라 packet 추가
		- 만약 Frame buffer가 가득 찼다면 Frame에 packet 추가 안 함
		- 만약 Packet의 Frame number가 SendCheck를 하지 않았던 Frame이라면 SendCheck()로 실행
		- 만약 Packet의 Frame number가 소비했던 Frame Number보다 크다면 FrameConsumer()로 실행
	5.  SendCheck(Address from, Ptr\<Socket> socket)
		- Frame buffer에 있는 packet들이 온전히 다 왔는지 확인하고, 
		- 안 온 packet이 있다면 resend를 요청하는 함수
	6. FrameConsumer (void)
		- 소비해야 하는 Frame이 resend를 요청한 상황이라면 break
		- 소비해야 하는 Frame이 resend를 요청한 것이 없다면, framebuffer에서 frame을 지우면서 소비한다.
	7. StopApplication ()
		- Socket 닫고 Callback 함수 초기화하여 Application 종료


### 4) Reliability of UDP 조하영
- Client가 HandleRead()-> PutFrameBuffer() 거치면서 SendCheck()에서 만약 없는 Frame/Packets이 있다면 Streamer에게 재요청한다. 
- 재요청하는 방법은 빠진 Packet의 Sequence number를 packet header에 추가한 후, Streamer에게 보내는 것이다.
- Streamer가 Client에게 이 Packet을 받으면 HandleRead()가 Callback으로 불리게 된다.
- HandleRead()에서 받은 Packet의 Header를 제거하여 Sequence number를 확인한 후, 다시 Packet을 보낸다. (이때는 Loss Enable 적용 안 함)

### 5) Congestion Control of UDP 이규민
- Streamer가 Flowcontrol()를 이용해서, Fps를 조절해준다. Flowcontrol()에서 control mode를 선택할 수 있는데 AIMD, Slow Start가 있다.
- AIMD의 경우 1 Frame을 보내면 Fps가 1 증가하고, Client가 resend를 요청하면 Fps를 반으로 한다. 단 최저값 이하로는 내려가지 않는다.
- Slow Start의 경우 threshold에 도달하지 않으면 1 Frame을 보내면 Fps가 2배로 증가하고, threshold에 도달하면 1 Frame을 보내면 Fps가 1 증가한다.
  Client가 resend를 요청했고 slowstart 중이라면, resend 요청이 seqN 보다 3 낮으면, threshold를 줄이고 Fps를 최저값으로 변경한다. 만약 resend 요청이 
  seqN 보다 3 낮지 않으면 Fps를 최저값으로 변경하고 끝낸다.
  slowstart가 아니라면, threshold를 줄이고 Fps를 최저값으로 변경한다.

### 6) Main Function & Network Topology 이원규, 강남구
#### CMD Arguments 설명
##### 1. Arguments for  Streamer
- PacketSize : 패킷 사이즈
- PackletNip : 프레임 당 패킷 개수
- Fps : 스트리밍 FPS
- LossEn : Forced Packet Loss on/off
- LossRate : Loss probability
- Mode : Select congestion control mode
- ThresHold : Select threshold


##### 2. Arguments for Client
- PacketSize : 패킷 사이즈
- PackletNip : 프레임 당 패킷 개수
- BufferSize : The frame buffer size
	
##### 3. Arguments for OnOffAppl (only for subflow,  Only when subflow exists)
- protocol : protocol of sub flow if Tcp  type true if Udp type false (boolean type)
- ontime : subflow's ontime
- offtime : subflow's offtime

#####    Topol 1.

        N1 - N2    p2p

#####    Topol 2.

	 Wifi 192.168.1.0
   			
          AP
        *    *
        |    |
        n1   n2
               


#####    Topol 3.
	
        N1                 
           \                
              N3- N4     N1-N4 Streamer& Client  N2-N4: UDP or TCP 
           /                
        N2   

#####    Topol 4.
        
        - - csma  - -       N1-N2: OnOff&sink
        |   |   |   |	    N3-N4: Streamer&Client
        N1  N2  N3  N4
    

<br>

## 4. Running the Application
### 1) DockerHub Link 
#### docker hub link
https://hub.docker.com/r/skarn2158/netproj-team12/tags
#### docker pull command
docker pull skarn2158/netproj-team12:base

### 2) Running the App 이원규 강남구 
- /src/applications/model/ : client.cc, client.h, streamer.cc streamer.h
- /src/applictions/helper/ : helper.cc, helper.h
- /scratch/ : topology main files

- /src/application/wscript 에서
- module.source = [ 'model/client.cc', 'model/streamer.cc', 'helper/helper.cc'] 추가
- headers.source = [ 'model/client.h', 'model/streamer.h', 'helper/helper.h'] 추가

```
ns-3.29$ ./waf clean
ns-3.29$ ./waf configure --build-profile=debug
ns-3.29$ ./waf
ns-3.29$ ./waf --run scratch/topology1_p2p_pair
ns-3.29$ ./waf --run scratch/topology2_wifi
ns-3.29$ ./waf --run scratch/topology3_Y
ns-3.29$ ./waf --run scratch/topology4_csma
	
```


### 3) Result 이원규 강남구 
### sequence number & resend count test(Flow1: seq num, Flow2: resend count)
base parameter: packet size = 30, packet nip = 30, mode 0, buffer size = 40 

> #### packet nip = 10, 30, 100
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171214248-19eda640-75c5-41e4-b832-dec8ad18bbdd.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171215417-e63c0fd3-758f-43d1-b109-3d99e1512f36.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171215550-a70c5495-8b78-48b5-b818-0eb125f072c7.png" width="250" height="250"/>
</p>
	
> #### packet size = 10, 30, 100
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171215782-43a27547-00e3-4003-a89e-9e1e7f8d9d0e.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171215417-e63c0fd3-758f-43d1-b109-3d99e1512f36.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171215817-c252ff22-3faf-4670-965c-20c7172bccf1.png" width="250" height="250"/>
</p>
	
> #### mode 0, 1
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171215417-e63c0fd3-758f-43d1-b109-3d99e1512f36.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171216066-9f0c7878-4535-44b5-86d0-47b5b31aa3ea.png" width="250" height="250"/>
</p>
	
> #### threshold 50, 500
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171216455-dd42ad40-fc9f-4854-b4e0-ed0b1c1de368.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171216525-1981cef7-ed68-4f05-a8c1-363662536846.png" width="250" height="250"/>
</p>
	
> #### lossRate 0.01, 0.1
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171214248-19eda640-75c5-41e4-b832-dec8ad18bbdd.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171218804-2ae563fb-6a32-4129-8667-ecca46f75968.png" width="250" height="250"/>
</p>
	
### FPS test
#### parameter(삭제예정)
uint32_t fps = 30; 			
uint32_t packetSize = 100; 
uint32_t packetNip = 100; 
bool lossEnable = true;	
double lossRate = 0.01;	 	
uint32_t mode = 1; 		
uint32_t thresHold = 200; 	
uint32_t bufferSize = 40; 	
	
##### mode 0 fps 10, 20, 30
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171199940-ed9563a1-4555-43cd-abb4-e5cab3accaa2.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171200357-6f95a087-7423-4fdf-9f94-5fbc48f7f177.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171201119-f89b849c-8210-44df-83a8-ed02fa852221.png" width="250" height="250"/>
</p>
<br>
	
##### mode 1 fps 10, 20, 30
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171201270-62bf41a3-2f62-4042-9f7d-2b4e822d13d1.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171201428-f741b8e6-01b3-4038-a8de-cea969bcbc38.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171201633-1be494b5-5f84-4646-8709-ed0e4e3963c9.png" width="250" height="250"/>
</p>

##### loss 0.01, 0.1 (mode 1 fps 30)
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171201633-1be494b5-5f84-4646-8709-ed0e4e3963c9.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171209642-813d9980-f512-451a-b686-147270d9084a.png" width="250" height="250"/>
</p>
	
##### buf 40, buf 100 (mode 1 fps 30)
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171215417-e63c0fd3-758f-43d1-b109-3d99e1512f36.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171209812-fe0a8f59-0a1e-4818-90cb-d9979b3a0491.png" width="250" height="250"/>
</p>
	
#### 	retry 
	base >>
	uint32_t fps = 30; 			//30
	uint32_t packetSize = 100; 	//30
	uint32_t packetNip = 100; 	//30
	bool lossEnable = false;	//false
	double lossRate = 30;	 	//0.01
	uint32_t mode = 1; 			//0
	uint32_t thresHold = 200; 	//200
	uint32_t bufferSize = 40; 	//40	
	
	no loss, loss 0.1, 0.3
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171259482-bb9c2ac1-bfe3-4435-8980-22fdba7c4c3c.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171259498-48e87e44-fa9d-4a2f-9477-e9e503776e0c.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171259726-a226b40c-43a3-439a-99f4-16f191a883a9.png" width="250" height="250"/>
</p>
	
## Topology2 ( WIFI )
##### base( 	uint32_t fps = 30; 
	uint32_t packetSize = 100; 
	uint32_t packetNip = 100; 
	bool lossEnable = false;
	double lossRate = 10;	 
	uint32_t mode = 0; 	
	uint32_t thresHold = 200; 
	uint32_t bufferSize = 40; )<br>
	loss 0.1 mode 0
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171256552-f8a202aa-3b49-4bff-ad97-c4c829fd53b3.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171209812-fe0a8f59-0a1e-4818-90cb-d9979b3a0491.png" width="250" height="250"/>
</p>
	
##### loss 0.1, 0.3, 0.5 (mode 1)
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171255557-f8f2697e-c239-4651-aa6b-804e798e88db.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171255704-944b301a-c8fd-4491-b188-f16b0f3c827b.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171255829-6a3c5368-f724-4e5c-902a-1bbb5066be61.png" width="250" height="250"/>
</p>	
	
##### loss 0.3 mode 1 threshold 100, 200
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171255955-e94b5c79-5351-4982-af8e-329963c0be2f.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171255704-944b301a-c8fd-4491-b188-f16b0f3c827b.png" width="250" height="250"/>
</p>	
	
##### loss 0.5 mode 1 threshold 200 bufsize 20, 40
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171256199-d67b2cfe-0b01-423f-914b-b9fd28452238.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171255829-6a3c5368-f724-4e5c-902a-1bbb5066be61.png" width="250" height="250"/>
</p>	


## Topology3 ( Y )
##### udp 1-0, 0-1, 1-1 (no loss)
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171266111-f1426a26-98ed-4edb-9450-dcbc71a6a807.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171266095-6bb01e6e-4618-4d42-9c2e-899857389b7b.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171266138-3801fa2c-5f11-4d51-98af-2aef4749a5d0.png" width="250" height="250"/>
</p>
	
##### udp 1-0, 0-1, 1-1 (loss 0.3)
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171266381-a1a63406-c509-4707-9487-5fbbfdad187a.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171266407-267bdfec-a6f6-4d3b-8374-b3cf010ba7cf.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171266436-39077678-af76-42e0-aea4-008464779105.png" width="250" height="250"/>
</p>	

##### tcp 1-0/ no loss, loss 0.3
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171265994-54ca724c-c716-45e6-b667-8dfd57af7f47.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171266021-50a20e1e-c262-445d-b247-0768ab236386.png" width="250" height="250"/>
</p>	

## Topology4 ( CSMA )
##### tcp 1-1, udp 1-1/ no loss
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171275440-2e24ffd0-143d-4efd-829a-7a9e878b98e9.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171275461-260d2fbb-f260-484f-9062-fc3b29637966.png" width="250" height="250"/>
</p>	

##### tcp 1-1, udp 1-1/ loss 0.3
<p float="left">
	<img src="https://user-images.githubusercontent.com/28288186/171275488-9f8513f6-610f-4a0e-acea-fd1912ee54ac.png" width="250" height="250"/>
	<img src="https://user-images.githubusercontent.com/28288186/171275522-f8715714-5ea5-4e5d-80fc-98ef3706650c.png" width="250" height="250"/>
</p>	


### 4) Video Link 장지호
최종 발표 비디오


<br>

## 5. Team Members & Roles
### 1) SubTeam 1.
- Streamer.h, Streamer.cc, Client.h, Client.cc, README.md
- 조하영 : Streaming Code의 전체적인 틀, Reliability of UDP, README.md 작성
- 이규민 : Streaming Code의 전체적인 틀, Congestion control of UDP, README.md 작성

### 2) SubTeam 2.
- StreamingHelper.cc StreamingHelper.h
- 이경돈 : Streaming Helper Code 전체

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
- 5/31~6/1 : Project Meeting (SubTeam 3)
- 5/29~6/1 : Development of SubTeam 4
- 6/1 : Feedback & Submission 
