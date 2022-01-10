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
   xTaskCreate(TaskPIRControl, "PIRControl",128, NULL, 1, NULL);
   xTaskCreate(TaskSerial, "SERIALControl", 240, NULL, 1, NULL);
   vTaskStartScheduler();
   
}

void loop() {}


void TaskPIRControl(void *pvParameters)
{
    int read_pir = 0;
    int pragPIR = 128;
    bool stare_pir = false;
    byte mesaj[3];
    for (;;){
    if (xSemaphoreTake(mutex, 100) == pdTRUE)
    {
      read_pir = analogRead(A0);

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
        //xTaskCreate(TaskLumina,"PIRControl_Lumina",   128, 1, 1, NULL);
        xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
        xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
        xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
        stare_pir = true;
      }
      else
        if (read_pir < pragPIR && stare_pir == true)
        {
           mesaj[0] = 0x05; mesaj[1] = 0x11; mesaj[2] = 0x00;
           //xTaskCreate(TaskLumina,"PIRControl_Lumina", 128, 0, 1, NULL);
           xQueueSend(xQueue,&mesaj[0],portMAX_DELAY);
           xQueueSend(xQueue,&mesaj[1],portMAX_DELAY);
           xQueueSend(xQueue,&mesaj[2],portMAX_DELAY);
           stare_pir = false;
        }
    xSemaphoreGive(mutex);   
    }

    vTaskDelay(10);
    }
}











void TaskSerial(void *pvParameters)
{
   Serial.begin(9600);
   byte header = 0x00;
   byte trash = 0x00;
   byte mesaj[3] = {0x00,0x00,0x00};

   byte buffer_serial_temp = 255;
   byte buffer_serial[3] = {0x00,0x00,0x00};
   int index = 0; 
   
   for (;;){
   if (xSemaphoreTake(mutex, 100) == pdTRUE)
   {
       xQueuePeek(xQueue,&mesaj[0],10);    // daca mesajul are ca destinatie Thread-ul de seriala, atunci descarca-l din queue
          if (mesaj[0] == 0x05) { 
             if (xQueueReceive(xQueue,&mesaj[0],10 ) == pdPASS){
                if (xQueueReceive(xQueue,&mesaj[1],10 ) == pdPASS){
                   if (xQueueReceive(xQueue,&mesaj[2],10 ) == pdPASS){
                       if (mesaj[1] == 0x12) {Serial.print("Light: " + String(mesaj[2]));} // In functie de sursa, se transmite valoarea citita cu prefix corespunzator.
                       if (mesaj[1] == 0x11) {Serial.print("Light state: " + String(mesaj[2]));} // In functie de sursa, se transmite valoarea citita cu prefix corespunzator.
                       if (mesaj[1] == 0x22) {Serial.print("Temp: "  + String(mesaj[2]));}
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
   vTaskDelay(10);
   }
}













