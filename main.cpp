/*
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 * Copyright 2018 Intel Corporation All Rights Reserved.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#include <signal.h>
#include <string>
#include <iostream>

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "Platform.h"
#include "DeviceServer.h"
#include "HumiResource.h"
#include "TempResource.h"

#include "json/json.h"

//#include "BinarySwitchResource.h"

// main application
//
// starts the server
//
//
#define SHT_DEVICE_ID 0x44


#define SIZE 256
#define KEY 1234


typedef struct{
    long mtype;
    char data[SIZE];
} msg_data;


int pid;
int msgid; // queue_id
void* recv_queue(void * arg){
  
    
    //pid = fork();
    pid = 9999;
    //if(pid <0){
    //	std::cout << "fork() Error" << std::endl;
    //}

    
    msgid = msgget((key_t)KEY,IPC_CREAT|0666);
    if(msgid == -1){
	std::cout << "msgget() Error" << std::endl;
    }

    //std::cout << "pid = "<< pid << std::endl;
    std::cout << "msgid = "<< msgid << std::endl;


    msg_data msg;
    int msg_size = 0;
    msg_size = sizeof(msg) - sizeof(msg.mtype);


    Json::Value root;
    Json::Reader reader;
    

    while(1){
		
	if (msgrcv(msgid,&msg,msg_size,7777,0) > 0){
	//if (msgrcv(msgid,&msg,msg_size,0,IPC_NOWAIT) > 0){
	    std::cout << "msg_data_ mtype :"<< msg.mtype << std::endl;
	    

	    std::string data(msg.data);


	    std::cout << "data : "<< data<< std::endl;
	    
	    bool success =   reader.parse(data,root);

	    if(!success){
		std::cout << "json parse Error"<< data << std::endl;
	    }
    

	    std::cout << "JSON DATA "<< root["parameter1"] << std::endl;

	    if(root.isMember("parameter1")){
		std::cout << "@@@@@@@@@@@@@@@ p1"<< std::endl;
	    }
	    
	    if(root.isMember("parameter5")){
		std::cout << "@@@@@@@@@@@@@@@ p5"<< std::endl;
	    }

	    /*for( int i = 0 ; i < root.size(); i++){
		std::cout << root[i] << std::endl;
	    }*/
	    //std::cout << "msg_data_ :"<< data << std::endl;

	}
	//sleep(1);
    }
}

void* startI2C(void * arg){

    //DeviceServer *ds = static_cast<DeviceServer*>(arg);    
    DeviceServer *ds = (DeviceServer*)arg;
    int fd = -1;

    ds ->m_tempInstance.sendNotification();
    /*fd = wiringPiI2CSetup(SHT_DEVICE_ID);

    if(fd != -1){
	char config[2] = {0};
	config[0] = 0x24;
	config[1] = 0x0b;
	
	char i2c_rx_buf[6] = {0};

	while(1){
	    write(fd,config,2);
	    sleep(1);
	    read(fd,i2c_rx_buf,6);
	    float temp = (float)((float)175.0 * (float)(i2c_rx_buf[0] * 0x100 + i2c_rx_buf[1]) / (float)65535.0 - 45.0);
	    float humi = (float)((float)100.0 * (float)(i2c_rx_buf[3] * 0x100 + i2c_rx_buf[4]) / (float)65535.0);
	    
	    std::cout << temp << std::endl;
	    std::cout << humi << std::endl;
	    ds -> m_humiInstance.setHumi((int)humi);
	    ds -> m_tempInstance.setTemp((int)temp);


	    ds ->m_humiInstance.sendNotification();
	    ds ->m_tempInstance.sendNotification();
	    //std::cout <<"tttttt\n" << ds -> m_humiInstance.getHumi()<< std::endl;



	}
    }*/

}

int main(void)
{
    Platform platform;
    if (OC_STACK_OK != platform.start())
    {
        std::cerr << "Failed to start the IoTivity platform." << std::endl;
        return 1;
    }
    // initialize "oic/p"
    std::cout << "oic/p" << std::endl;
    if (OC_STACK_OK != platform.registerPlatformInfo())
    {
        std::cerr << "Failed Platform Registration (oic/p)." << std::endl;
    }
    // initialize "oic/d"
    std::cout << "oic/d" << std::endl;
    if (OC_STACK_OK != platform.setDeviceInfo())
    {
        std::cerr << "Failed Device Registration (oic/d)." << std::endl;
    }

    std::cout << "device type: " <<  platform.deviceType << std::endl;
    std::cout << "platformID: " <<  platform.getPlatformInfo()->platformID << std::endl;
    std::cout << "platform independent: " << platform.protocolIndependentID << std::endl;



   /* 
    DeviceServer ds;
    if (OC_STACK_OK != ds.registerResources())
    {
        std::cerr << "Failed to register resources server resources." << std::endl;
        return 1;
    }
    
    
    pthread_t threadId;
    pthread_create(&threadId,NULL,startI2C,&ds);
    */

    TempResource t;
    OCStackResult result = OC_STACK_ERROR;

    result =  t.registerResource();
    if(result != OC_STACK_OK){
        std::cout << "Failed to register TEMP resources." << std::endl;
	return 0;
    }


    HumiResource h;
    result = h.registerResource();

    if(result != OC_STACK_OK){
        std::cout << "Failed to register HUMI resources." << std::endl;
	return 0;
    }


 
    pthread_t threadId;
    pthread_create(&threadId,NULL,recv_queue,(void *)0);
    

     



    int fd = -1;
    fd = wiringPiI2CSetup(SHT_DEVICE_ID);
    
    if(fd != -1){
	char config[2] = {0};
	config[0] = 0x24;
	config[1] = 0x0b;
	
	char i2c_rx_buf[6] = {0};

	while(1){
	    write(fd,config,2);
	    sleep(1);
	    read(fd,i2c_rx_buf,6);
	    float temp = (float)((float)175.0 * (float)(i2c_rx_buf[0] * 0x100 + i2c_rx_buf[1]) / (float)65535.0 - 45.0);
	    float humi = (float)((float)100.0 * (float)(i2c_rx_buf[3] * 0x100 + i2c_rx_buf[4]) / (float)65535.0);
	    
	    //std::cout << temp << std::endl;
	    //std::cout << humi << std::endl;
	    
	    //t.setTemp(temp);
	    //h.setHumi(humi);
	    
	    t.sendNotification();
	    h.sendNotification();
	


	}



    }
    return 0;
}
