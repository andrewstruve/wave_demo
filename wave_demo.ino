#define DAC_PIN 25
#define ADC_PIN 39

#define SQUARE_WAVE 0
#define PULSE_WAVE 1
#define SINE_WAVE 2
#define TRIANGLE_WAVE 3
#define SAWTOOTH_WAVE 4

#define FUNCTION_DRIVEN 0
#define TABLE_DRIVEN 1

#define STATE_ON 1
#define STATE_OFF 0

uint8_t g_wave_type = 0;
float g_frequency = 1;
float g_cycle_time = 0.0;
float g_duty_cycle = 0.1;

void adc_read()
{
  uint16_t val;
  val = analogRead(ADC_PIN);
  //Serial.printf("%d, %d\r\n", millis(), val);
  //Serial.println(val);
}
// at 10 Hz, 10 Cycles persecond
// 1 Cycle = 100 ms
// On time = 50 ms
// Off time = 50 ms
// 1 / (10*2)

void square_wave()
{
  static uint32_t last_event;
  static uint8_t state;
  
  if(state == STATE_OFF && millis() - last_event > (g_cycle_time / 2.0 ))
  {
    dacWrite(DAC_PIN, 255);
    last_event = millis();
    state = STATE_ON;
  }
  if(state == STATE_ON && millis() - last_event > (g_cycle_time / 2.0 ))
  {
    dacWrite(DAC_PIN, 0);
    last_event = millis();
    state = STATE_OFF;
  }
}
void pulse_wave()
{
  static uint32_t last_event;
  static uint8_t state;
  float duty_cycle = (g_duty_cycle/100.0);
  uint32_t off_time= (g_cycle_time * (1-duty_cycle));
  uint32_t on_time = (g_cycle_time * duty_cycle );

  Serial.printf("g_cycle_time = %5.3f, duty_cycle=%5.3f, off_time =%d on_time=%d\r\n", g_cycle_time, duty_cycle, off_time, on_time );
  
  if(state == STATE_OFF && millis() - last_event > off_time)
  {
    dacWrite(DAC_PIN, 255);
    last_event = millis();
    state = STATE_ON;
  }
  if(state == STATE_ON && millis() - last_event > on_time)
  {
    dacWrite(DAC_PIN, 0);
    last_event = millis();
    state = STATE_OFF;
  }
}
void sine_wave()
{
  uint32_t update_time = ( g_cycle_time / 256.02 );
  static uint32_t last_update;
  static uint8_t val;
  static float z_radians;
  static float sin_val;
  static uint8_t output;
  
  if(millis() - last_update >= update_time)
  {
    z_radians = 2.0 * 3.14159 * (val/256.0);
    // -1 to 1
    sin_val = sin(z_radians);
    // we need an output from 0 to 2
    output = 255*((sin_val +1)/2.0);
    //Serial.printf("z_rads=%5.3f, sin_val=%5.3f, output =%d\r\n", z_radians, sin_val, output);
    dacWrite(DAC_PIN, output); 
    last_update = millis();
    val++;
  }
}
#define TRIANGLE_COUNT_UP 0
#define TRIANGLE_COUNT_DOWN 1
void triangle_wave()
{
  // this gets us down to single digit milliseconds. Will need to use microseconds for this
  uint32_t update_time = ( g_cycle_time / 256.0 );
  static uint32_t last_update;
  static uint8_t val;
  static uint8_t state;

  //Serial.printf("update_time =%d\r\n", update_time);
  if(millis() - last_update >= update_time)
  {
    dacWrite(DAC_PIN, val); 
    
    if(val == 255)
    {
        state = TRIANGLE_COUNT_DOWN;
    }
    if(val == 0) 
    {
       state = TRIANGLE_COUNT_UP;
    }
  
    if( state == TRIANGLE_COUNT_UP) 
    {
      val++; 
    }
    else if (state == TRIANGLE_COUNT_DOWN)
    {
      val--; 
    }    
    last_update = millis();
  }

}
// bits should increase at a rate of ( g_cycle_time / 256 )
void saw_tooth_wave()
{
  uint32_t update_time = ( g_cycle_time / 256.0 );
  static uint32_t last_update;
  static uint8_t val;
  
  if(millis() - last_update >= update_time)
  {
    dacWrite(DAC_PIN, val); 
    val++; 
    last_update = millis();
  }
         
}

void printMenu()
{
  Serial.println("Select a wave using numbers 1 - 5");
  Serial.println("Change frequency using z and a, duty cycle (if applicable) using x and s");
  Serial.println("1 - Square Wave");
  Serial.println("2 - Pulse Wave");
  Serial.println("3 - Sine Wave");
  Serial.println("4 - Triangle Wave");
  Serial.println("5 - Sawtooth Wave");
  Serial.println("a - increase frequency by 10 Hz");
  Serial.println("z - decrease frequency by 10 Hz");
  Serial.println("s - increase duty cycle by 10 percent");
  Serial.println("x - decrease duty cycle by 10 percent");
  Serial.println("p - reprint this menu");
}
void processMenuInput()
{
  int incomingByte;
  char incomingChar;
  if (Serial.available() > 0) 
  {
    // read the incoming byte:
    incomingByte = Serial.read();
    incomingChar = (char) incomingByte;
    switch(incomingChar)
    {
      case '1':
        g_wave_type = 0;
        break;
      case '2':
        g_wave_type = 1;
        break;
      case '3':
        g_wave_type = 2;
        break;
      case '4':
        g_wave_type = 3;
        break;
      case '5':
        g_wave_type = 4;
        break;
      case 'a':
        if(g_frequency < 20.0)
          g_frequency += 1.0;
        break;
      case 'z':
        if(g_frequency > 1.0)
          g_frequency -= 1.0;
        break;
      case 's':
        if(g_duty_cycle < 100.0)
          g_duty_cycle += 10.0;
        break;
      case 'x':
        if(g_duty_cycle > 10.0)
          g_duty_cycle -= 10.0;
        break;
      case 'p':
        printMenu();
      default:
        break;
    }
  }
}
void wave_selector(uint8_t wave_type)
{
  g_cycle_time = 1000.0 / g_frequency;
  switch(wave_type)
  {
    case SQUARE_WAVE:
      square_wave();
      break;
    case PULSE_WAVE:
      pulse_wave();
      break;
    case SINE_WAVE:
      sine_wave();
      break;
    case TRIANGLE_WAVE:
      triangle_wave();
      break;
    case SAWTOOTH_WAVE:
      saw_tooth_wave();
      break;
    default:
      break;
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Wave Demo");
  printMenu();
}

void loop() {
  processMenuInput();
  wave_selector( g_wave_type);
  adc_read();
}
