typedef struct t  {
    unsigned long tStart;
    unsigned long tTimeout;
};


t t_func1  = {0, 1000};
//const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = LOW;
int voltage = 0;
float wind = 0;
volatile int prev_count = 0;
volatile int current_count = 0;
//volatile int total_count = 0;

bool tCheck (struct t *t ) {
  if (millis() > t->tStart + t->tTimeout) return true;    
}

void tRun (struct t *t) {
    t->tStart = millis();
}

void setup() {
//  pinMode(
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
}

void loop() {

  if (tCheck(&t_func1 )) {
      func1();
      tRun(&t_func1 );
    }

  if (current_count>1000) {current_count =0; prev_count=0;};

  voltage = digitalRead(interruptPin);
  digitalWrite(LED_BUILTIN, state);
  Serial.print("Count:");
  Serial.println(current_count);

  Serial.print("Speed:");
  Serial.println(wind);

//  Serial.print("Prev:");
//  Serial.println(prev_count);
  
  delay(1000);
}

ICACHE_RAM_ATTR void blink() {
  state = !state;
  current_count++;
}

void func1(){
  wind = (current_count - prev_count)*0.03768;
  
  prev_count = current_count;
//  Serial.println(millis());
}
