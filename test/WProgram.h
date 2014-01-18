#ifndef WPROGRAM_FAKE
#define WPROGRAM_FAKE
inline void delay(int) {}

static class {
public:
    inline static void send(int) {}
    inline static void beginTransmission(int) {}
    inline static int receive() {return 0;}
    inline static void endTransmission() {}
    inline static void begin() {}
    inline static int requestFrom(uint8_t&, uint8_t) { return 0; }
} Wire;

#endif
