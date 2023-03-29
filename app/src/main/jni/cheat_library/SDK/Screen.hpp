#pragma once


namespace Screen {
    enum TypeValue {
        COMPLEX_UNIT_PX = 0,
        COMPLEX_UNIT_DIP = 1,
        COMPLEX_UNIT_SP = 2,
        COMPLEX_UNIT_PT = 3,
        COMPLEX_UNIT_IN = 4,
        COMPLEX_UNIT_MM = 5
    };

    struct Metrics {
        float density;
        int densityDpi;
        float scaledDensity;
        float xdpi;
        float ydpi;
    };

    extern int width;
    extern int height;
    extern Metrics metrics;

    //OpenGL size
    extern int glWidth;
    extern int glHeight;

    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();

    float applyDimension(TypeValue unit, float value);
}
