/// board type
#define FEATHER_M0  1
#define ITTH_SA001	1 - FEATHER_M0
#if (FEATHER_M0 | ITTH_SA001)
	#warning "You need to check the type of board to be used and comment this line"
#endif

#define _DEBUG_     1 

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiMDNSResponder.h>
#if FEATHER_M0
  #include <FlashStorage.h>
#endif
#include "OneM2MClient.h"
#include <DHT.h>
#include <Wire.h>

/*************************************************************************/
/******************************		LCD		******************************/
#define Use_SSD1306		0
#define Use_SH1106G		0
#if (Use_SH1106G | Use_SSD1306)
	#warning "You need to check the type of LCD to be used and comment this line"
#endif

#if Use_SSD1306
	#include <Adafruit_SSD1306.h>
	// adafruit 1.3" OLED 128x64 I2C
	#define SCREEN_WIDTH  128 // OLED display width, HIGH pixels
	#define SCREEN_HEIGHT  64 // OLED display height, HIGH pixels

	// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
	// The pins for I2C are defined by the Wire-library. 
	// HIGH an arduino UNO:       A4(SDA), A5(SCL)
	// HIGH an arduino MEGA 2560: 20(SDA), 21(SCL)
	// HIGH an arduino LEONARDO:   2(SDA),  3(SCL), ...
	#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
	#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
	Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#elif Use_SH1106G
	#include <Adafruit_GFX.h>
	#include <Adafruit_SH110X.h>

	#define WHITE		SH110X_WHITE

	/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
	#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
	//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

	#define SCREEN_WIDTH 128 // OLED display width, HIGH pixels
	#define SCREEN_HEIGHT 64 // OLED display height, HIGH pixels
	#define OLED_RESET -1   //   QT-PY / XIAO
	Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif


#if (Use_SH1106G | Use_SSD1306)
	#include <qrcode.h>
	QRCode qrcode;
	const char* MESSAGE_OPEN_WEBAPP[4] = { "Scan QR", "to open", "Lumifera", "webapp " };
#endif
/******************************		LCD		******************************/
/*************************************************************************/


#define UseFlash		1
#define UseE2p			0
#if	UseE2p
	#define	Addr_24LC32AT		0x50
	
	#include "Adafruit_EEPROM_I2C.h"
	#include "Adafruit_FRAM_I2C.h"
	
	Adafruit_EEPROM_I2C i2ceeprom;
	#define EEPROM_ADDR 0x50  // the default address!
#endif

#define ledPin			13
#define ledDipTest		11

/******************************		WEB	******************************/
typedef enum eWEB_MODE_tag  {
    eWeb_Mode_User = 0
  , eWeb_Mode_Ae
  , eWeb_Mode_Ces
}eWEB_MODE;

/******************************		WIFI	******************************/
typedef enum eWIFI_STAT_tag {
		eWiFi_Stat_Init		= 0
	,	eWiFi_Stat_TryToConn
	,	eWiFi_Stat_Connected
#if 0
	,	eWiFi_Stat_ReConnect
#endif
}eWIFI_STAT;

typedef enum eWIFI_MODE_tag {
		eWiFi_Mode_Ap	= 0         //Access Point Mode(AP): 다른 장치가 접속(Access)할 수 있는 곳(Point)
	,	eWiFi_Mode_WebServer
	,	eWiFi_Mode_Sa             //STAtion Mode(STA): 공유기(AP)에 접속할 수 있게, AP에서 뿌리는 데이터 신호들을 받을 수 있는 모드
}eWIFI_MODE;

typedef struct stWIFI_tag {
	eWIFI_STAT	eStat	: 4;
	eWIFI_MODE	eMode	: 4;
}stWIFI;

/******************************		MQTT	******************************/
typedef enum eMQTT_STAT_tag {
		eMqtt_Stat_Init	= 0
	,	eMqtt_Stat_TryToConn
	,	eMqtt_Stat_Connected
	
	,	eMqtt_Stat_Reconnect
	,	eMqtt_Stat_Ready
	,	eMqtt_Stat_Idle
}eMQTT_STAT;

typedef struct stMQTT_tag {
	eMQTT_STAT		eStat	: 3;
	unsigned char	nRetry	: 4;
	unsigned char			: 1;

	char			aId[16];
}stMQTT;

/******************************		QUEUE	******************************/
#define QueueSize		8
typedef struct stQUEUE_BASE_tag {
	unsigned char	PopIdx;
	unsigned char	PushIdx;
	String			sRef[QueueSize];
	String			sCon[QueueSize];
	String			sRqi[QueueSize];
//	unsigned long*	previousMillis[QueueSize];
}stQUEUE_BASE;

typedef struct stNOTICE_tag {
	stQUEUE_BASE	stQue;
	String			sRef[8];
	int				nRefLen;
}stNOTICE;

typedef enum eUPLOAD_STAT_tag {
		eUploaded	= 0
	,	eUploading
}eUPLOAD_STAT;

typedef struct stUPLOAD_tag {
	eUPLOAD_STAT	eStat		: 4;
	unsigned char	nRetryCnt	: 2;
	unsigned char	bSetFlag	: 1;
	unsigned char	bDisplay	: 1;
	stQUEUE_BASE	stQue;
}stUPLOAD;

typedef struct stPV_tag {
	float	fPv;
	float	fAdj;
}stPV;

typedef struct stNETWORK_tag {
	char	aSsid[M2M_MAX_SSID_LEN];
	char	aPass[17];
}stNETWORK;

#define NetWorkCnt		1
typedef struct stFLASH_tag {
	stNETWORK		astNet   [NetWorkCnt];
	char			aAeName  [20];
	char			aCseId   [10];
	char			aCbName  [10];	// Configure CSEBase resource name
	char			aBrokerIp[16];
	unsigned short	usBrokerPort;
}stFLASH;

#define TopicSize	48
typedef struct stTOPIC_tag {
	char	aResp[TopicSize];
	char	aNoti[TopicSize];
}stTOPIC;

typedef enum eNCube_State_tag {
		eNCube_Stat_Req	= 0
	,	eNCube_Stat_Ing
}eNCube_Stat;

typedef enum eNCube_Mode_tag {
		eCreate_Ae		= 0
	,	eCreate_Cnt
	,	eDelete_Sub
	,	eCreate_Sub
	,	eCreate_Cin
}eNCube_Mode;

typedef struct stNCube_tag {
	union {
		struct {
			eNCube_Stat		eStat	: 4;
			eNCube_Mode		eMode	: 4;
		};
		unsigned char usReg;
	};
	unsigned char	ucIdx;
	char			aReqId[10];
}stNCube;

typedef struct stUSR_tag {
	stWIFI		stWifi;
	stFLASH		stFlash;
	stTOPIC		stTopic;
	stNCube		stnCube;
	stMQTT		stMqtt;
	stNOTICE	stNoti;
	stUPLOAD	stUpload;
	stPV		stTemp;
	stPV		stHumi;
}stUSR;

WiFiServer server(80);

stUSR			stUsr;
WiFiClient		wifiClient;
PubSubClient	Mqtt(wifiClient);
OneM2MClient 	nCube;

char in_message[MQTT_MAX_PACKET_SIZE];
StaticJsonBuffer<MQTT_MAX_PACKET_SIZE> jsonBuffer;

#if FEATHER_M0
	#define DHTPIN 		12	// DHT22 - Adafruit connect pin
#elif ITTH_SA001
	#define DHTPIN 		0	// DHT22 - HYS-H-ITTH_SA001 connect pin
#endif
#define DHTYPE		DHT22	// temp/humid sensor type
DHT dht22 = DHT(DHTPIN, DHTYPE);

#if (FEATHER_M0 && UseFlash)
	FlashStorage(stSysConf, stFLASH);
#endif
// Create a MDNS responder to listen and respond to MDNS name requests.
//WiFiMDNSResponder mdnsResponder;

void ShowUsrConf(void)
{
  unsigned char i;
  Serial.printf("\taAeName      : %s\n", stUsr.stFlash.aAeName);
  Serial.printf("\taCseId       : %s\n", stUsr.stFlash.aCseId);
  Serial.printf("\taCbName      : %s\n", stUsr.stFlash.aCbName);
  Serial.printf("\taBrokerIp    : %s\n", stUsr.stFlash.aBrokerIp);
  Serial.printf("\tusBrokerPort : %d\n", stUsr.stFlash.usBrokerPort);
  for(i=0; i<NetWorkCnt; i++) {
    Serial.printf("\taSsid[%d]     : %s\n", i, stUsr.stFlash.astNet[i].aSsid);
    Serial.printf("\taPass[%d]     : %s\n", i, stUsr.stFlash.astNet[i].aPass);
  }
	if(stUsr.stFlash.usBrokerPort == 1883) {
#if _DEBUG_
    /*
		unsigned char i;
    
		Serial.printf("\taAeName      : %s\n", stUsr.stFlash.aAeName);
		Serial.printf("\taCseId       : %s\n", stUsr.stFlash.aCseId);
		Serial.printf("\taCbName      : %s\n", stUsr.stFlash.aCbName);
		Serial.printf("\taBrokerIp    : %s\n", stUsr.stFlash.aBrokerIp);
		Serial.printf("\tusBrokerPort : %d\n", stUsr.stFlash.usBrokerPort);
		for(i=0; i<NetWorkCnt; i++) {
			Serial.printf("\taSsid[%d]     : %s\n", i, stUsr.stFlash.astNet[i].aSsid);
			Serial.printf("\taPass[%d]     : %s\n", i, stUsr.stFlash.astNet[i].aPass);
		}
    */
#endif
#if (Use_SH1106G | Use_SSD1306)
		display.clearDisplay();
		display.setCursor(1, 1);
		display.println("==== User Config ====");

		display.setCursor(1, 16);
		display.print("AE : ");
		display.println(stUsr.stFlash.aAeName);
		
		display.setCursor(1, 26);
		display.print("CSE : ");
		display.println(stUsr.stFlash.aCseId);
		
		display.setCursor(1, 36);
		display.print("CB : ");
		display.println(stUsr.stFlash.aCbName);
		
		display.setCursor(1, 46);
		display.print("IP : ");
		display.println(stUsr.stFlash.aBrokerIp);
		
		display.setCursor(1, 57);
		display.print("Port : ");
		display.println(stUsr.stFlash.usBrokerPort);
		display.display();
		delay(3000);
#endif
	}
}

#if UseFlash
void GetUsrConf(void)
{
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);						
#endif
	stUsr.stFlash = stSysConf.read();
}
#elif UseE2p
void GetUsrConf(void)
{
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);						
#endif

#if FEATHER_M0
	if(i2ceeprom.begin(0x50))			Serial.println("Found I2C EEPROM");	// you can stick the new i2c addr HIGH here, e.g. begin(0x51);
#elif ITTH_SA001
	if(i2ceeprom.begin(0x50, &Wire1))	Serial.println("Found I2C EEPROM");	// you can stick the new i2c addr HIGH here, e.g. begin(0x51);
#endif
	else {
		Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
		while(1);
	}

	i2ceeprom.read(0, (unsigned char*)&stUsr.stFlash, sizeof(stFLASH));
	if(stUsr.stFlash.usBrokerPort == 0xFFFF)	memset(&stUsr.stFlash, 0x0, sizeof(stUsr.stFlash));
}

void SetUseConf(void)
{
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);						
#endif

	i2ceeprom.write(0, (unsigned char*)&stUsr.stFlash, sizeof(stFLASH));
}
#else
void GetUsrConf(void)
{
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);						
#endif
	strcpy(stUsr.stFlash.aAeName, "Dynamic_T230531");
	strcpy(stUsr.stFlash.aCseId,  "/Mobius2");
	strcpy(stUsr.stFlash.aCbName, "Mobius");
	strcpy(stUsr.stFlash.aBrokerIp, "192.168.2.88");
	stUsr.stFlash.usBrokerPort = 1883;
}
#endif

void BuildResource(void)
{
	String to;
	String rn[] = {"update", "led", "temp", "humid", "ledDipTest" }; // 연결되면 디렉터리(방)를 만듬

#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
#endif

#if 1	// Application Entity
	to  = '/';
	to += stUsr.stFlash.aCbName;
	nCube.configResource(2, to, stUsr.stFlash.aAeName);
	#if _DEBUG_
	Serial.printf("\tnCube.configResource( 2, %s, %s)\n", to.c_str(), stUsr.stFlash.aAeName);
	#endif
#endif
#if 1	// Container Entity					**********/
	to += '/';
	to += stUsr.stFlash.aAeName;
	for(unsigned char i = 0; i<5; i++) {
		nCube.configResource(3, to, rn[i]);
	#if _DEBUG_
		Serial.printf("\tnCube.configResource( 3, %s, %s)\n", to.c_str(), rn[i].c_str());
	#endif
	}
#endif
#if 1	// Sub Entity
	nCube.configResource(23, to + "/update",     "sub");
	nCube.configResource(23, to + "/led",        "sub");
	nCube.configResource(23, to + "/ledDipTest", "sub");
	#if _DEBUG_
	Serial.printf("\tnCube.configResource(23, %s/update, sub)\n", to.c_str());
	Serial.printf("\tnCube.configResource(23, %s/led, sub)\n", to.c_str());
	Serial.printf("\tnCube.configResource(23, %s/ledDipTest, sub)\n", to.c_str());
	#endif
#endif
}


void Topics_Init(void) // topic으로 데이터를 주고 받는다.
{
	String tmp;
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
#if 1	// Resp topic 응답
	tmp  = "/oneM2M/resp/";
	tmp += "S";
	tmp += stUsr.stFlash.aAeName;
	tmp += stUsr.stFlash.aCseId;
	tmp += "/json";
	tmp.toCharArray(stUsr.stTopic.aResp, TopicSize);
	#if _DEBUG_
	Serial.printf("\tResp topic : %s\n", stUsr.stTopic.aResp);
	#endif
#endif
#if 1	// Req topic 요청
	tmp  = "/oneM2M/req";
	tmp += stUsr.stFlash.aCseId;
	tmp += "/S";
	tmp += stUsr.stFlash.aAeName;
	tmp += "/json";
	tmp.toCharArray(stUsr.stTopic.aNoti, TopicSize);
	#if _DEBUG_
	Serial.printf("\tNoti topic : %s\n", stUsr.stTopic.aNoti);
	#endif
#endif
}

void nCube_Init(void) //입력 받은 서버를 등록
{
	String AeId;

#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
	AeId = "S";
	AeId += stUsr.stFlash.aAeName;

#if 1
	nCube.Init(stUsr.stFlash.aCseId, stUsr.stFlash.aBrokerIp, AeId);
#else
	nCube.Init(stUsr.stFlash.aCseId, "192.168.2.88", AeId);
#endif
#if _DEBUG_
	Serial.printf("\tnCube.Init(%s, %s, %s)\n", stUsr.stFlash.aCseId, stUsr.stFlash.aBrokerIp, AeId.c_str());
#endif

	//stUsr.stnCube.eMode = eCreate_Cnt;	///////////////////////////////
	//stUsr.stnCube.eStat = eNCube_Stat_Req;
}

void Mqtt_Init(void) // mqtt 프로토콜로 Mobius 서버와 통신할 때 필요한 정보 설정(IP, PORT, 데이터를 보내거나 받을 때 콜백함수는 어떤 것을 쓰는지 등)
{
	byte mac[6];
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
#endif

	WiFi.macAddress(mac);
	sprintf(stUsr.stMqtt.aId, "nCube-%.2X%.2X", mac[1], mac[0]);
  Serial.printf(" mqtt id : %s\n", stUsr.stMqtt.aId);
    randomSeed(mac[0] + mac[1]);
#if 1
	Mqtt.setServer(stUsr.stFlash.aBrokerIp, stUsr.stFlash.usBrokerPort);
#else
	Mqtt.setServer("192.168.2.88", stUsr.stFlash.usBrokerPort);
#endif
    Mqtt.setCallback(mqtt_message_handler);

#if _DEBUG_
	Serial.printf("\tMqtt.setServer(%s, %d)\n", stUsr.stFlash.aBrokerIp, stUsr.stFlash.usBrokerPort);
	Serial.printf("\tMqtt.setCallback(mqtt_message_handler)\n");
#endif

	stUsr.stMqtt.eStat = eMqtt_Stat_Init;
}

void resp_handler(int response_code, String response_id)
{
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
	String request_id = String(stUsr.stnCube.aReqId);

	if(request_id == response_id) {
		switch(response_code) {
			case 2000 :
			case 2001 :
			case 2002 :
			case 4004 :
			case 4105 :
				switch(stUsr.stnCube.eMode) {
					case eCreate_Ae :
						if(++stUsr.stnCube.ucIdx >= nCube.getAeCount()) {
							stUsr.stnCube.ucIdx = 0;
							stUsr.stnCube.eMode = eCreate_Cnt;
							
						}
						break;
					case eCreate_Cnt :
						if(++stUsr.stnCube.ucIdx >= nCube.getCntCount()) {
							stUsr.stnCube.ucIdx = 0;
							stUsr.stnCube.eMode = eDelete_Sub;
						}
						break;
					case eDelete_Sub :
						stUsr.stnCube.eMode = eCreate_Sub;
						break;
					case eCreate_Sub :
						if(++stUsr.stnCube.ucIdx >= nCube.getSubCount()) {
							stUsr.stnCube.ucIdx = 0;
							stUsr.stnCube.eMode = eCreate_Cin;
						}
						break;
					case eCreate_Cin :
						if(++stUsr.stUpload.stQue.PopIdx >= QueueSize)	stUsr.stUpload.stQue.PopIdx = 0;

						digitalWrite(ledPin, LOW);
						break;
					default :
						break;
				}
#if (Use_SH1106G | Use_SSD1306)
				if(!stUsr.stUpload.bSetFlag) {
					if(stUsr.stnCube.eMode >= eCreate_Cnt) {
						display.setCursor(1, 16);
						display.println("createAE... OK");
					}
					if(stUsr.stnCube.eMode >= eDelete_Sub) {
						display.setCursor(1, 29);
						display.println("createCnt... OK");
					}
					if(stUsr.stnCube.eMode >= eCreate_Sub) {
						display.setCursor(1, 43);
						display.println("deleteSub... OK");
					}
					if(stUsr.stnCube.eMode >= eCreate_Cin) {
						display.setCursor(1, 57);
						display.println("createSub... OK");
					}
					display.display();
					if(stUsr.stnCube.eMode >= eCreate_Cin) {
						stUsr.stUpload.bSetFlag = 1;
						delay(3000);
					}
				}
#endif
				if(stUsr.stnCube.eMode == eCreate_Cin) {
#if _DEBUG_
					Serial.printf("\tresponse_code : %d", response_code);
					Serial.printf("\tFifo GetSize : %d, GetFree : %d\n\n", GetSize(&stUsr.stUpload.stQue), GetFree(&stUsr.stUpload.stQue));
#endif
					stUsr.stUpload.eStat = eUploaded;
				}
				else {
#if _DEBUG_
					Serial.printf("\tresponse_code : %d\n\n", response_code);
#endif
					stUsr.stnCube.eStat  = eNCube_Stat_Req;
				}
				break;
			default :
				break;
		}
	}
#if _DEBUG_
	else	Serial.printf("\tID mismatch - Req : %s, Resp : %s\n\n", request_id.c_str(), response_id.c_str());
#endif
}

void noti_handler(String sur, String rqi, String con)
{
	if(stUsr.stnCube.eMode == eCreate_Cin) {
#if _DEBUG_
		Serial.printf("%s():%d\n", __func__, __LINE__);

		if(sur.charAt(0) != '/')	sur = '/' + sur;
		Serial.printf("\t%s\n", sur.c_str());
#endif
		String valid_sur = nCube.validSur(sur);
		if(valid_sur != "empty") {
			stUsr.stNoti.stQue.sRef[stUsr.stNoti.stQue.PushIdx] = valid_sur;
			stUsr.stNoti.stQue.sCon[stUsr.stNoti.stQue.PushIdx] = con;
			stUsr.stNoti.stQue.sRqi[stUsr.stNoti.stQue.PushIdx] = rqi;
#if 1
			if(++stUsr.stNoti.stQue.PushIdx >= QueueSize)		stUsr.stNoti.stQue.PushIdx = 0;
#else
			if(++stUsr.stNoti.stQue.PushIdx >= QueueSize)		stUsr.stNoti.stQue.PushIdx = 0;
			if(  stUsr.stNoti.stQue.PushIdx == stUsr.stNoti.stQue.PopIdx) {
				if(++stUsr.stNoti.stQue.PopIdx >= QueueSize)	stUsr.stNoti.stQue.PopIdx  = 0;
			}
#endif
		}
	}
}

void mqtt_message_handler(char* topic_in, byte* payload, unsigned int length)
{
	unsigned char res = 0;
	String topic      = String(topic_in);
	
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);

	Serial.printf("\ttopic_in : %s len : %d\n\t", topic_in, length);
	for(unsigned long i=0; i<length; i++)	Serial.print((char)payload[i]);
	Serial.println();
#endif
	if(topic.substring(8, 12) == "resp")	res = 1;
	else {
		if(topic.substring(8, 11) == "req")	res = 2;
	}

	if(res) {
		memset((char*)in_message, '\0',    length + 1);
        memcpy((char*)in_message, payload, length);

		JsonObject& root = jsonBuffer.parseObject(in_message);

		if(!root.success())	Serial.println("\tparseObject() failed...");
		else {
			wifiClient.flush();
			
			switch(res) {
				case 1 :
					resp_handler(root["rsc"], root["rqi"]);
					break;
				case 2 :
					noti_handler(root["pc"]["m2m:sgn"]["sur"], root["rqi"], root["pc"]["m2m:sgn"]["nev"]["rep"]["m2m:cin"]["con"]);
					break;
				default :
					break;
			}
		}
		jsonBuffer.clear();
	}
	//system_watchdog = 0;
}

void rand_str(char *dest, size_t length)
{
    char charset[] = "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while(length-- > 0) {
        size_t index = random(62);
        *dest++      = charset[index];
    }
    *dest = '\0';
}

void WiFiInit(void)
{
	unsigned char toggle = 1;
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
#if FEATHER_M0
	WiFi.setPins(8, 7,  4, 2);
#elif ITTH_SA001
	WiFi.setPins(8, 7, 48, 2);
#endif	

	if(WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present");
		while(1) {
			digitalWrite(ledPin, toggle);
			toggle = 1 - toggle;
			delay(100);
		}
	}
}

unsigned short GetSize(stQUEUE_BASE* pFifo)
{
	unsigned short res;

	res = (pFifo->PushIdx >= pFifo->PopIdx) ? (            pFifo->PushIdx - pFifo->PopIdx)\
											: (QueueSize + pFifo->PushIdx - pFifo->PopIdx);
	return res;
}

unsigned short GetFree(stQUEUE_BASE* pFifo)
{
	unsigned short res;

	res = (pFifo->PushIdx >= pFifo->PopIdx) ? (QueueSize + pFifo->PopIdx - pFifo->PushIdx - 1)\
											: (            pFifo->PopIdx - pFifo->PushIdx - 1);
	return res;
}

void ToggleLed(unsigned long sTerm = 500)
{
	       unsigned long curr    = millis();
	static unsigned long sPrev   = 0;
	static unsigned char sToggle = 0;

	if(curr - sPrev >= sTerm) {
		sPrev = curr;

		digitalWrite(ledPin, sToggle);
		sToggle = 1 - sToggle;
	}
}

void setup() {
	// put your setup code here, to run once:
	pinMode(ledPin, OUTPUT);

#if _DEBUG_
	Serial.begin(115200);
	while(!Serial) {
		for(unsigned char i=0; i<20; i++) {
			digitalWrite(ledPin, i & 0x1);
			delay(100);
		}
		break;	/// about 2 sec
	}
#endif
	digitalWrite(ledPin, LOW);

	WiFiInit(); // wifi 초기화
	dht22.begin(); // 온습도 센서 초기화

#if (Use_SH1106G | Use_SSD1306)
	#if Use_SSD1306
    // OLED 디스플레이 초기화
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));

      for(;;); // Don't proceed, loop forever
    }
	#elif Use_SH1106G
	display.begin(i2c_Address, true);
	display.display();
	delay(1000);
	#endif
    // 아두이노 부팅 후, 한영넉스 AI 연구소 를 표시 한다.
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(28, 20);
    display.println("HANYOUNG NUX");	/// TextSize = 1, Font width = 6, height = 8
    display.setCursor(46, 30);
    display.println("AI R&D");
    display.setCursor(0, 57);
    display.print("2023/07");
    display.setCursor(86, 57);
    display.print("Ver 1.1");
    display.display();
	stUsr.stTemp.fPv = dht22.readTemperature() + stUsr.stTemp.fAdj;
	stUsr.stHumi.fPv = dht22.readHumidity()    + stUsr.stHumi.fAdj;
    delay(2000);
#endif

	GetUsrConf();   // EEPROM 데이터 가져오기
	ShowUsrConf();  // EEPROM에서 가져온 데이터 출력

	if(stUsr.stFlash.usBrokerPort == 1883) { //onem2m 서버와 mqtt로 통신할 때 필요한 정보들 세팅
		Topics_Init();
		nCube_Init();
		Mqtt_Init();
		BuildResource();

		stUsr.stWifi.eMode = eWiFi_Mode_Sa; // change eWiFi_Mode_Ap  eWiFi_Mode_Sa
	}
#if 0
	OTAClient.begin(AE_NAME, FIRMWARE_VERSION); // onem2m 서버에 펌웨어를 올리면 다운로드 해주는 기능
#endif
	digitalWrite(ledPin, HIGH);
}

/**
@fn     void ConnToSTA(stNETWORK* pNet)
@brief  WIFI STA 모드 연결
@param  pNet  stNETWORK 구조체 타입 포인터
*/
void ConnToSTA(stNETWORK* pNet)
{
	unsigned char cnt = 0;

#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
	Serial.printf("\tAttempting to connect to the network named : %s", pNet->aSsid);
#endif
#if (Use_SH1106G | Use_SSD1306)
	display.clearDisplay();
	display.setTextSize(1);
	display.setCursor(1, 32);
	display.print("Try to connect WIFI...");
	display.display();
	delay(3000);
#endif
	while(WiFi.status() != WL_CONNECTED) {
		WiFi.begin(pNet->aSsid, pNet->aPass);

		if(++cnt >= 5)	break;

		delay(2000);	// wait 2 seconds for connection
#if _DEBUG_
		Serial.printf("\n\t[%d] Try to connect...", cnt);
#endif
	}
#if _DEBUG_
	if(cnt >= 5)	Serial.printf(" Fail...\n");
	else			Serial.printf(" Connect!\n");
#endif
}

/**
@fn   void ConnToAP(void)
@brief  WIFI AP 모드 연결
*/
void ConnToAP(void) // Feather_WiFi[mac address]로 주변에 WIFI를 검색하면 이 모드가 이 WIFI를 잡는다.
{
	char i;
	unsigned char cnt = 0;
	unsigned char nTmp;
	byte          mac[6];
	String        sTmp = "Feather_WiFi[";
	String        sMac = "";
	char          aAp[sTmp.length() + 2 + (sizeof(mac) << 1)];
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);
#endif

	WiFi.macAddress(mac);

	for(i=sizeof(mac)-1; i>0; i--) sMac += String(mac[i], HEX);
	sMac += String(mac[i], HEX);
	sMac.toUpperCase();
	sTmp += sMac;
	sTmp += ']';
	sTmp.toCharArray(aAp, sTmp.length() + 1);
#if _DEBUG_
	Serial.printf("\tCreating access point named : %s\n", aAp);
#endif
#if (Use_SH1106G | Use_SSD1306)
	display.clearDisplay();
	display.setCursor(1, 1);
	display.println("WIFI Mode : AP");
	display.setCursor(1, 16);
	display.printf("AP name : %s\n", aAp);
#endif
	while(WiFi.status() != WL_AP_LISTENING) {
		WiFi.beginAP(aAp);

		if(++cnt >= 5)	break;
	}

#if _DEBUG_
	if(++cnt >= 5)	Serial.println("\tFailed...");
#endif
}

/**
@fn   
@brief  
*/
inline int IsHex(int x)
{
  return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F');
}

int UrlDecode(const char *s, char *dec)
{
  char *o;
  const char *e = s + strlen(s);
  int c;

  for(o=dec; s<=e; o++) {
    c = *s++;
    if(c == '+')  c = ' ';
    else if(c == '%' && (!IsHex(*s++) || !IsHex(*s++) || !sscanf(s-2, "%02x", &c)))  return -1;

    if(dec) *o = c;    
  }
  return o - dec;
}

void WebService(void) // Webserver에 연결되어 사용자로부터 정보를 입력 받는다.
{
#if 0
	// Call the update() function HIGH the MDNS responder every loop iteration to
	// make sure it can detect and respond to name requests.
	mdnsResponder.poll();
#endif
	WiFiClient client = server.available();

	if(client) {
		String currentLine = "";
    String webModeStr[] = { "/ae"
            , "/ces"};
    eWEB_MODE eWebMode = eWeb_Mode_User;
    unsigned char i;

		while(client.connected()) {
			if(client.available()) {
				char c = client.read();
#if _DEBUG_
				Serial.write(c);
#endif
				if(c == '\n') {
          if(!eWebMode) { // Web Mode 파악
              for(i=0; i<sizeof(webModeStr)/sizeof(String); i++)  {
                if(currentLine.indexOf(webModeStr[i]) != -1)  {
                  switch(i) {
                    case 0:
                      eWebMode = eWeb_Mode_Ae;
                      break;
                    case 1:
                      eWebMode = eWeb_Mode_Ces;
                      break;
                    default:
                      break;
                  }
                }
              }
            }
					if(currentLine.length() == 0) {
#if _DEBUG_
						// request ended, web page print out
						Serial.print("Client Request Ended...\t");
						Serial.println("Web page transfer Start!");
#endif
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println("Connection: close");
						client.println();
						//
						client.println("<!DOCTYPE html>");
						client.println("<html>");
						client.println("<head>");
						client.println("<meta charset=\"UTF-8\">");
						client.println("<title>Network Info</title>");
						client.println("</head>");
						client.println("<body>");
						client.println("<h1>WiFi Connection Status</h1>");
						client.println("<p>IP Address : ");
						IPAddress ip = WiFi.localIP();
						client.println(ip);
						client.println("</p>");
						client.print("<p>SSID : ");
						client.print(WiFi.SSID());
						client.print("</p>");
						client.print("<p>RSSI : ");
						client.print(WiFi.RSSI());
						client.println(" dBm</p>");
						client.println("<hr>");
						client.println("<h2>Network Credentials</h2>");
						client.println("<form>");
#if 1
            switch(eWebMode)  {
              case eWeb_Mode_Ae:
                client.println("<input type=\"text\" name=\"ae name\" placeholder=\"AE NAME\"><br>");
                break;
              case eWeb_Mode_Ces:
                client.println("<input type=\"text\" name=\"cse id\" placeholder=\"CSE ID\"><br>");
                client.println("<input type=\"text\" name=\"cb name\" placeholder=\"CB NAME\"><br>");
                client.println("<input type=\"text\" name=\"broker ip\" placeholder=\"BROKER IP\"><br>");
                client.println("<input type=\"text\" name=\"broker port\" placeholder=\"BROKER PORT\"><br>");
                break;
              default:
                client.println("<input type=\"text\" name=\"ssid\" placeholder=\"SSID\"><br>");
						    client.println("<input type=\"password\" name=\"pass\" placeholder=\"PASSWORD\"><br>");
#if	(NetWorkCnt >= 2)
						    client.println("<input type=\"text\" name=\"ssid2\" placeholder=\"SSID2\"><br>");
						    client.println("<input type=\"password\" name=\"pass2\" placeholder=\"PASSWORD2\"><br>");
#endif
#if	(NetWorkCnt >= 3)
						    client.println("<input type=\"text\" name=\"ssid3\" placeholder=\"SSID3\"><br>");
						    client.println("<input type=\"password\" name=\"pass3\" placeholder=\"PASSWORD3\"><br>");
#endif
                break;
            }
#endif
						client.println("<input type=\"submit\" value=\"Submit\">");
						client.println("<input type=\"reset\" value=\"Reset\">");
						client.println("</form>");
						client.println("</body>");
						client.println("</html>");
						break;
					}
					else {
						String        sTmp;
						int           sIdx, eIdx;
						unsigned char res = 0x0;
            unsigned char FirstIdx;
						unsigned char MaxIdx;
            unsigned char Count;
						unsigned char j = 0;
            unsigned char k;
            String webModeStr[] = { "/ae"
                    , "/ces"};
						String IdxStr[] = {	"ae+name="
										,	"cse+id="
										,	"cb+name="
										,	"broker+ip="
										,	"broker+port="
										,	"ssid="
										,	"pass="
#if	(NetWorkCnt >= 2)
										,	"ssid2="
										,	"pass2="
#endif
#if	(NetWorkCnt >= 3)
										,	"ssid3="
										,	"pass3="
#endif
										,	"HTTP/1.1" };
						
						MaxIdx = (sizeof(IdxStr) / sizeof(String));
            if(eWebMode == eWeb_Mode_Ae)  {
              FirstIdx = 0;
              Count = MaxIdx - 8;
            }
            else if(eWebMode == eWeb_Mode_Ces)  {
              FirstIdx = 1;
              Count = MaxIdx - 4;
            }
            else  {
              FirstIdx = 5;
              Count = MaxIdx - 2;
            }

						for(i=FirstIdx; i<=Count; i++) {
							sIdx = currentLine.indexOf(IdxStr[i]);

              if(i != Count)    eIdx = currentLine.indexOf('&' + IdxStr[i + 1]);
              else              eIdx = currentLine.indexOf(IdxStr[MaxIdx - 1]);

              if(sIdx == -1 || eIdx == -1)	break;
              sTmp = currentLine.substring(sIdx + IdxStr[i].length(), eIdx);
              
							switch(i) {
								case 0 :
                  sTmp.trim();
									UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aAeName);
                  res = 1;
									break;
								case 1 :
									UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aCseId);
									break;
								case 2 :
									UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aCbName);
									break;
								case 3 :
									UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aBrokerIp);
									break;
								case 4 :
									sTmp.trim();
									stUsr.stFlash.usBrokerPort = sTmp.toInt();
                  res = 1;
									break;
								case 5 :
								case 7 :
								case 9 :
									UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.astNet[j].aSsid);
									break;
								case  6 :
								case  8 :
								case 10 :
									sTmp.trim();
									sTmp.toCharArray(stUsr.stFlash.astNet[j].aPass, sTmp.length()+1);

									if(++j >= NetWorkCnt)	res = 1;
									break;
								default :
									break;
							}
						}
#if 0
						sIdx = currentLine.indexOf("ae+name=");
						eIdx = currentLine.indexOf("&cse+id=");
						if(sIdx != -1 && eIdx != -1) {
							sTmp = currentLine.substring(sIdx + 8, eIdx);
							UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aAeName);
	#if _DEBUG_
							Serial.print("AE name : ");
							Serial.println(stUsr.stFlash.aAeName);
	#endif
						}

						sIdx = currentLine.indexOf("cse+id=");
						eIdx = currentLine.indexOf("&cb+name=");
						if(sIdx != -1 && eIdx != -1) {
							sTmp = currentLine.substring(sIdx + 7, eIdx);
							UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aCseId);
	#if _DEBUG_
							Serial.print("CSE Id : ");
							Serial.println(stUsr.stFlash.aCseId);
	#endif
						}

						sIdx = currentLine.indexOf("cb+name=");
						eIdx = currentLine.indexOf("&broker+ip=");
						if(sIdx != -1 && eIdx != -1) {
							sTmp = currentLine.substring(sIdx + 8, eIdx);
							UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aCbName);
	#if _DEBUG_
							Serial.print("CB Name : ");
							Serial.println(stUsr.stFlash.aCbName);
	#endif
						}

						sIdx = currentLine.indexOf("broker+ip=");
						eIdx = currentLine.indexOf("&broker+port=");
						if(sIdx != -1 && eIdx != -1) {
							sTmp = currentLine.substring(sIdx + 10, eIdx);
							UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.aBrokerIp);
	#if _DEBUG_
							Serial.print("Broker Ip : ");
							Serial.println(stUsr.stFlash.aBrokerIp);
	#endif
						}

						sIdx = currentLine.indexOf("broker+port=");
						eIdx = currentLine.indexOf("&ssid=");
						if(sIdx != -1 && eIdx != -1) {
							sTmp = currentLine.substring(sIdx + 12, eIdx);
							sTmp.trim();

							stUsr.stFlash.usBrokerPort = sTmp.toInt();
	#if _DEBUG_
							Serial.print("Broker Port : ");
							Serial.println(stUsr.stFlash.usBrokerPort);
	#endif
						}
#endif
#if 0
						//
						sIdx = currentLine.indexOf("ssid=");
						eIdx = currentLine.indexOf("&pass=");
						if(sIdx != -1 && eIdx != -1) {
							res++;
							sTmp = currentLine.substring(sIdx + 5, eIdx);

							UrlDecode((char*)sTmp.c_str(), stUsr.stFlash.astNet[0].aSsid);
	#if _DEBUG_
							Serial.print("User input id : ");
							Serial.println(stUsr.stFlash.astNet[0].aSsid);
	#endif
						}

						sIdx = currentLine.indexOf("pass=");
						eIdx = currentLine.indexOf("HTTP/1.1");
						if(sIdx != -1 && eIdx != -1) {
							res++;
							sTmp = currentLine.substring(sIdx + 5, eIdx);
							sTmp.trim();

							sTmp.toCharArray(stUsr.stFlash.astNet[0].aPass, sTmp.length()+1);
	#if _DEBUG_
							Serial.print("User input pw : ");
							Serial.println(stUsr.stFlash.astNet[0].aPass);
	#endif
						}
#endif
						if(res) {
              stUsr.stWifi.eMode = eWiFi_Mode_Sa;

							ShowUsrConf();
#if UseFlash
							stSysConf.write(stUsr.stFlash);
#elif UseE2p
							SetUseConf();
#endif
							delay(2000);

              if(eWebMode == eWeb_Mode_User)  {
#if 1
                Topics_Init();
                nCube_Init();
                Mqtt_Init();
                BuildResource();
#endif
              }
              client.stop();
              WiFi.end();

							digitalWrite(ledPin, LOW);
						}
						// new line, clear currentLine
						currentLine = "";
					}
				} else if(c != '\r')	currentLine += c;
			}
		}
		client.stop(); // close the connection
#if _DEBUG_
    Serial.println("\tClient disconnected");
#endif
	}
}

void WiFi_SAMode(void)
{
	static unsigned char sConnected = 0;

	if(WiFi.status() != WL_CONNECTED || WiFi.SSID() == 0) {
		String ssid;
		unsigned char i;
#if _DEBUG_
		Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
		stUsr.stWifi.eStat = eWiFi_Stat_Init;

		for(i=0; i<NetWorkCnt; i++) {
			ssid = stUsr.stFlash.astNet[i].aSsid;

			if(ssid.length() == 0)	continue;

			stUsr.stWifi.eStat = eWiFi_Stat_TryToConn;

			ConnToSTA(&stUsr.stFlash.astNet[i]);

			if(WiFi.status() == WL_CONNECTED)	break;
		}

		if(WiFi.status() == WL_CONNECTED) {
			sConnected         = true;
			stUsr.stWifi.eStat = eWiFi_Stat_Connected;
		}
		else {
			/// Flash memory에 저장된 id, pw로 연결 실패시 AP 모드 진입
			if(!sConnected)	stUsr.stWifi.eMode = eWiFi_Mode_Ap;
			else {
				/// Flash memory 혹은 AP 모드에서 등록한 id, pw 연결 실패시 시스템 리셋
				if(nCube.getAeCount() >= AE_COUNT) {
					Serial.println("NVIC_SystemReset...");
					delay(100);
					NVIC_SystemReset();
				}
			}
		}
	}
}
//char mdnsName[] = "wifi101"; // the MDNS name that the board will respond to
void WiFi_APMode(void)
{
	switch(stUsr.stWifi.eMode) {
		case eWiFi_Mode_Ap :
			ConnToAP(); // WIFI AP모드 연결

			if(WiFi.status() == WL_AP_LISTENING) {
				server.begin();

				stUsr.stWifi.eMode = eWiFi_Mode_WebServer;
#if 0//_DEBUG_
				// Setup the MDNS responder to listen to the configured name.
				// NOTE: You _must_ call this _after_ connecting to the WiFi network and
				// being assigned an IP address.
				if (!mdnsResponder.begin(mdnsName)) {
					Serial.println("Failed to start MDNS responder!");
					while(1);
				}

				Serial.print("\tServer listening at http://");
				Serial.print(mdnsName);
				Serial.println(".local/");
#else
				IPAddress ip = WiFi.localIP();
				Serial.print("\tServer begin.... IP Address : ");
				Serial.println(ip);
	#if (Use_SH1106G | Use_SSD1306)
				display.setCursor(1, 40);
				display.print("IP Addr : ");
				display.println(ip);
				
				display.setCursor(1, 56);
				display.println("Server begin...");
				display.display();
	#endif
#endif
			}
			else	break;
		case eWiFi_Mode_WebServer :
			ToggleLed();
			WebService(); // WIFI를 잡으면 웹 서버로 넘어가면서 실행
			break;
		default :
			break;
	}
}

void WiFi_Task(void)
{
	switch(stUsr.stWifi.eMode) {
		case eWiFi_Mode_Ap :
		case eWiFi_Mode_WebServer :
			WiFi_APMode(); // 공유기(AP) 모드
			break;
		default :
			WiFi_SAMode(); // 일반 wifi모드
			break;
	}
}

#define MqttReconnectTime	2000
unsigned long prev_Mqtt = 0;
void Mqtt_Reconnect(void)
{
	if(stUsr.stWifi.eStat == eWiFi_Stat_Connected) {
		unsigned long curr = millis();

		if(curr - prev_Mqtt >= MqttReconnectTime) {
			prev_Mqtt = curr;
#if _DEBUG_
			Serial.printf("%s():%d\n", __func__, __LINE__);
			Serial.print("\tAttempting MQTT connection...");
#endif
			if(Mqtt.connect(stUsr.stMqtt.aId)) {
				Serial.print("\tOk!\n");
				
				if(!Mqtt.subscribe(stUsr.stTopic.aResp))	Serial.printf("\t%s\tFail subscribed\n", stUsr.stTopic.aResp);
				if(!Mqtt.subscribe(stUsr.stTopic.aNoti))	Serial.printf("\t%s\tFail subscribed\n", stUsr.stTopic.aNoti);

				stUsr.stMqtt.nRetry = 0;
				stUsr.stMqtt.eStat  = eMqtt_Stat_Connected;

				nCube.reset_heartbeat();
			}
			else	Serial.printf("\tFailed, rc = %d\tTry to connect HIGH 2 sec\n", Mqtt.state());
		}
	}
}

void Mqtt_Task(void)
{
	if(stUsr.stWifi.eStat == eWiFi_Stat_Connected) {
		if(!Mqtt.loop()) {
#if (Use_SH1106G | Use_SSD1306)
			display.clearDisplay();
			display.setTextSize(1);
			display.setCursor(1, 32);
			display.println("Try to connect MQTT...");
			display.display();
			delay(3000);
#endif
			stUsr.stMqtt.eStat = eMqtt_Stat_TryToConn;

			Mqtt_Reconnect();

			if(stUsr.stMqtt.eStat != eMqtt_Stat_Connected) {
				if(++stUsr.stMqtt.nRetry >= 10) {
					stUsr.stMqtt.nRetry = 0;
					stUsr.stWifi.eMode  = eWiFi_Mode_Ap;
				}
			}
		}
		else	stUsr.stMqtt.eStat = eMqtt_Stat_Connected;
	}
}

#define PublisherReqTime	15000
unsigned long prev_Publisher = 0;
void Publisher(void)
{
	static unsigned long sPrev = 0;

	if(stUsr.stWifi.eStat == eWiFi_Stat_Connected && stUsr.stMqtt.eStat == eMqtt_Stat_Connected) {
		switch(stUsr.stnCube.eStat) {
			case eNCube_Stat_Ing :
				if(millis() - sPrev >= PublisherReqTime) {
#if _DEBUG_
					Serial.printf("%s():%d\n", __func__, __LINE__);
					Serial.printf("\tReq timeout...\n");
#endif
					stUsr.stnCube.eStat = eNCube_Stat_Req;
				}
				else	break;
			case eNCube_Stat_Req :
				{
					String strBody = "";
					
					sPrev = millis();
					rand_str(stUsr.stnCube.aReqId, sizeof(stUsr.stnCube.aReqId) - 2);
#if _DEBUG_
					Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
					switch(stUsr.stnCube.eMode) {
						case eCreate_Ae  :
#if _DEBUG_
							Serial.printf("\tnCube.createAE(Mqtt, %s, %d, 3.14)\n", stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx);
#endif
#if (Use_SH1106G | Use_SSD1306)
							display.clearDisplay();
							display.setCursor(1, 1);
							display.println("==== OneM2M API ====");
							display.setCursor(1, 16);
							display.println("createAE...");
#endif
							digitalWrite(ledPin, HIGH);
							strBody = nCube.createAE(Mqtt, stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx, "3.14");
							break;
						case eCreate_Cnt :
#if _DEBUG_
							Serial.printf("\tnCube.createCnt(Mqtt, %s, %d)\n", stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx);
#endif
#if (Use_SH1106G | Use_SSD1306)
							display.setCursor(1, 29);
							display.println("createCnt...");
#endif
              strBody = nCube.createCnt(Mqtt, stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx);
							break;
						case eDelete_Sub :
#if _DEBUG_
							Serial.printf("\tnCube.deleteSub(Mqtt, %s, %d)\n", stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx);
#endif
#if (Use_SH1106G | Use_SSD1306)
							display.setCursor(1, 43);
							display.println("deleteSub...");
#endif
							strBody = nCube.deleteSub(Mqtt, stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx);
							break;
						case eCreate_Sub :
#if _DEBUG_
							Serial.printf("\tnCube.createSub(Mqtt, %s, %d)\n", stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx);
#endif
#if (Use_SH1106G | Use_SSD1306)
							display.setCursor(1, 57);
							display.println("createSub...");
#endif
							strBody = nCube.createSub(Mqtt, stUsr.stnCube.aReqId, stUsr.stnCube.ucIdx);
							break;
						case eCreate_Cin :
						default :
							break;
					}
#if _DEBUG_
					if(strBody == "0")	Serial.println("\tReq fail...\n");
					else {
	#if (Use_SH1106G | Use_SSD1306)
						display.display();
	#endif
						stUsr.stnCube.eStat = eNCube_Stat_Ing;

						Serial.printf("\t[%d] ", strBody.length() + 1);
						Serial.println(nCube.getReqTopic());
						Serial.print("\t");
						Serial.println(strBody);
					}
#else
					if(strBody != "0")	stUsr.stnCube.eStat = eNCube_Stat_Ing;
#endif
				}
				break;
			default :
				break;
		}
	}
}

#define UploadRetryTime		10000	// ms
#define UploadMaxCnt		2
void Upload_Task(void)
{
	static unsigned long sPrev = 0;

	if(stUsr.stWifi.eStat == eWiFi_Stat_Connected && stUsr.stMqtt.eStat == eMqtt_Stat_Connected) {
		switch(stUsr.stUpload.eStat) {
			case eUploading :
				if(millis() - sPrev >= UploadRetryTime) {
#if _DEBUG_
					Serial.printf("%s():%d\n", __func__, __LINE__);
					Serial.printf("\tUpload timeout...\n");
#endif
					if(++stUsr.stUpload.nRetryCnt > UploadMaxCnt) {
						stUsr.stUpload.nRetryCnt = 0;
						
						if(!GetFree(&stUsr.stUpload.stQue)) {
							stUsr.stUpload.stQue.PopIdx = stUsr.stUpload.stQue.PushIdx = 0;
#if _DEBUG_
							Serial.printf("\tFifo reset....\n");
#endif
						}
					}
					stUsr.stUpload.eStat = eUploaded;
				}
				else	break;

			case eUploaded :
				if(GetSize(&stUsr.stUpload.stQue)) {
					if(wifiClient.available())	return;
					
					String strBody;

					sPrev                     = millis();
					stUsr.stUpload.nRetryCnt = 0;
					stUsr.stUpload.bDisplay  = 1;

					digitalWrite(ledPin, HIGH);

					stUsr.stUpload.stQue.sRqi[stUsr.stUpload.stQue.PopIdx].toCharArray(stUsr.stnCube.aReqId, sizeof(stUsr.stnCube.aReqId));
					strBody = nCube.createCin(Mqtt
											, stUsr.stUpload.stQue.sRqi[stUsr.stUpload.stQue.PopIdx]
											, stUsr.stUpload.stQue.sRef[stUsr.stUpload.stQue.PopIdx]
											, stUsr.stUpload.stQue.sCon[stUsr.stUpload.stQue.PopIdx]);
					wifiClient.flush();
#if _DEBUG_
					Serial.printf("%s():%d\n", __func__, __LINE__);

					if(strBody == "0")	Serial.println("\tReq fail...");
					else {
						stUsr.stUpload.eStat = eUploading;

						Serial.printf("\t[%s]---> %d]\n", nCube.getReqTopic().c_str(), strBody.length() + 1);
						Serial.print("\t");
						Serial.println(strBody);
					}
#else
					if(strBody != "0")	stUsr.stUpload.eStat = eUploading;
#endif
				}
				break;
			default :
				break;
		}
	}
}

void Split(String sData, char cSeprator)
{
//	int nCnt     = 0;
	int nIdx     = 0;
	String sTmp  = "";
	String sCopy = sData;
	
	stUsr.stNoti.nRefLen = 0;
	
	while(true) {
		nIdx = sCopy.indexOf(cSeprator);

		if(nIdx != -1) {
			sTmp                                      = sCopy.substring(0, nIdx);
			stUsr.stNoti.sRef[stUsr.stNoti.nRefLen++] = sTmp;
			sCopy                                     = sCopy.substring(nIdx + 1);
		}
		else {
			stUsr.stNoti.sRef[stUsr.stNoti.nRefLen++] = sCopy;
			break;
		}
//		nCnt++;
	}
}

void ledDipTestSetLED(String level)
{
	if(String(level) == "0")	digitalWrite(ledDipTest, LOW );
	else						digitalWrite(ledDipTest, HIGH);
}

void Noti_Task(void)
{
	if(GetSize(&stUsr.stNoti.stQue)) {
		unsigned char res = 0;
		String strBody;
#if _DEBUG_
		Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
		Split(stUsr.stNoti.stQue.sRef[stUsr.stNoti.stQue.PopIdx], '/');

		if(stUsr.stNoti.sRef[stUsr.stNoti.nRefLen - 1] == "led") {
			res = 1;
//			tasLed.setLED(noti_q.con[noti_q.pop_idx]);
		}
		else if(stUsr.stNoti.sRef[stUsr.stNoti.nRefLen - 1] == "update") {
			if(stUsr.stNoti.stQue.sCon[stUsr.stNoti.stQue.PopIdx] == "active") {
				res = 2;
//				OTAClient.start();   // active OTAClient upgrad process
			}
		}
		else {
			if(stUsr.stNoti.sRef[stUsr.stNoti.nRefLen - 1] == "ledDipTest") {
				res = 3;
				ledDipTestSetLED(stUsr.stNoti.stQue.sCon[stUsr.stNoti.stQue.PopIdx]);
			}
		}
		
		if(res) {
#if _DEBUG_
			Serial.printf("\t%s\n", stUsr.stNoti.stQue.sRef[stUsr.stNoti.nRefLen - 1].c_str());
#endif
			strBody = "{\"rsc\":\"2000\",\"to\":\"\",\"fr\":\"" + nCube.getAeid() + "\",\"pc\":\"\",\"rqi\":\"" + stUsr.stNoti.stQue.sRqi[stUsr.stNoti.stQue.PopIdx] + "\"}";
			nCube.response(Mqtt, strBody);
		}

		if(++stUsr.stNoti.stQue.PopIdx >= QueueSize)	stUsr.stNoti.stQue.PopIdx = 0;
	}
}

#define CinTime	60000
void Temp_Task(void)
{
	if(stUsr.stWifi.eStat == eWiFi_Stat_Connected) {
#if _DEBUG_
		Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
		if(GetFree(&stUsr.stUpload.stQue)) {
			String cnt = "temp";
			String con   = "\"?\"";
			
			if(!isnan(stUsr.stTemp.fPv)) {
				char rqi[10];

				con = String(stUsr.stTemp.fPv);
				con = "\"" + con + "\"";

				rand_str(rqi, 8); // 랜덤으로 8개 문자&숫자 생성
				stUsr.stUpload.stQue.sRef[stUsr.stUpload.stQue.PushIdx] = "/" + String(stUsr.stFlash.aCbName) + "/" + String(stUsr.stFlash.aAeName) + "/" + cnt;
				stUsr.stUpload.stQue.sCon[stUsr.stUpload.stQue.PushIdx] = con;
				stUsr.stUpload.stQue.sRqi[stUsr.stUpload.stQue.PushIdx] = String(rqi);

				if(++stUsr.stUpload.stQue.PushIdx >= QueueSize)		stUsr.stUpload.stQue.PushIdx = 0; // 로컬에다 넣고 데이터가 있으면 메시지를 보낸다?
#if _DEBUG_
				Serial.printf("\tTemp : %.2f\'C", stUsr.stTemp.fPv);
				Serial.printf("\tFifo GetSize : %d, GetFree : %d\n", GetSize(&stUsr.stUpload.stQue), GetFree(&stUsr.stUpload.stQue));
#endif
			}
#if _DEBUG_
			else	Serial.printf("\tisnan\tTemp : %.2f\'C, Adj : %.2f\n", stUsr.stTemp.fPv, stUsr.stTemp.fAdj);
#endif
		}
#if _DEBUG_
		else	Serial.printf(" Fifo full...\n");
#endif
	}
	else	Serial.println(" temp - not connected");
}

#define HumiTime	60000
void Humi_Task(void)
{
	if(stUsr.stWifi.eStat == eWiFi_Stat_Connected) {
#if _DEBUG_
		Serial.printf("%s():%d\n", __func__, __LINE__);
#endif
		if(GetFree(&stUsr.stUpload.stQue)) {
			String cnt = "humid";    // 2023.09.08 cnt 이름 일치(humid -> humi)
			String con = "\"?\"";
			
			if(!isnan(stUsr.stHumi.fPv)) {
				char rqi[10];

				con = String(stUsr.stHumi.fPv);
				con = "\"" + con + "\"";

				rand_str(rqi, 8);
				stUsr.stUpload.stQue.sRef[stUsr.stUpload.stQue.PushIdx] = "/" + String(stUsr.stFlash.aCbName) + "/" + String(stUsr.stFlash.aAeName) + "/" + cnt;
				stUsr.stUpload.stQue.sCon[stUsr.stUpload.stQue.PushIdx] = con;
				stUsr.stUpload.stQue.sRqi[stUsr.stUpload.stQue.PushIdx] = String(rqi);

				if(++stUsr.stUpload.stQue.PushIdx >= QueueSize)		stUsr.stUpload.stQue.PushIdx = 0;
#if _DEBUG_
				Serial.printf("\tHumi : %.2f %%", stUsr.stHumi.fPv);
				Serial.printf("\tFifo GetSize : %d, GetFree : %d\n", GetSize(&stUsr.stUpload.stQue), GetFree(&stUsr.stUpload.stQue));
#endif
			}
#if _DEBUG_
			else	Serial.printf("\tisnan\tHumi : %.2f %%, Adj : %.2f\n", stUsr.stHumi.fPv, stUsr.stHumi.fAdj);
#endif
		}
#if _DEBUG_
		else	Serial.printf(" Fifo full...\n");
#endif
	}
	else	Serial.println(" humi - not connected");
}

#define DispTime	1000
#if (Use_SH1106G | Use_SSD1306)
// QR code 생성: OLED: 128x32
void drawQrCode(const char* qrStr, const char* lines[])
{
	uint8_t qrcodeData[qrcode_getBufferSize(3)];
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);						
#endif
	qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrStr);
#if 0
	// Text starting point
	int cursor_start_y = 0; // 글자 시작 oled x 위치
	int cursor_start_x = 0; // 글자 시작 oled y 위치
	int font_height    = 8; // 글씨 크기 (OLED의 글자 기본 높이: 7)
#endif
	// QR Code Starting Point
	int offset_x       = 95;  // QR code 시작 oled x 위치
	int offset_y       =  0;  // OR code 시작 oled y 위치
#if 1
	for(int y = 0; y < qrcode.size; y++) {
		for(int x = 0; x < qrcode.size; x++)
			display.fillRect(offset_x + x, offset_y + y, 1, 1, 1 - qrcode_getModule(&qrcode, x, y));
	}
#else
	for(int y = 0; y < qrcode.size; y++) {
		for(int x = 0; x < qrcode.size; x++) {
			int newX = offset_x + (x * 1);
			int newY = offset_y + (y * 1);

			display.fillRect(newX, newY, 2, 2, 1 - qrcode_getModule(&qrcode, x, y));
		}
	}
#endif
}

void Disp_Task(void)
{
	int nTmp[2];
	static unsigned char toggle = 0;
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);						
#endif
    // 온습도값 표시
	display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);

    display.setCursor(1, 1); // 온도 표시 시작지점
    // oled 디스플레이 표시 자리수 
	nTmp[0] = stUsr.stTemp.fPv / 10;
    if(stUsr.stTemp.fPv < 0) {  // 온도값이 -값일 때,
      if(nTmp[0] == 0)		display.println("T "  + String(stUsr.stTemp.fPv, 1) + "C");	// 일의 자리  // T: -2.3C  
	  else					display.println("T"   + String(stUsr.stTemp.fPv, 1) + "C");	// 십의 자리  // T:-12.3C  
    }
	else {  // 온도값이 +값일 때, oled 디스플레이 표시
      if(nTmp[0] == 0)		display.println("T  " + String(stUsr.stTemp.fPv, 1) + "C");	// 일의 자리  // T:  2.3C  
	  else					display.println("T "  + String(stUsr.stTemp.fPv, 1) + "C"); // 십의 자리  // T: 12.3C  
    }
    
    display.setCursor(1, 30); // 습도 표시 시작지점
	nTmp[0] = stUsr.stHumi.fPv / 10;
	nTmp[1] = stUsr.stHumi.fPv / 100;

    if(stUsr.stHumi.fPv < 0){ // 습도값이 -값일 때,
      if(nTmp[0] == 0)		display.println("H "  + String(stUsr.stHumi.fPv, 1) + "%");  // 일의 자리	// H: -2.3C  
      else					display.println("H"   + String(stUsr.stHumi.fPv, 1) + "%");  // 십의 자리	// H:-12.3C  
    }
	else{          // 습도값이 +값일 때,
      if(nTmp[0] == 0)		display.println("H  " + String(stUsr.stHumi.fPv, 1) + "%");  // 일의 자리	// H:  2.3%
      else if(nTmp[1] == 0)	display.println("H "  + String(stUsr.stHumi.fPv, 1) + "%");  // 십의 자리	// H: 12.3%  
      else					display.println("H"   + String(stUsr.stHumi.fPv, 1) + "%");  // 백의 자리	// H:100.0%
    }
	
	display.setTextSize(2);
    display.setTextColor(WHITE);
	display.setCursor(124, 58);

	if(toggle)	display.drawCircle(124, 58, 3, WHITE);
	else		display.fillCircle(124, 58, 3, WHITE);

	toggle = 1 - toggle;

	drawQrCode("HTTP://www.hynux.co.kr", MESSAGE_OPEN_WEBAPP);
	
	if(stUsr.stUpload.bDisplay) {
		stUsr.stUpload.bDisplay = 0;

		display.setTextSize(1);
		display.setCursor(1, 54);
		display.println("Uploading...");
	}

    display.display();
}
#endif

void UpdatePv(void)
{
	float fTmp;
#if _DEBUG_
	Serial.printf("%s():%d\n", __func__, __LINE__);						
#endif
	fTmp = dht22.readTemperature();
#if _DEBUG_
	Serial.printf(" temp : %.2f\n", fTmp);
#endif
	if(!isnan(fTmp))	stUsr.stTemp.fPv = fTmp + stUsr.stTemp.fAdj;
	else				stUsr.stTemp.fPv = -99.9;

	fTmp = dht22.readHumidity();
#if _DEBUG_
	Serial.printf(" humi : %.2f\n", fTmp);
#endif
	if(!isnan(fTmp))	stUsr.stHumi.fPv = fTmp + stUsr.stHumi.fAdj;
	else				stUsr.stHumi.fPv = -99.9;
}

void loop()
{
	       unsigned long curr;
	static unsigned long sPrev = 0;
	static unsigned char sec = 0;

  // put your main code here, to run repeatedly:
  	WiFi_Task(); //wifi 연결상태 체크
	Mqtt_Task(); // 건드리는 게 아님. mqtt로 데이터를 주고 받을 때 loop를 계속 타야지 이 함수가 실제로 서버에서 온 메시지를 받아서 topic에 담는 함수를 호출해준다. 따라서 연결상태 계속 체크

	switch(stUsr.stnCube.eMode) {
		case eCreate_Cin :
			if(stUsr.stWifi.eStat == eWiFi_Stat_Connected) {
				curr = millis();
				if(curr - sPrev >= DispTime) {
					sPrev = curr;

					if(sec & 0x1)	digitalWrite(ledPin, HIGH);
					else			digitalWrite(ledPin, LOW);

					UpdatePv(); // 1초마다 현재 온습도 update
		#if (Use_SH1106G | Use_SSD1306)
					Disp_Task(); // 온습도 display
		#endif
					if(++sec >= 60) {
						sec = 0;

						Temp_Task(); // 1분마다 온도 topic에 맞게 저장
						Humi_Task(); // 1분마다 습도 topic에 맞게 저장
					}
				}
			}
			else	digitalWrite(ledPin, LOW);

			Upload_Task(); // 서버에 upload
			Noti_Task(); // 제어를 위한 Mobius 프로세스 알림
//			ToggleLed();
			break;
		default :
			Publisher(); // 상태를 display
			break;
	}
}
