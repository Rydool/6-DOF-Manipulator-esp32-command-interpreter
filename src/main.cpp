#include <Arduino.h>
#include "config/machine_config.h"
#include "drivers/stepgen.h"
#include "drivers/servo.h"
#include "motion/planner.h"
#include "motion/homing.h"
#include "motion/queue.h"

unsigned long lastUpdate = 0;
static String uartBuffer = "";

// --------------------------------------
// Soft-limity — sprawdzane dla MOVE/GOTO/JOG/PGOTO
// --------------------------------------
bool checkLimits(const MotionBlock& b) {
    for (int i = 0; i < AXIS_COUNT; i++) {
        float t = b.target_deg[i];
        if (t < machineConfig.limit_min_deg[i] || t > machineConfig.limit_max_deg[i]) {
            Serial.print("ERR_LIMIT_AXIS_");
            Serial.println(i + 1);
            return false;
        }
    }
    return true;
}

// --------------------------------------
// Parser komend
// --------------------------------------
void processCommand(String line) {
    line.trim();
    if (line.length() == 0) return;

    // =========================
    // HELP (główne + szczegółowe)
    // =========================
    if (line == "HELP") {
        Serial.println("=== HELP MENU ===");
        Serial.println("Dostepne kategorie:");
        Serial.println("  HELP MOVE");
        Serial.println("  HELP GOTO");
        Serial.println("  HELP JOG");
        Serial.println("  HELP HOME");
        Serial.println("  HELP LIMITS");
        Serial.println("  HELP POS");
        Serial.println("  HELP PSET");
        Serial.println("  HELP PGOTO");
        Serial.println("  HELP SYSTEM");
        Serial.println("  HELP ALL");
        return;
    }

    if (line == "HELP MOVE") {
        Serial.println("MOVE X Y Z A B C F");
        Serial.println("  Absolutny ruch wszystkich osi.");
        Serial.println("  Przyklad:");
        Serial.println("    MOVE 10 20 30 40 50 60 100");
        Serial.println("  Podlega soft-limitom.");
        return;
    }

    if (line == "HELP GOTO") {
        Serial.println("GOTO X.. Y.. Z.. A.. B.. C.. F..");
        Serial.println("  Ruch absolutny tylko wybranych osi.");
        Serial.println("  Przyklad:");
        Serial.println("    GOTO X10 Z50 F80");
        Serial.println("  Podlega soft-limitom.");
        return;
    }

    if (line == "HELP JOG") {
        Serial.println("JOG X+ [step]");
        Serial.println("JOG X- [step]");
        Serial.println("  Ruch wzgledny osi.");
        Serial.println("  Przyklady:");
        Serial.println("    JOG X+");
        Serial.println("    JOG Y- 5");
        Serial.println("  Podlega soft-limitom.");
        return;
    }

    if (line == "HELP HOME") {
        Serial.println("HOME <axis>");
        Serial.println("  Homing osi z rampa, ignoruje soft-limity.");
        Serial.println("");
        Serial.println("SETHOME");
        Serial.println("  Ustawia aktualny HOME na biezaca pozycje.");
        Serial.println("");
        Serial.println("SETHOMEDEF <haslo>");
        Serial.println("  Ustawia DOMYSLNY HOME (haslo: kosmos123).");
        Serial.println("");
        Serial.println("HOMEDEFAULT");
        Serial.println("  Przywraca aktualny HOME = domyslny.");
        Serial.println("");
        Serial.println("GOTOHOME [F]");
        Serial.println("  Jedzie do aktualnego HOME.");
        return;
    }

    if (line == "HELP LIMITS") {
        Serial.println("SETLIMIT <axis> <min> <max>");
        Serial.println("  Ustawia soft-limity osi.");
        Serial.println("  Przyklad:");
        Serial.println("    SETLIMIT 3 -25 195");
        Serial.println("");
        Serial.println("LIMITS");
        Serial.println("  Wyswietla limity i HOME.");
        return;
    }

    if (line == "HELP POS") {
        Serial.println("POS");
        Serial.println("  Wyswietla aktualne pozycje osi.");
        return;
    }

    if (line == "HELP PSET") {
        Serial.println("PSET <n>");
        Serial.println("  Zapisuje biezaca pozycje jako P<n>.");
        Serial.println("");
        Serial.println("PSET <n> X.. Y.. Z.. A.. B.. C..");
        Serial.println("  Ustawia P<n> na podane wartosci.");
        Serial.println("  Przyklad:");
        Serial.println("    PSET 1 X10 Y20 Z30");
        return;
    }

    if (line == "HELP PGOTO") {
        Serial.println("PGOTO <n> [F]");
        Serial.println("  Jedzie do zapisanej pozycji P<n>.");
        Serial.println("  Przyklad:");
        Serial.println("    PGOTO 1 F80");
        Serial.println("  Podlega soft-limitom.");
        return;
    }

    if (line == "HELP SYSTEM") {
        Serial.println("Komendy systemowe:");
        Serial.println("  SAVE");
        Serial.println("  LOAD");
        Serial.println("  DEFAULTS");
        Serial.println("  STATUS");
        Serial.println("  QUEUE");
        Serial.println("  AXISINFO");
        Serial.println("  STOP");
        return;
    }

    if (line == "HELP ALL") {
        Serial.println("=== PELNA LISTA KOMEND ===");
        Serial.println("MOVE, GOTO, JOG");
        Serial.println("HOME, SETHOME, SETHOMEDEF, HOMEDEFAULT, GOTOHOME");
        Serial.println("SETLIMIT, LIMITS");
        Serial.println("PSET, PGOTO, PLIST");
        Serial.println("POS, STATUS, QUEUE, AXISINFO");
        Serial.println("SAVE, LOAD, DEFAULTS, STOP");
        Serial.println("SERVO <angle>");
        return;
    }

    // =========================
    // RESZTA KOMEND
    // =========================

    // POS
    if (line == "POS") {
        for (int i = 0; i < AXIS_COUNT; i++) {
            Serial.print("Axis ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(stepgen_getPositionDeg(i), 3);
        }
        return;
    }

    // STATUS
    if (line == "STATUS") {
        Serial.print("Planner: ");
        Serial.println(motionPlanner.isBusy() ? "BUSY" : "IDLE");

        Serial.print("Homing: ");
        Serial.println(homing_isActive() ? "ACTIVE" : "INACTIVE");
        return;
    }

    // QUEUE
    if (line == "QUEUE") {
        Serial.print("Queue size: ");
        Serial.println(motionQueue.size());
        return;
    }

    // LIMITS
    if (line == "LIMITS") {
        for (int i = 0; i < AXIS_COUNT; i++) {
            Serial.print("Axis ");
            Serial.print(i + 1);
            Serial.print(": min=");
            Serial.print(machineConfig.limit_min_deg[i]);
            Serial.print(" max=");
            Serial.print(machineConfig.limit_max_deg[i]);
            Serial.print(" home=");
            Serial.println(machineConfig.home_offset_deg[i]);
        }
        return;
    }

    // SETLIMIT <axis> <min> <max>
    if (line.startsWith("SETLIMIT")) {
        int idx1 = line.indexOf(' ');
        int idx2 = line.indexOf(' ', idx1 + 1);
        int idx3 = line.indexOf(' ', idx2 + 1);
        if (idx1 < 0 || idx2 < 0 || idx3 < 0) {
            Serial.println("ERR");
            return;
        }
        int axis = line.substring(idx1 + 1, idx2).toInt() - 1;
        float minDeg = line.substring(idx2 + 1, idx3).toFloat();
        float maxDeg = line.substring(idx3 + 1).toFloat();

        machineConfig.setLimits(axis, minDeg, maxDeg);
        machineConfig.save();
        Serial.println("OK");
        return;
    }

    // SETHOME
    if (line == "SETHOME") {
        float cur[AXIS_COUNT];
        for (int i = 0; i < AXIS_COUNT; i++)
            cur[i] = stepgen_getPositionDeg(i);

        machineConfig.setHomeToCurrent(cur);
        machineConfig.save();
        Serial.println("OK");
        return;
    }

    // SETHOMEDEF <haslo>
    if (line.startsWith("SETHOMEDEF")) {
        int idx = line.indexOf(' ');
        if (idx < 0) {
            Serial.println("ERR_PASS");
            return;
        }
        String pass = line.substring(idx + 1);
        pass.trim();
        if (pass != "kosmos123") {
            Serial.println("ERR_PASS");
            return;
        }

        float cur[AXIS_COUNT];
        for (int i = 0; i < AXIS_COUNT; i++)
            cur[i] = stepgen_getPositionDeg(i);

        machineConfig.setFactoryHomeToCurrent(cur);
        machineConfig.save();
        Serial.println("OK");
        return;
    }

    // HOMEDEFAULT
    if (line == "HOMEDEFAULT") {
        machineConfig.restoreFactoryHome();
        machineConfig.save();
        Serial.println("OK");
        return;
    }

    // GOTOHOME [F]
    if (line.startsWith("GOTOHOME")) {
        float feed = 50.0f;
        int idx = line.indexOf(' ');
        if (idx > 0) {
            feed = line.substring(idx + 1).toFloat();
            if (feed <= 0) feed = 50.0f;
        }

        MotionBlock b;
        b.homing = false;
        for (int i = 0; i < AXIS_COUNT; i++)
            b.target_deg[i] = machineConfig.home_offset_deg[i];
        b.feed_deg_s = feed;

        if (!checkLimits(b)) return;

        motionQueue.push(b);
        Serial.println("OK");
        return;
    }

    // PLIST
    if (line == "PLIST") {
        for (int s = 0; s < POS_SLOT_COUNT; s++) {
            Serial.print("P");
            Serial.print(s + 1);
            if (!machineConfig.pos_valid[s]) {
                Serial.println(": EMPTY");
                continue;
            }
            Serial.print(": ");
            for (int i = 0; i < AXIS_COUNT; i++) {
                Serial.print(machineConfig.pos_deg[s][i], 2);
                if (i < AXIS_COUNT - 1) Serial.print(", ");
            }
            Serial.println();
        }
        return;
    }

    // PSET <n> ...
    if (line.startsWith("PSET")) {
        int idx1 = line.indexOf(' ');
        if (idx1 < 0) {
            Serial.println("ERR");
            return;
        }
        int idx2 = line.indexOf(' ', idx1 + 1);
        int slotNum;
        if (idx2 < 0) {
            slotNum = line.substring(idx1 + 1).toInt();
        } else {
            slotNum = line.substring(idx1 + 1, idx2).toInt();
        }
        int slot = slotNum - 1;
        if (slot < 0 || slot >= POS_SLOT_COUNT) {
            Serial.println("ERR");
            return;
        }

        float pos[AXIS_COUNT];

        if (idx2 < 0) {
            for (int i = 0; i < AXIS_COUNT; i++)
                pos[i] = stepgen_getPositionDeg(i);
        } else {
            for (int i = 0; i < AXIS_COUNT; i++)
                pos[i] = 0.0f;

            String rest = line.substring(idx2 + 1) + " ";
            for (int i = 0; i < rest.length(); i++) {
                char c = rest[i];
                if (c == 'X' || c == 'Y' || c == 'Z' ||
                    c == 'A' || c == 'B' || c == 'C') {

                    int axis = (c == 'X') ? 0 :
                               (c == 'Y') ? 1 :
                               (c == 'Z') ? 2 :
                               (c == 'A') ? 3 :
                               (c == 'B') ? 4 : 5;

                    int start = i + 1;
                    int end = rest.indexOf(' ', start);
                    pos[axis] = rest.substring(start, end).toFloat();
                }
            }
        }

        machineConfig.setPosSlot(slot, pos);
        machineConfig.save();
        Serial.println("OK");
        return;
    }

    // PGOTO <n> [F]
    if (line.startsWith("PGOTO")) {
        int idx1 = line.indexOf(' ');
        if (idx1 < 0) {
            Serial.println("ERR");
            return;
        }
        int idx2 = line.indexOf(' ', idx1 + 1);

        int slotNum;
        float feed = 50.0f;

        if (idx2 < 0) {
            slotNum = line.substring(idx1 + 1).toInt();
        } else {
            slotNum = line.substring(idx1 + 1, idx2).toInt();
            feed = line.substring(idx2 + 1).toFloat();
            if (feed <= 0) feed = 50.0f;
        }

        int slot = slotNum - 1;
        float pos[AXIS_COUNT];
        if (!machineConfig.getPosSlot(slot, pos)) {
            Serial.println("ERR");
            return;
        }

        MotionBlock b;
        b.homing = false;
        for (int i = 0; i < AXIS_COUNT; i++)
            b.target_deg[i] = pos[i];
        b.feed_deg_s = feed;

        if (!checkLimits(b)) return;

        motionQueue.push(b);
        Serial.println("OK");
        return;
    }

    // AXISINFO
    if (line == "AXISINFO") {
        for (int i = 0; i < AXIS_COUNT; i++) {
            Serial.print("Axis ");
            Serial.print(i + 1);
            Serial.print(": steps/deg=");
            Serial.print(machineConfig.steps_per_deg[i]);
            Serial.print(", max_accel=");
            Serial.println(machineConfig.max_accel_deg_s2[i]);
        }
        return;
    }

    // SERVO <angle>
    if (line.startsWith("SERVO")) {
        float angle = line.substring(5).toFloat();
        servoDriver.setAngle(angle);
        Serial.println("OK");
        return;
    }

    // GOTO X.. Y.. Z.. A.. B.. C.. F..
    if (line.startsWith("GOTO")) {
        float target[AXIS_COUNT];
        for (int i = 0; i < AXIS_COUNT; i++)
            target[i] = stepgen_getPositionDeg(i);

        float feed = 50;
        line += " ";

        for (int i = 0; i < line.length(); i++) {
            char c = line[i];

            if (c == 'X' || c == 'Y' || c == 'Z' ||
                c == 'A' || c == 'B' || c == 'C') {

                int axis = (c == 'X') ? 0 :
                           (c == 'Y') ? 1 :
                           (c == 'Z') ? 2 :
                           (c == 'A') ? 3 :
                           (c == 'B') ? 4 : 5;

                int start = i + 1;
                int end = line.indexOf(' ', start);
                target[axis] = line.substring(start, end).toFloat();
            }

            if (c == 'F') {
                int start = i + 1;
                int end = line.indexOf(' ', start);
                feed = line.substring(start, end).toFloat();
            }
        }

        MotionBlock b;
        b.homing = false;
        for (int i = 0; i < AXIS_COUNT; i++)
            b.target_deg[i] = target[i];
        b.feed_deg_s = feed;

        if (!checkLimits(b)) return;

        motionQueue.push(b);
        Serial.println("OK");
        return;
    }

    // JOG X+ [step]
    if (line.startsWith("JOG")) {
        float step = 1.0f;

        int lastSpace = line.lastIndexOf(' ');
        if (lastSpace > 0) {
            String maybeNum = line.substring(lastSpace + 1);
            if (maybeNum.length() > 0 && isDigit(maybeNum[0])) {
                step = maybeNum.toFloat();
                line = line.substring(0, lastSpace);
            }
        }

        int axis = -1;
        int dir = 0;

        if (line.indexOf("X+") > 0) { axis = 0; dir = +1; }
        if (line.indexOf("X-") > 0) { axis = 0; dir = -1; }
        if (line.indexOf("Y+") > 0) { axis = 1; dir = +1; }
        if (line.indexOf("Y-") > 0) { axis = 1; dir = -1; }
        if (line.indexOf("Z+") > 0) { axis = 2; dir = +1; }
        if (line.indexOf("Z-") > 0) { axis = 2; dir = -1; }
        if (line.indexOf("A+") > 0) { axis = 3; dir = +1; }
        if (line.indexOf("A-") > 0) { axis = 3; dir = -1; }
        if (line.indexOf("B+") > 0) { axis = 4; dir = +1; }
        if (line.indexOf("B-") > 0) { axis = 4; dir = -1; }
        if (line.indexOf("C+") > 0) { axis = 5; dir = +1; }
        if (line.indexOf("C-") > 0) { axis = 5; dir = -1; }

        if (axis < 0) {
            Serial.println("ERR");
            return;
        }

        float target[AXIS_COUNT];
        for (int i = 0; i < AXIS_COUNT; i++)
            target[i] = stepgen_getPositionDeg(i);

        target[axis] += dir * step;

        MotionBlock b;
        b.homing = false;
        for (int i = 0; i < AXIS_COUNT; i++)
            b.target_deg[i] = target[i];
        b.feed_deg_s = 50;

        if (!checkLimits(b)) return;

        motionQueue.push(b);
        Serial.println("OK");
        return;
    }

    // MOVE X Y Z A B C F
    if (line.startsWith("MOVE")) {
        MotionBlock b;
        b.homing = false;

        int idx = 5;
        for (int i = 0; i < AXIS_COUNT; i++) {
            int next = line.indexOf(' ', idx);
            if (next == -1) next = line.length();
            b.target_deg[i] = line.substring(idx, next).toFloat();
            idx = next + 1;
        }

        b.feed_deg_s = line.substring(idx).toFloat();
        if (b.feed_deg_s <= 0) b.feed_deg_s = 50;

        if (!checkLimits(b)) return;

        motionQueue.push(b);
        Serial.println("OK");
        return;
    }

    // HOME <axis> — homing z rampa, ignoruje soft-limity
    if (line.startsWith("HOME")) {
        int axis = line.substring(4).toInt();

        MotionBlock b;
        b.homing = true;
        b.target_deg[0] = axis;

        motionQueue.push(b);
        Serial.println("OK");
        return;
    }

    if (line == "SAVE") {
        machineConfig.save();
        Serial.println("OK");
        return;
    }

    if (line == "LOAD") {
        machineConfig.load();
        Serial.println("OK");
        return;
    }

    if (line == "DEFAULTS") {
        machineConfig.restoreDefaults();
        Serial.println("OK");
        return;
    }

    if (line == "STOP") {
        motionPlanner.stopAll();
        Serial.println("OK");
        return;
    }

    Serial.println("UNKNOWN_CMD");
}

// --------------------------------------
// SETUP
// --------------------------------------
void setup() {
    Serial.begin(115200);
    delay(300);

    Serial.println("Robot Controller Booting...");

    machineConfig.load();
    motionPlanner.begin();
    servoDriver.begin();
    stepgen.begin();

    lastUpdate = micros();

    Serial.println("Ready.");
}

// --------------------------------------
// LOOP
// --------------------------------------
void loop() {
    unsigned long now = micros();
    if (now - lastUpdate >= 2000) {
        float dt = (now - lastUpdate) / 1e6f;
        lastUpdate = now;

        if (homing_isActive()) {
            homing_update(dt);
        } else {
            motionPlanner.update(dt);
        }
    }

    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\r') continue;

        if (c == '\n') {
            processCommand(uartBuffer);
            uartBuffer = "";
            return;
        }

        if (c >= 32 && c <= 126) {
            uartBuffer += c;
        }
    }
}
