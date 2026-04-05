#include "machine_config.h"

MachineConfig machineConfig;

// domyślne wartości
static const float DEFAULT_STEPS_PER_DEG[AXIS_COUNT] = {
    10, 10, 10, 10, 10, 10
};

static const float DEFAULT_MAX_SPEED[AXIS_COUNT] = {
    90, 90, 90, 60, 60, 60
};

static const float DEFAULT_MAX_ACCEL[AXIS_COUNT] = {
    200, 200, 200, 150, 150, 150
};

// domyślne soft-limity (Twoje)
static const float DEFAULT_LIMIT_MIN[AXIS_COUNT] = {
    -180, -105, -25, -270, -110, -270
};

static const float DEFAULT_LIMIT_MAX[AXIS_COUNT] = {
     180,  100, 195,  270,  110,  270
};

// domyślny HOME = 0
static const float DEFAULT_FACTORY_HOME[AXIS_COUNT] = {
    0, 0, 0, 0, 0, 0
};

MachineConfig::MachineConfig() {}

void MachineConfig::loadArray(const char* key, float* arr, const float* defaults) {
    size_t len = prefs.getBytesLength(key);
    if (len == AXIS_COUNT * sizeof(float)) {
        prefs.getBytes(key, arr, len);
    } else {
        memcpy(arr, defaults, AXIS_COUNT * sizeof(float));
    }
}

void MachineConfig::saveArray(const char* key, const float* arr) {
    prefs.putBytes(key, arr, AXIS_COUNT * sizeof(float));
}

void MachineConfig::loadArray2D(const char* key, float arr[][AXIS_COUNT]) {
    size_t expected = POS_SLOT_COUNT * AXIS_COUNT * sizeof(float);
    size_t len = prefs.getBytesLength(key);
    if (len == expected) {
        prefs.getBytes(key, arr, len);
    } else {
        for (int s = 0; s < POS_SLOT_COUNT; s++)
            for (int i = 0; i < AXIS_COUNT; i++)
                arr[s][i] = 0.0f;
    }
}

void MachineConfig::saveArray2D(const char* key, const float arr[][AXIS_COUNT]) {
    prefs.putBytes(key, arr, POS_SLOT_COUNT * AXIS_COUNT * sizeof(float));
}

void MachineConfig::loadBoolArray(const char* key, bool* arr) {
    size_t expected = POS_SLOT_COUNT * sizeof(uint8_t);
    size_t len = prefs.getBytesLength(key);
    if (len == expected) {
        uint8_t tmp[POS_SLOT_COUNT];
        prefs.getBytes(key, tmp, len);
        for (int i = 0; i < POS_SLOT_COUNT; i++)
            arr[i] = (tmp[i] != 0);
    } else {
        for (int i = 0; i < POS_SLOT_COUNT; i++)
            arr[i] = false;
    }
}

void MachineConfig::saveBoolArray(const char* key, const bool* arr) {
    uint8_t tmp[POS_SLOT_COUNT];
    for (int i = 0; i < POS_SLOT_COUNT; i++)
        tmp[i] = arr[i] ? 1 : 0;
    prefs.putBytes(key, tmp, POS_SLOT_COUNT * sizeof(uint8_t));
}

void MachineConfig::load() {
    prefs.begin("machine", true);

    // stare pola
    loadArray("spd",    steps_per_deg,    DEFAULT_STEPS_PER_DEG);
    loadArray("maxspd", max_speed_deg_s,  DEFAULT_MAX_SPEED);
    loadArray("maxacc", max_accel_deg_s2, DEFAULT_MAX_ACCEL);

    // nowe: limity
    loadArray("limmin", limit_min_deg, DEFAULT_LIMIT_MIN);
    loadArray("limmax", limit_max_deg, DEFAULT_LIMIT_MAX);

    // nowe: domyślny HOME
    loadArray("fhome", factory_home_deg, DEFAULT_FACTORY_HOME);

    // nowe: aktualny HOME (jeśli brak, = domyślny)
    size_t lenHome = prefs.getBytesLength("homeoff");
    if (lenHome == AXIS_COUNT * sizeof(float)) {
        prefs.getBytes("homeoff", home_offset_deg, lenHome);
    } else {
        memcpy(home_offset_deg, factory_home_deg, AXIS_COUNT * sizeof(float));
    }

    // nowe: pozycje P1–P10
    loadArray2D("posdeg", pos_deg);
    loadBoolArray("posval", pos_valid);

    prefs.end();
}

void MachineConfig::save() {
    prefs.begin("machine", false);

    // stare pola
    saveArray("spd",    steps_per_deg);
    saveArray("maxspd", max_speed_deg_s);
    saveArray("maxacc", max_accel_deg_s2);

    // nowe: limity
    saveArray("limmin", limit_min_deg);
    saveArray("limmax", limit_max_deg);

    // nowe: HOME
    saveArray("fhome",  factory_home_deg);
    prefs.putBytes("homeoff", home_offset_deg, AXIS_COUNT * sizeof(float));

    // nowe: pozycje P1–P10
    saveArray2D("posdeg", pos_deg);
    saveBoolArray("posval", pos_valid);

    prefs.end();
}

void MachineConfig::restoreDefaults() {
    memcpy(steps_per_deg,    DEFAULT_STEPS_PER_DEG, sizeof(steps_per_deg));
    memcpy(max_speed_deg_s,  DEFAULT_MAX_SPEED,     sizeof(max_speed_deg_s));
    memcpy(max_accel_deg_s2, DEFAULT_MAX_ACCEL,     sizeof(max_accel_deg_s2));

    memcpy(limit_min_deg, DEFAULT_LIMIT_MIN, sizeof(limit_min_deg));
    memcpy(limit_max_deg, DEFAULT_LIMIT_MAX, sizeof(limit_max_deg));

    memcpy(factory_home_deg, DEFAULT_FACTORY_HOME, sizeof(factory_home_deg));
    memcpy(home_offset_deg,  DEFAULT_FACTORY_HOME, sizeof(home_offset_deg));

    for (int s = 0; s < POS_SLOT_COUNT; s++) {
        pos_valid[s] = false;
        for (int i = 0; i < AXIS_COUNT; i++)
            pos_deg[s][i] = 0.0f;
    }

    save();
}

void MachineConfig::setStepsPerDeg(int axis, float value) {
    if (axis < 0 || axis >= AXIS_COUNT) return;
    steps_per_deg[axis] = value;
}

void MachineConfig::setMaxSpeed(int axis, float value) {
    if (axis < 0 || axis >= AXIS_COUNT) return;
    max_speed_deg_s[axis] = value;
}

void MachineConfig::setMaxAccel(int axis, float value) {
    if (axis < 0 || axis >= AXIS_COUNT) return;
    max_accel_deg_s2[axis] = value;
}

void MachineConfig::setLimits(int axis, float minDeg, float maxDeg) {
    if (axis < 0 || axis >= AXIS_COUNT) return;
    limit_min_deg[axis] = minDeg;
    limit_max_deg[axis] = maxDeg;
}

void MachineConfig::setHomeToCurrent(const float currentPos[AXIS_COUNT]) {
    for (int i = 0; i < AXIS_COUNT; i++) {
        home_offset_deg[i] = currentPos[i];
    }
}

void MachineConfig::setFactoryHomeToCurrent(const float currentPos[AXIS_COUNT]) {
    for (int i = 0; i < AXIS_COUNT; i++) {
        factory_home_deg[i] = currentPos[i];
        home_offset_deg[i]  = currentPos[i];
    }
}

void MachineConfig::restoreFactoryHome() {
    for (int i = 0; i < AXIS_COUNT; i++) {
        home_offset_deg[i] = factory_home_deg[i];
    }
}

void MachineConfig::setPosSlot(int slot, const float pos[AXIS_COUNT]) {
    if (slot < 0 || slot >= POS_SLOT_COUNT) return;
    for (int i = 0; i < AXIS_COUNT; i++) {
        pos_deg[slot][i] = pos[i];
    }
    pos_valid[slot] = true;
}

bool MachineConfig::getPosSlot(int slot, float outPos[AXIS_COUNT]) const {
    if (slot < 0 || slot >= POS_SLOT_COUNT) return false;
    if (!pos_valid[slot]) return false;
    for (int i = 0; i < AXIS_COUNT; i++) {
        outPos[i] = pos_deg[slot][i];
    }
    return true;
}
