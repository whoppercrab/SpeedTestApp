#include <stdlib.h>
#include <stdio.h>
#include <mosquitto.h>
#include <time.h>
#include <Windows.h>
struct mosquitto *mosq;
char broker_IP[16];// = "192.168.10.56"; //브로커 IP
int broker_port = 1883; //브로커 port
void on_message(struct mosquitto * mosq, void *obj, const struct mosquitto_message *message);
void MosquittoStart();
void send_to_ap(char* payload, char* topic);
char* timeToString(struct tm *t);
char query[10];
FILE * fp,*stream,*config;
int count, max_cnt;
struct tm *t;
time_t timer;
int main()
{





	//printf("%s\n", timeToString(t));
	char temp[20];
	
	config = fopen("config.txt","r");
	if (!config)
	{
		printf("error config\n");
		exit(0);
	}
	int cnt = 0;
	while (fgets(temp, 16, config))
	{	
			strcpy(broker_IP, temp);
	

		memset(temp, 0x00, 20);
	//	cnt++;
	}
	//printf("%", broker_IP);
	//printf("%d\n", max_cnt);
	MosquittoStart();
}
void send_to_ap(char* payload, char* topic)
{
	mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 1, NULL);
}
void start_iperf()
{
	size_t      readSize = 0;
	char        buff[1024];
	memset(buff, 0x00, 1024);
	printf("다운로드 속도측정중..\n\n");
	fprintf(fp, "[%s] 다운로드 속도측정중..\n",timeToString(t));
	stream = _popen("iperf3.exe -c 192.168.1.107 -b 100M -t 180","r");
	if (!stream)
	{
		printf("popen error\n");
	}

	// 결과값 읽기
	while (fgets(buff, 1024, stream))
		fprintf(fp, buff);

	_pclose(stream);
#if 0
	printf("업로드 속도측정중..\n\n");
	fprintf(fp, "[%s] 업로드 속도측정중..\n",timeToString(t));
	stream = _popen("iperf3.exe -c 192.168.1.107  -b 100M -R -t 3600", "r");
	if (!stream)
	{
		printf("popen error\n");
	}

	// 결과값 읽기
	while (fgets(buff, 1024, stream))
		fprintf(fp, buff);

	_pclose(stream);
#endif
}
void start_func(char *message)
{
	fp = fopen("data.txt", "a+");
	if (fp == NULL)
	{
		printf("fopen error\n");
	}
	

	switch (message[0])
	{
	case '0': //스타트 메세지
		count = 0;
		send_to_ap("13", "AP"); //채널 3번
		send_to_ap("20", "AP"); //자동채널 해제
		send_to_ap("30", "AP"); // 확장채널 0
		send_to_ap("40", "AP"); //  20MHZ
		send_to_ap("5", "AP"); //  무선초기화
		fprintf(fp, "[%s] 채널 3번 20Mhz 설정\n", timeToString(t));
		break;
	case '1':
		send_to_ap("17", "AP"); //채널 7번
		send_to_ap("5", "AP"); // 무선초기화
		fprintf(fp, "[%s] 채널 7번 20Mhz 설정\n", timeToString(t));
		break;
	case '2':
		send_to_ap("113", "AP"); //채널 13번
		send_to_ap("5", "AP"); // 무선초기화
		fprintf(fp, "[%s] 채널 13번 20Mhz 설정\n", timeToString(t));
		break;
	case '3':
		send_to_ap("13", "AP"); //채널 3번
		send_to_ap("20", "AP"); //자동채널 해제
		send_to_ap("31", "AP"); // 확장채널 1
		send_to_ap("41", "AP"); //  40MHZ
		send_to_ap("5", "AP"); // 무선초기화
		fprintf(fp, "[%s] 채널 3번 40Mhz 설정\n", timeToString(t));
		break;
	case '4':
		send_to_ap("17", "AP"); //채널 7번
		send_to_ap("31", "AP"); // 확장채널 1
		send_to_ap("41", "AP"); //  40MHZ
		send_to_ap("5", "AP"); // 무선초기화
		fprintf(fp, "채널 7번 40Mhz 설정\n", timeToString(t));
		break;
	case '5':
		send_to_ap("113", "AP"); //채널 13번
		send_to_ap("30", "AP"); // 확장채널 1
		send_to_ap("41", "AP"); //  40MHZ
		send_to_ap("5", "AP"); //  무선초기화
		fprintf(fp, "[%s] 채널 13번 40Mhz 설정\n", timeToString(t));
		break;
	case '6':

		break;

	}

	fclose(fp);
}

void on_message(struct mosquitto * mosq, void *obj, const struct mosquitto_message *message)
{
	printf("============================================================================================\n");
	printf("mid | payload | payloadlen |qos | retain | topic \n");
	printf("%d  | %s      | %d  | %d  | %d  | %s \n", message->mid, (char*)message->payload, message->payloadlen, message->qos, message->retain, message->topic);
	printf("============================================================================================\n");
	
	if ((strcmp((char*)message->topic, "AP/START")==0))
	{
		start_func((char *)message->payload);
	}
	else if ((strcmp((char*)message->topic , "AP/OK")==0))
	{
		fp = fopen("data.txt", "a+");
		if (fp == NULL)
		{
			printf("fopen error\n");
		}
		fprintf(fp, "[%s] 설정 완료 !\n",timeToString(t));
		//for(int i=0; i<max_cnt; i++)
		Sleep(30000);
		start_iperf();
		fclose(fp);
		
		count++;
		char str[2];
		memset(str, 0x00, strlen(str));
		sprintf(str, "%d", count);
		send_to_ap(str, "AP/RESTART");
		
	}
	else if ((strcmp((char*)message->topic , "AP/ERROR")==0))
	{
		fp = fopen("data.txt", "a+");
		if (fp == NULL)
		{
			printf("fopen error\n");
		}
		printf("error\n");
		fprintf(fp, "[%s] 설정 ERROR!!", timeToString(t));
		fclose(fp);
	}


}
void MosquittoStart()
{
	printf("Mosquitto Start..\n");
	//printf("%s", broker_IP);
	fp = fopen("data.txt", "a+");
	if (fp == NULL)
	{
		printf("fopen error\n");
	}
	else
	{
		fprintf(fp, "[%s] start\n", timeToString(t));
	}
	fclose(fp); 

	if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS)
	{
		printf("init error");
	}
	mosq = mosquitto_new("test", true, NULL);
	if (mosquitto_connect(mosq, broker_IP, broker_port, false) != MOSQ_ERR_SUCCESS)
	{
		printf("connect error\n");
	}
	else
	{
		printf("connect to broker\n broker \nBroker info IP:%s , PORT: %d\n", broker_IP, broker_port);
	}
	if (mosquitto_subscribe(mosq, NULL, "AP/+", 0) != MOSQ_ERR_SUCCESS)
	{
		printf("subscribe error\n");
	}
	mosquitto_message_callback_set(mosq, on_message);
	mosquitto_loop_forever(mosq, 1000, 1);
}
char* timeToString(struct tm *t) {
	static char s[20];

	timer = time(NULL);    // 현재 시각을 초 단위로 얻기
	t = localtime(&timer); // 초 단위의 시간을 분리하여 구조체에 넣기
	sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec
	);

	return s;
}