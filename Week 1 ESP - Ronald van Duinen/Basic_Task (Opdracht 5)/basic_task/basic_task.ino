#define LED1 12 //GPIO 12
#define LED2 13 // etc.
#define LED3 15
int looptimer = 0; // een counter die bijhoudt hoe vaak er al een seconde geweest is, globaal gedeclareerd omdat anders de loop de waarde zou resetten

struct s_led {
  byte gpio; // LED GPIO Number
  byte state; // Led state
  unsigned napms; // Delay to use (ms)
  TaskHandle_t taskh; // Task handle
};

static s_led leds[3] = {
  { LED1, 0, 500, 0 },
  { LED2, 0, 200, 0 },
  { LED3, 0, 750, 0 }
};

static void led_task_func(void *argp) {
  s_led *ledp = (s_led*)argp;
  unsigned stack_hwm = 0, temp;

  delay(1000);

  for (;;) {
    digitalWrite(ledp->gpio, ledp->state ^= 1);
    temp = uxTaskGetStackHighWaterMark(nullptr);
    if ( !stack_hwm || temp < stack_hwm ) {
      stack_hwm = temp;
      printf("Task for gpio %d has stack hwm %u\n",
             ledp->gpio, stack_hwm);
    }
    delay(ledp->napms);
  }
}


void setup() {
  int app_cpu = 0;  // CPU number
  delay(500);  // Pause for serial setup
  app_cpu = xPortGetCoreID();
  printf("app_cpu is %d (%s core)\n",
         app_cpu,
         app_cpu > 0 ? "Dual" : "Single");

  printf("LEDs on gpios: ");
  for ( auto& led : leds ) {
    pinMode(led.gpio, OUTPUT);
    digitalWrite(led.gpio, LOW);
    xTaskCreatePinnedToCore(
      led_task_func,
      "led_task",
      2048,
      &led,
      1,
      &led.taskh,
      app_cpu
    );
    printf("%d ", led.gpio);
  }
  putchar('\n');
}


void loop() {
  delay(1000); // Secondeteller
  looptimer++; // Zorgt ervoor dat de timer omhoog gaat per 1 seconde
  if (looptimer == 10) { // Conditie, als 10 seconden voorbij zijn dan:
      for ( auto& led : leds ) { // Loopt door elk ledje heen
        vTaskSuspend(led.taskh); // Stopt per ledje de taak
      }
      delay(3000); // De taak stopt voor 3 seconden
      looptimer = 0; // De timer wordt gereset
      for ( auto& led : leds ) { // Loopt weer door elke ledje heen
        vTaskResume(led.taskh); // Hervat de gestopte taken voor de ledjes
      }
    }
}
