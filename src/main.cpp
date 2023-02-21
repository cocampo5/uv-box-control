#include <TinyI2CMaster.h>
#include <Tiny4kOLED.h>

// defines
#define BTN_PIN PB3
#define TRANSITOR_GATE_PIN PB1
#define POT_PIN PB2
#define ONE_SECOND 1000.0
#define REFRESH_CYCLE 0.17
#define WIDTH 128
#define HEIGHT 64

// variables
boolean started = false;
double pos = 1.0;
double remainingTime = 0.0;
double lastMillis = 0.0;
double lastMillisScreen = 0.0;

void checkEncoder()
{
    int val = analogRead(POT_PIN);
    pos = map(val, 0, 1023, 1, 30);
}

char *convertToString(int v)
{
    static char buf[17];
    return itoa(v, buf, 10);
}

void reset()
{
    started = false;
    pos = 1.0;
    remainingTime = 0.0;
    lastMillis = 0.0;
}

void refreshScreen()
{
    long currentMillis = millis();
    if ((currentMillis - lastMillisScreen) >= (ONE_SECOND * REFRESH_CYCLE))
    {
        lastMillisScreen = millis();
        oled.clear();
    }
    if (!started)
    {
        oled.setCursor(0, 0);
        oled.print(F("SET: "));
        oled.setCursor(32, 0);
        oled.print(pos, 0);
    }
    else
    {
        oled.setCursor(0, 0);
        oled.print(F("LEFT: "));
        oled.setCursor(50, 0);
        oled.print(remainingTime / ONE_SECOND, 0);
        oled.setCursor(100, 0);
        oled.print(F("s"));
    }
}

ISR(PCINT0_vect) // Interrupt service routine
{
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 250)
    {
        started = !started;
    }
    last_interrupt_time = interrupt_time;
}

void setup()
{
    pinMode(BTN_PIN, INPUT_PULLUP);
    pinMode(TRANSITOR_GATE_PIN, OUTPUT);
    cli();              // disables interrupts
    GIMSK = 0b00100000; // turns on pin change interrupts
    PCMSK = 0b00001000; // turn on interrupts on pin PB1
    MCUCR = 0b00000010; // Configuring as falling edge
    sei();              // enables interrupts

    oled.begin(2, 0, WIDTH, HEIGHT, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
    oled.clear();
    oled.on();
    oled.setFont(FONT8X16);
}

void loop()
{
    if (started)
    {
        // TURN ON THE TRANSISTOR
        digitalWrite(TRANSITOR_GATE_PIN, HIGH);
        // wait
        if (remainingTime >= 0)
        {
            double currentMillis = millis();
            if ((currentMillis - lastMillis) >= ONE_SECOND)
            {
                lastMillis = millis();
                remainingTime = remainingTime - ONE_SECOND;
            }
        }
        else
        {
            // TURN OFF THE TRANSISTOR AND RESET
            digitalWrite(TRANSITOR_GATE_PIN, LOW);
            reset();
        }
    }
    else
    {
        digitalWrite(TRANSITOR_GATE_PIN, LOW);
        reset();
        checkEncoder();
        remainingTime = pos * 60.0 * ONE_SECOND;
    }
    refreshScreen();
}