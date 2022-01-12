**detailed analysis in real time requirements**

Scenario 1: A base line scenario is to do read actions and
check the time it takes from the button press to the actual data
is received.
The following data was used:Time to wait after task: 50 / portTICK PERIOD MS; Time to wait for mutex: 2 /portTICK PERIOD MS; Total number of tasks: 3.
The following data was reseived: Fastest response: 1145.1 ms; Slowest response: 1586.3 ms; Average for 11 measurements: 1395.3 ms.
Scenario 2: Using the data from the previous scenario, the
second scenario will try to improve the timing by lowering the time it takes for the response by lowering the time to wait
between tasks.
In this scenario, the current solution failed to respond to the
requests. This test was run with the following parameters:Time to wait after task: 20 /portTICK PERIOD MS; Time to wait after task: 40 / portTICK PERIOD MS; Time to wait after task: 45 / portTICK PERIOD MS.
The device failed to resond event when the time to wait for
mutex was risen to 10 / portTICK PERIOD MS.
In this case, it is concludent for this scenario that using this
configuration from scenario 1 is the baseline for the project.
Scenario 3: In this scenario, a dummy task is introduced
that will simulate another pair of devices, an will occupy the
processing time for 50 / portTICK PERIOD MS.
The following data was used:Time to wait after task: 50 / portTICK PERIOD MS; Time to wait for mutex: 2 / portTICK PERIOD MS; Total number of tasks:4 ;Dummy tasks 1; Occupy time: 50 /portTICK PERIOD MS
The following data was received: Fastest response: 1305.8 ms; Slowest response: 1797.8 ms; Average for 11 measurements: 1514.1 ms
It is observed a small rise in all the timings, as expected.

**Block diagram of the system**

![image](https://user-images.githubusercontent.com/61541965/149101882-1296db1d-bba8-4c12-a216-471512f1ada5.png)

**product specifications**


The proposed system is composed of a LM35 temperature
sensor and a foto-resistor as inputs, an LED and a buzzer
as outputs, and a central processing unit composed of an
arduino MEGA2560 board. The system is implemented as a
fire detection system, since a fire emits heat and light. The
data colected from the sensors are sent via Serial to a control
software which can program the thresholds for the sensors and
to visualize the values.
