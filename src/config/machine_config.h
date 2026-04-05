#pragma once
#include <Arduino.h>
#include <Preferences.h>

#define AXIS_COUNT 6
#define POS_SLOT_COUNT 10   // P1–P10

// Indeksy osi
enum AxisIndex {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_Z = 2,
    AXIS_A = 3,
    AXIS_B = 4,
    AXIS_C = 5
};

class MachineConfig {
public:
    MachineConfig();

    // aktualne wartości
    float steps_per_deg[AXIS_COUNT];
    float max_speed_deg_s[AXIS_COUNT];
    float max_accel_deg_s2[AXIS_COUNT];

    // soft-limity (względem domyślnego HOME = 0)
    float limit_min_deg[AXIS_COUNT];
    float limit_max_deg[AXIS_COUNT];

    // domyślny globalny HOME (fabryczny)
    float factory_home_deg[AXIS_COUNT];

    // aktualny HOME (używany przez GOTOHOME)
    float home_offset_deg[AXIS_COUNT];

    // zapisane pozycje P1–P10
    float pos_deg[POS_SLOT_COUNT][AXIS_COUNT];
    bool  pos_valid[POS_SLOT_COUNT];

    // ładowanie / zapis
    void load();
    void save();
    void restoreDefaults();

    // zmiana pojedynczej osi
    void setStepsPerDeg(int axis, float value);
    void setMaxSpeed(int axis, float value);
    void setMaxAccel(int axis, float value);

    // soft-limity
    void setLimits(int axis, float minDeg, float maxDeg);

    // aktualny HOME = bieżąca pozycja
    void setHomeToCurrent(const float currentPos[AXIS_COUNT]);

    // domyślny HOME = bieżąca pozycja
    void setFactoryHomeToCurrent(const float currentPos[AXIS_COUNT]);

    // aktualny HOME = domyślny
    void restoreFactoryHome();

    // pozycje P1–P10
    void setPosSlot(int slot, const float pos[AXIS_COUNT]);
    bool getPosSlot(int slot, float outPos[AXIS_COUNT]) const;

private:
    Preferences prefs;

    void loadArray(const char* key, float* arr, const float* defaults);
    void saveArray(const char* key, const float* arr);

    void loadArray2D(const char* key, float arr[][AXIS_COUNT]);
    void saveArray2D(const char* key, const float arr[][AXIS_COUNT]);

    void loadBoolArray(const char* key, bool* arr);
    void saveBoolArray(const char* key, const bool* arr);
};

// instancja globalna
extern MachineConfig machineConfig;
