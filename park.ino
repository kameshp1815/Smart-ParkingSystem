#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);

#define IR_ENTRY 2
#define IR_EXIT 4
#define SERVO_PIN 3

const int slotSensors[6] = {5, 6, 7, 8, 9, 10};
int slotStatus[6] = {0, 0, 0, 0, 0, 0};
int availableSlots = 6;
bool entryFlag = false, exitFlag = false;

Servo barrierServo;

void displaySlots() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Available Slots: ");
    lcd.print(availableSlots);

    for (int i = 0; i < 6; i++) {
        lcd.setCursor((i % 2) * 10, 1 + (i / 2));
        lcd.print("Slot");
        lcd.print(i + 1);
        lcd.print(":");
        lcd.print(slotStatus[i] == 1 ? "Full " : "Empty");
    }
}

void updateSlotStatus() {
    for (int i = 0; i < 6; i++) {
        slotStatus[i] = (digitalRead(slotSensors[i]) == LOW) ? 1 : 0;
    }
    availableSlots = 6 - countOccupiedSlots();
}

int countOccupiedSlots() {
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if (slotStatus[i] == 1) count++;
    }
    return count;
}

void setup() {
    Serial.begin(9600);
    barrierServo.attach(SERVO_PIN);
    barrierServo.write(90);

    pinMode(IR_ENTRY, INPUT);
    pinMode(IR_EXIT, INPUT);

    for (int i = 0; i < 6; i++) {
        pinMode(slotSensors[i], INPUT);
    }

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SMART PARKING");
    lcd.setCursor(0, 1);
    lcd.print("SYSTEMS..!");
    delay(5000);
    lcd.clear();

    updateSlotStatus();
}

void loop() {
    displaySlots();

    if (availableSlots == 0 && digitalRead(IR_ENTRY) == LOW && !entryFlag && !exitFlag) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SORRY PARKING");
        lcd.setCursor(0, 1);
        lcd.print("IS FULL :)");
        barrierServo.write(90);
        return;
    }

    if (digitalRead(IR_ENTRY) == LOW && !entryFlag && !exitFlag) {
        entryFlag = true;
        barrierServo.write(180);
        lcd.setCursor(0, 0);
        lcd.print("Vehicle Entering...");
        delay(500);
    }

    if (entryFlag && digitalRead(IR_EXIT) == LOW) {
        barrierServo.write(90);
        lcd.setCursor(0, 0);
        lcd.print("Gate Closing...");
        delay(1500);
        entryFlag = false;
    }

    if (digitalRead(IR_EXIT) == LOW && !exitFlag && !entryFlag) {
        exitFlag = true;
        barrierServo.write(180);
        lcd.setCursor(0, 0);
        lcd.print("Vehicle Exiting...");
        delay(500);
    }

    if (exitFlag && digitalRead(IR_ENTRY) == LOW) {
        barrierServo.write(90);
        lcd.setCursor(0, 0);
        lcd.print("Gate Closing...");
        delay(1500);
        exitFlag = false;
        updateSlotStatus();
        delay(500);
    }
}
