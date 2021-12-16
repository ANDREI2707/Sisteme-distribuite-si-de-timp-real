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

   pinMode(Buzzer, OUTPUT);
   digitalWrite (Buzzer,LOW);
   pinMode(LED, OUTPUT);
   digitalWrite (LED,LOW);
   //Serial.println(initializare);
   pinMode(A0, INPUT);
   pinMode(A1, INPUT);
   xSemaphoreGive(mutex);
   xTaskCreate(TaskSerial, "SERIALControl",      240, NULL, 1, NULL);
   vTaskStartScheduler();
   
}

void loop() {}

void TaskSerial(void *pvParameters)
{
   Serial.begin(9600);
   byte header = 50;
   byte mesaj = 0x00;

   byte buffer_serial = 0x00;
   int index = 0; 
   
   for (;;){if (xSemaphoreTake(mutex, 100) == pdTRUE)
   {
       xQueuePeek(xQueue,&mesaj,10);
          if (mesaj == 50) {Serial.println("YEEEY");}
          
       buffer_serial = Serial.read();
       if (buffer_serial != 255){
           Serial.println(buffer_serial);
           xQueueSend(xQueue,&buffer_serial, portMAX_DELAY);
       }
       xSemaphoreGive(mutex);
   }
   vTaskDelay(10);
   }
}






////////// Testare echou de la PC -> Seriala -> Queue -> Seriala -> PC cu filtrare de valoare 50






