#include "CDHDeployment/LedBlinker/LedBlinker.hpp"
#include <fstream>
#include <string>

namespace CDHDeployment {

static constexpr int LED_GPIO = 27;  // txLed on CubeSatSim

static void gpioWrite(int pin, int value) {
    { std::ofstream f("/sys/class/gpio/export");
      if (f) f << pin; }
    { std::ofstream f("/sys/class/gpio/gpio" + std::to_string(pin) + "/direction");
      if (f) f << "out"; }
    { std::ofstream f("/sys/class/gpio/gpio" + std::to_string(pin) + "/value");
      if (f) f << value; }
}

LedBlinker::LedBlinker(const char* compName)
    : LedBlinkerComponentBase(compName) {
    gpioWrite(LED_GPIO, 0);
}

LedBlinker::~LedBlinker() {
    gpioWrite(LED_GPIO, 0);
}

void LedBlinker::seqCmdIn_handler(FwIndexType portNum, Fw::ComBuffer& data, U32 context) {
    gpioWrite(LED_GPIO, 1);
    m_blinkCount = BLINK_TICKS;
    seqCmdOut_out(0, data, context);
}

void LedBlinker::seqCmdStatusIn_handler(FwIndexType portNum, FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdResponse& response) {
    seqCmdStatusOut_out(0, opCode, cmdSeq, response);
}

void LedBlinker::schedIn_handler(FwIndexType portNum, U32 context) {
    if (m_blinkCount > 0) {
        if (--m_blinkCount == 0) {
            gpioWrite(LED_GPIO, 0);
        }
    }
}

} // namespace CDHDeployment
