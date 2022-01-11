#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

SemaphoreHandle_t mutex;
QueueHandle_t xQueue;

// Structura mesaj: Destinatie / Sursa / valoare
void TaskPIRControl(void *pvParameters);
void TaskLumina(void *pvParameters);

void TaskTempControl(void *pvParameters);
void TaskBuzzer(void *pvParameters);

void TaskSerial(void *pvParameters);

int Buzzer = 7;
int LED = 8;

void setup() {

   xQueue=xQueueCreate(10, sizeof(byte));
   mutex = xSemaphoreCreateMutex();

   Serial.begin(9600);
   pinMode(Buzzer, OUTPUT);
   digitalWrite (Buzzer,LOW);
   pinMode(LED, OUTPUT);
   digitalWrite (LED,LOW);
   //Serial.println(initializare);
   pinMode(A0, INPUT);
   pinMode(A8, INPUT);
   xSemaphoreGive(mutex);
   xTaskCreate(TaskPIRControl,  "PIRControl",    200, NULL, 2, NULL);
   xTaskCreate(TaskTempControl, "TEMPControl",   200, NULL, 2, NULL);
   xTaskCreate(TaskSerial,      "SERIALControl", 200, NULL, 2, NULL);
   vTaskStartScheduler();
   
}

void loop() {}


void TaskTempControl(void *pvParameters)
{
    int readTemp = 0;
    int pragTemp = 128;
    bool stareTemp = false;
    byte mesaj[3];
    for (;;){
    if (xSemaphoreTake(mutex, 2/ portTICK_PERIOD_MS) == pdTRUE)
    {

      readTemp = analogRead(A0);

      xQueuePeek(xQueue,&mesaj[0],10);    						// PEAK
      if (mesaj[0] == 0x21 || mesaj[0] == 0x22) {
        if (xQueueReceive(xQueue,&mesaj[0],10 ) == pdPASS){ // GET 1
        if (xQueueReceive(xQueue,&mesaj[1],10 ) == pdPASS){	// GET 2
        if (xQueueReceive(xQueue,&mesaj[2],10 ) == pdPASS){	// GET 3
        if ((mesaj[0] == 0x21) && (mesaj[1] == 0x05)) pragTemp = (int)mesaj[2];
        if ((mesaj[0] == 0x22) && (mesaj[1] == 0x05)){     
          mesaj[0] = 0x05; 		
		  mesaj[1] = 0x22; 
		  mesaj[2] = (byte)readTemp;
          xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
          xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
          xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
                     }
                  }
               }
            }
         }
      if (readTemp > pragTemp && stareTemp == false){
        mesaj[0] = 0x05; mesaj[1] = 0x21; mesaj[2] = 0x01;
        digitalWrite (Buzzer,HIGH);
        xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
        xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
        xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
        stareTemp = true;
      }
      else
        if (readTemp < pragTemp && stareTemp == true)
        {
           mesaj[0] = 0x05; mesaj[1] = 0x21; mesaj[2] = 0x00;
           digitalWrite (Buzzer,LOW);
           xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
           xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
           xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
           stareTemp = false;
        }
    xSemaphoreGive(mutex);   
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void TaskPIRControl(void *pvParameters)
{
    int read_pir = 0;
    int pragPIR = 128;
    bool stare_pir = false;
    byte mesaj[3];
    for (;;){
    if (xSemaphoreTake(mutex, 2 / portTICK_PERIOD_MS) == pdTRUE)
    {
      read_pir = analogRead(A8);

      xQueuePeek(xQueue,&mesaj[0],10);    // daca mesajul are ca destinatie Thread-ul de seriala, atunci descarca-l din queue
         if (mesaj[0] == 0x11 || mesaj[0] == 0x12) {
            if (xQueueReceive(xQueue,&mesaj[0],10 ) == pdPASS){
               if (xQueueReceive(xQueue,&mesaj[1],10 ) == pdPASS){
                  if (xQueueReceive(xQueue,&mesaj[2],10 ) == pdPASS){
                     if ((mesaj[0] == 0x11) && (mesaj[1] == 0x05)) pragPIR = (int)mesaj[2];
                     if ((mesaj[0] == 0x12) && (mesaj[1] == 0x05)){
                        mesaj[0] = 0x05; mesaj[1] = 0x12; mesaj[2] = (byte)read_pir;
                        xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
                        xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
                        xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
                     }
                  }
               }
            }
         }
      if (read_pir > pragPIR && stare_pir == false){
        mesaj[0] = 0x05; mesaj[1] = 0x11; mesaj[2] = 0x01;
        digitalWrite (LED , HIGH);
        xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
        xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
        xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
        stare_pir = true;
      }
      else
        if (read_pir < pragPIR && stare_pir == true)
        {
           mesaj[0] = 0x05; mesaj[1] = 0x11; mesaj[2] = 0x00;
           digitalWrite (LED,LOW);
           xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
           xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
           xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
           stare_pir = false;
        }
    xSemaphoreGive(mutex);   
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void TaskSerial(void *pvParameters)
{
   byte header = 0x00;
   byte trash = 0x00;
   byte mesaj[3] = {0x00,0x00,0x00};

   byte buffer_serial_temp = 255;
   byte buffer_serial[3] = {0x00,0x00,0x00};
   int index = 0; 
   
   for (;;){
   if (xSemaphoreTake(mutex, 2 / portTICK_PERIOD_MS) == pdTRUE)
   {
       xQueuePeek(xQueue,&mesaj[0],10);    // daca mesajul are ca destinatie Thread-ul de seriala, atunci descarca-l din queue
          if (mesaj[0] == 0x05) { 
             if (xQueueReceive(xQueue,&mesaj[0],10 ) == pdPASS){
                if (xQueueReceive(xQueue,&mesaj[1],10 ) == pdPASS){
                   if (xQueueReceive(xQueue,&mesaj[2],10 ) == pdPASS){
                       Serial.write(mesaj,3);
                   }
                }
             }
          }
          //else{
          //   for (int i = 0; i < 3; i++){if (xQueueReceive(xQueue,&trash,10 ) == pdPASS);}  // System de Scoatere din Queue a mesajelor gresite, doar pentru Debug. 
          //}
       mesaj[0] = 0x00; mesaj[1] = 0x00; mesaj[2] = 0x00;
       
       buffer_serial_temp = Serial.read();        // utilizatorul trebuie sa trimita 3 bytes obligatoriu pentru a forma mesajul // destinatie, sursa, valoare  
       if (buffer_serial_temp != 255) {buffer_serial[index] = buffer_serial_temp; buffer_serial_temp = 255; index++; }

       if (index == 3){    // Formare mesaj si plasare in Queue
           xQueueSend(xQueue,&buffer_serial[0], portMAX_DELAY);
           xQueueSend(xQueue,&buffer_serial[1], portMAX_DELAY);
           xQueueSend(xQueue,&buffer_serial[2], portMAX_DELAY);
           buffer_serial[0] = 255; buffer_serial[1] = 255; buffer_serial[2] = 255;
           index = 0;
       }
       xSemaphoreGive(mutex);
   }
   vTaskDelay(50 / portTICK_PERIOD_MS);
   }
}













