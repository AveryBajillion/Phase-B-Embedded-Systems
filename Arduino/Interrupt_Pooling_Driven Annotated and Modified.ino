#define BUTTON_1  2   //Lines 1-13 define pins for use with the increment/decrement buttons
#define BUTTON_2  3   //,their LEDS and the outputs sent to the shift register and eventually 7-segment display
#define GREEN_LED 4
#define RED_LED   5

#define DATA      9   //74HC595  pin 8 DS
#define LATCH     8   //74HC595  pin 9 STCP
#define CLOCK     7  //74HC595  pin 10 SHCP

#define DIGIT_4   10
#define DIGIT_3   11
#define DIGIT_2   12
#define DIGIT_1   13

//Lines 17-19 form a table that contains the hexadecimals needed to represent digits '0-9'
//By indexing the table, these digits can be outputted to the shift register and 7-segment display
unsigned char table[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c
,0x39,0x5e,0x79,0x71,0x00};

unsigned char count = 0;
unsigned int buttonState = 0;  // variable for reading the pushbutton status
  char input_wifi[] 
//interrupt service routine flags need to be volatile for their augmentation throughout execution
volatile unsigned char isr_1_flag = 0;
volatile unsigned char isr_2_flag = 0;
volatile unsigned char isr_3_flag = 0;

void setup() {
  
  // Both the GREEN & RED LEDS are assigned outputs
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Lines 36-39 parameterize the buttons as inputs them ties the ISRs to trigger when their state changes
  pinMode(BUTTON_1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), Button_1_ISR, CHANGE);
  pinMode(BUTTON_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), Button_2_ISR, CHANGE);

  // 7-Seg Display
  pinMode(DIGIT_1, OUTPUT);
  pinMode(DIGIT_2, OUTPUT);
  pinMode(DIGIT_3, OUTPUT);
  pinMode(DIGIT_4, OUTPUT);

  // Shift Register Pins
  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);

  disp_on();

  // Lines 54-63 establish a real time clock by scaling the internal clock to one second, then generating an interrupt every second
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 62500;            // compare match register 16MHz/256
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

void Display(unsigned char num) //lines 66-72 index the table containing the digit representations then send that serially to the shift register
{

  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, MSBFIRST, table[num]);
  digitalWrite(LATCH, HIGH);
}

void Display_Numbers()
{
  Display(count);
  //Lines 77-82 by referencing the above block with the exceptiont that when the count exceeds hexadecimal representation, it resets to '0'
  //Reset Counter
  if(count == 16) 
  {
    count = 0;
  }
}

void disp_on()
{
  //lines 85-91 are part of the initialization for the peripherals that send count numbers to the shift register
   digitalWrite(DIGIT_1, LOW);
   digitalWrite(DIGIT_2, LOW);
   digitalWrite(DIGIT_3, LOW);
   digitalWrite(DIGIT_4, LOW);
}
//Lines 94-110 declare ISRs for use in the main, ISRs for the buttons augment the count via user input
//, the third ISR increments the count via the internal clock
void Button_1_ISR()
{
  // Set ISR Flag
  isr_1_flag = 1;
}

void Button_2_ISR()
{ 
  // Set ISR Flag
  isr_2_flag = 1;
}

ISR(TIMER1_COMPA_vect) // timer compare interrupt service routine
{
  // Set ISR Flag
  isr_3_flag = 1;
}
//Lines 115-156 entail the main code which augments the count via the interrupts from the buttons or from the real time internal clock
//serial.begin(9600) //initializes UART with ESP8266

void loop() 
{ 
  // Combination of Pooling and Interrupt

  // Attend Button 1
  if(isr_1_flag == 1)
  {
    // Reset ISR Flag
    isr_1_flag = 0;

    // Code
    //reading the button measures its state to look for changes
    //writing to the LED signals the input to the user
    buttonState = digitalRead(BUTTON_1);
    digitalWrite(RED_LED, buttonState);
    count = count + 1;  //increments counts
  }

  // Attend Button 2
  if(isr_2_flag == 1)
  {
    // Reset ISR Flag
    isr_2_flag = 0;

    // Code
    buttonState = digitalRead(BUTTON_2);
    digitalWrite(GREEN_LED, buttonState);
    count = count - 1;  //decrements count
  }

  // Attend 7-Seg Display
  if(isr_3_flag == 1)
  {
    // Reset ISR Flag
    isr_3_flag = 0;

    // Code
    count++;
    Display_Numbers();  //sends the 'count' to be processed by the 'display' function then sent to 'Display_Numbers' to protect against edge cases
                        //the count is incremented and displayed every second, the display only responds to the interrupts driven by the clock
  }
}
