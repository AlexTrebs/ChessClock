// Include Libraries
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_GFX.h>
#include <time.h>
#include <GameState.h>
#include <stdint.h>
#include <LibPrintf.h>

// Pin Definitions
#define OLED128X64_1_PIN_RST 4
#define OLED128X64_1_PIN_DC	8
#define OLED128X64_1_PIN_CS	10

#define OLED128X64_2_PIN_DC	7
#define OLED128X64_2_PIN_CS	3 

#define PUSHBUTTON_1_PIN_2	5
#define PUSHBUTTON_2_PIN_2	6
#define PUSHBUTTON_3_PIN_2	9 
#define TOGGLESWITCH_1_PIN_2	A4
#define TOGGLESWITCH_2_PIN_2	A3

// Global variables and defines
int plus_button, minus_button, reset_button, player1_switch, player2_switch;
int32_t initial_time, player1_time, player2_time;
enum GameState game_state;
bool player1_turn, player1_white, player1_win;

const int16_t one_sec_ms = 1000;
const int32_t one_min_ms = (int32_t)one_sec_ms * 60;
const int32_t max_time_increase = 5 * one_min_ms;
const int16_t time_change_rate = 50;
const int32_t max_time = one_min_ms * 99;
const int32_t min_time = 0;

// object initialization
Adafruit_ST7789 screen1 = Adafruit_ST7789(OLED128X64_1_PIN_CS, OLED128X64_1_PIN_DC, -1);
Adafruit_ST7789 screen2 = Adafruit_ST7789(OLED128X64_2_PIN_CS, OLED128X64_2_PIN_DC, -1);


void testdrawtext(String text, uint16_t color, Adafruit_ST7789 screen); 
void(* resetFunc) (void) = 0; //declare reset function @ address 0

// Setup the essentials for your circuit to work. It runs first every time your circuit is powered with electricity.
void setup(void) 
{
  // Setup Serial which is useful for debugging
  // Use the Serial Monitor to view printed messages
  Serial.begin(9600);
  while (!Serial) ; // wait for serial port to connect. Needed for native USB
  Serial.println("Start\n");
  pinMode(PUSHBUTTON_1_PIN_2, INPUT_PULLUP);
  pinMode(PUSHBUTTON_2_PIN_2, INPUT_PULLUP);
  pinMode(PUSHBUTTON_3_PIN_2, INPUT_PULLUP);
  pinMode(TOGGLESWITCH_1_PIN_2, INPUT_PULLUP);
  pinMode(TOGGLESWITCH_2_PIN_2, INPUT_PULLUP);

  screen1.init(135, 240); 
  screen2.init(135, 240); 
  screen1.setRotation(3);
  screen2.setRotation(3);
  screen1.setSPISpeed(40000000);
  screen2.setSPISpeed(40000000);

  screen1.fillScreen(ST77XX_BLACK);
  testdrawtext("Chess",
    ST77XX_WHITE,
    screen1);
  delay(1000);
  
  screen2.fillScreen(ST77XX_BLACK);
  testdrawtext("Clock",
    ST77XX_WHITE,
    screen2);  
  delay(1000);
  screen1.fillScreen(ST77XX_BLACK);
  screen2.fillScreen(ST77XX_BLACK);

  initial_time = one_min_ms * 30; //Average chess game is 30 mins

  game_state = NOT_BEGUN;
  printf("Time initial_time %ld\n", initial_time);
  printf("AAAAAAAA %ld\n", (one_min_ms * 30));
  printf("BBBBBBBBBB %ld\n", one_min_ms);

}

void testdrawtext(String text, uint16_t color, Adafruit_ST7789 screen) {
  screen.setCursor(0, 0);
  screen.setTextColor(color);
  screen.setTextWrap(true);
  screen.setTextSize(8);

  screen.print(text);
}

String get_clock_time(int32_t time) {

  unsigned long totalSeconds = time / one_sec_ms;
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  
  String timeString = "";

  if (minutes < 10) {
    timeString += "0";
  }
  timeString += String(minutes);
  timeString += ":";

  if (seconds < 10) {
    timeString += "0";
  }
  timeString += String(seconds);

  return timeString;

}

int32_t get_mins(int32_t time) {

  return time/one_min_ms;

}

void update_timers(int32_t time_1, int32_t time_2) {
  static int32_t prev_time_1 = 0;
  static int32_t prev_time_2 = 0;
  
  if(prev_time_1/one_sec_ms != time_1/one_sec_ms){
    printf("time_1 %ld\n", time_1);
    String clock = get_clock_time(time_1);

    screen1.fillScreen(ST77XX_BLACK);

    testdrawtext(clock,
      ST77XX_WHITE,
      screen1);
  }
 
  if(prev_time_2/one_sec_ms != time_2/one_sec_ms){
    printf("time_2 %ld\n", time_2);
    String clock = get_clock_time(time_2);
    
    screen2.fillScreen(ST77XX_BLACK);

    testdrawtext(clock,
      ST77XX_WHITE,
      screen2);
  }

  prev_time_1 = time_1;
  prev_time_2 = time_2;
}

void setup_menu() {
  static int32_t time_pressed = 0;
  static int32_t old_time_pressed = 0;
  static bool time_increased = false;
  static bool time_decreased = false;
  static int32_t weight = 0;
  update_timers(initial_time, initial_time);

  // Start game when either player presses their switch
  if(player1_switch == LOW || player2_switch == LOW){
    printf("Game starting.\n");
    printf("Time : %ld\n", initial_time);

    player1_turn = player1_white = player1_switch == LOW;
    player1_time = player2_time = initial_time;

    time_pressed = 0;
    game_state = INPROGRESS;
  }

  //If time is increased/decreased
  if ((plus_button == LOW || minus_button == LOW) && minus_button != plus_button) {
    unsigned long start_time = millis();

    weight = one_min_ms * (int32_t)(time_pressed/time_change_rate + 1);

    // Limits the number of increases/decreases to once per second
    if((int32_t)(time_pressed/time_change_rate) > (int32_t)(old_time_pressed/time_change_rate) || time_pressed == 0) {
      if(plus_button == LOW && initial_time <= max_time){
        if(time_increased != true) {
          old_time_pressed = time_pressed;
          time_pressed = (int32_t)(millis() - start_time);

          time_increased = true;
          time_decreased = false;
          weight = one_min_ms;
        }

        initial_time += weight >= max_time_increase ? max_time_increase : weight;
        printf("%ld\n", initial_time);
        printf("%ld\n", (weight >= max_time_increase ? max_time_increase : weight));
        if(initial_time > max_time) {
          initial_time = max_time;
        }
      }
      else if(minus_button == LOW && initial_time > min_time){
        if(time_decreased != false) {
          old_time_pressed = time_pressed;
          time_pressed = (int32_t)(millis() - start_time);

          time_increased = false;
          time_decreased = true;
          weight = one_min_ms;
        }

        initial_time -= weight >= max_time_increase ? max_time_increase : weight;
        printf("%ld\n", initial_time);
        printf("%ld\n", (weight >= max_time_increase ? max_time_increase : weight));

        if(initial_time < min_time) {
          initial_time = min_time;
        }
      }
    }

    old_time_pressed = time_pressed;
    time_pressed += (int32_t)(millis() - start_time);
  }

  if (plus_button == HIGH && minus_button == HIGH) {
    time_pressed = 0;
  }
}



int32_t update_time(int32_t player_time, int32_t time_passed) {

  return player_time - time_passed < 0 ? 0 : player_time - time_passed;

}


void inprogress_game() {
  static int32_t move_time = 0;
  static unsigned long start_time = millis();
   
  // If neither players are in turn, but game is still in-progress
  if(player1_switch == HIGH && player2_switch == HIGH) {
    game_state = PAUSED;
    printf("p1 : %d\n", player1_switch);
    printf("p2 : %d\n", player2_switch);

    printf("Game has been paused.\n");
  }

  // Update who's current turn it is
  if(player1_switch == LOW || player2_switch == LOW) {
    player1_turn = player1_switch != LOW;
    printf(
      "It is player %d's turn, there is %ld, and %ld time left on player 1 and player 2 respectively.\n", 
      player1_switch != LOW ? 1 : 2, 
      player1_time,
      player2_time
    );
  }

  // Update that player's timer 
  move_time = (int32_t)(millis() - start_time);
  start_time = millis();

  if (player1_turn) {
    player1_time = update_time(player1_time, move_time);
  } else {
    player2_time = update_time(player2_time, move_time);
  }
  
  update_timers(player1_time, player2_time);

  // When someone runs out of time
  if(player1_time <= 0 || player2_time <= 0) {
    game_state = FINISHED;
    player1_win = player1_time > 0;
  }

  move_time = 0;
}

void check_reset(){
  static int oldResetButtonValue = 0;
  static unsigned long startSequenceTime;

  // When reset button pressed
  if(reset_button == LOW) {
    if( reset_button != oldResetButtonValue )
    { 
      // Start 5 second timer
      startSequenceTime = millis() + one_sec_ms * 5;
    } 
    else {
      // If reset button is held for more than 5 seconds
      if( millis() >= startSequenceTime) {
        resetFunc();
      }
    }
  }

  oldResetButtonValue = reset_button;
}

// Main logic of your circuit. It defines the interaction between the components you selected. After setup, it runs over and over again, in an eternal loop.
void loop() 
{
  plus_button = digitalRead(PUSHBUTTON_1_PIN_2);
  minus_button = digitalRead(PUSHBUTTON_2_PIN_2);
  reset_button = digitalRead(PUSHBUTTON_3_PIN_2); 
  player1_switch = digitalRead(TOGGLESWITCH_1_PIN_2);
  player2_switch = digitalRead(TOGGLESWITCH_2_PIN_2);
  printf("%d", plus_button);
  check_reset();
  // Intialise intial start time
  if(game_state == NOT_BEGUN) {
    setup_menu();
  }
  else if(game_state == PAUSED) {
    printf("p1 : %d\n", player1_switch);
    printf("p2 : %d\n", player2_switch);

    // Update who's current turn it is
    if(player1_switch == LOW || player2_switch == LOW) {
      game_state = INPROGRESS;
    }
  }
  else if(game_state == INPROGRESS) {
    inprogress_game();
  }
  else if(game_state == FINISHED) {
    //update_timers(player1_time, player2_time);
    printf("Player %d wins through timeout!\nReset to play again nerd!", (player1_win ? 1 : 2));
    delay(one_sec_ms);

    screen1.fillScreen(ST77XX_BLACK);
    screen2.fillScreen(ST77XX_BLACK);

    testdrawtext("Won", ST77XX_WHITE, (player1_win ? screen1 : screen2));
    testdrawtext("Lost", ST77XX_WHITE, (!player1_win ? screen1 : screen2));
  }  
}
