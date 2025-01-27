#include <Bluepad32.h>

// Configuration
#define DEBUG_MODE 0  // Set to 1 to enable debug logging (temporarily enabled for troubleshooting)

// Pin Definitions
namespace Pins {
constexpr uint8_t R1_SER = 21;   // Serial input for the first of two shift-registers of Player 1 input
constexpr uint8_t R2_SER = 20;   // Serial input for the first of two shift-registers of Player 2 input
constexpr uint8_t R1_RCLK = 10;  // All shift registers share the same RCLK (Storage register clock)
constexpr uint8_t R1_SRCLK = 9;  // All shift registers share the same SRCLK (Shift register clock)
constexpr uint8_t LED_P1 = 0;    // LED for Player 1 connection status
constexpr uint8_t LED_P2 = 1;    // LED for Player 2 connection status
}

// Constants
namespace Constants {
constexpr int MAX_GAMEPADS = 2;          // Maximum number of supported gamepads
constexpr int STICK_DEAD_ZONE = 64;     // Analog stick dead zone value (0-512)
}

// Debug logging macro for conditional debug output
#define DEBUG_LOG(...) \
  do { \
    if (DEBUG_MODE) Serial.printf(__VA_ARGS__); \
  } while (0)

/**
 * @brief Manages a pair of 8-bit shift registers for gamepad output
 * 
 * This class handles the low-level communication with the shift registers,
 * converting gamepad states into serial data for output.
 */
class ShiftRegister {
public:
  ShiftRegister(uint8_t ser, uint8_t rclk, uint8_t srclk)
    : serPin_(ser), rclkPin_(rclk), srclkPin_(srclk), currentValue_{ 0, 0 } {
    initialize();
  }

  // Updates the internal state of the shift registers
  void update(uint8_t highByte, uint8_t lowByte) {
    currentValue_[0] = highByte;
    currentValue_[1] = lowByte;
  }

  // Sets the serial input bit for the shift register
  void setBit(bool value) {
    digitalWrite(serPin_, value);
  }

  // Getters for current register values
  uint8_t getHighByte() const { return currentValue_[0]; }
  uint8_t getLowByte() const { return currentValue_[1]; }

private:
  void initialize() {
    pinMode(serPin_, OUTPUT);
    pinMode(rclkPin_, OUTPUT);
    pinMode(srclkPin_, OUTPUT);
    digitalWrite(serPin_, LOW);
  }

  const uint8_t serPin_;    // Serial data input pin
  const uint8_t rclkPin_;   // Storage register clock pin
  const uint8_t srclkPin_;  // Shift register clock pin
  uint8_t currentValue_[2]; // Current register values: [0] = high byte, [1] = low byte
};

/**
 * @brief Main class handling gamepad connections and input processing
 * 
 * Manages Bluetooth gamepad connections, processes input data, and
 * outputs the processed states to shift registers.
 */
class GamepadManager {
public:
  GamepadManager()
    : reg1_(Pins::R1_SER, Pins::R1_RCLK, Pins::R1_SRCLK),
      reg2_(Pins::R2_SER, Pins::R1_RCLK, Pins::R1_SRCLK),
      connectedGamepads_(0) {
    initialize();
  }

  void setup() {
    setupBluePad32();
    initializeOutputs();
    setupLEDs();
    DEBUG_LOG("GamepadManager setup complete\n");
  }

  // Main update function - processes controller input
  void update() {
    if (BP32.update()) {
      processControllers();
    }
    updateLEDs();
  }

  // Static callback handlers for controller events
  static void onConnectedController(ControllerPtr ctl) {
    instance().handleControllerConnection(ctl);
  }

  static void onDisconnectedController(ControllerPtr ctl) {
    instance().handleControllerDisconnection(ctl);
  }

  // Singleton instance accessor
  static GamepadManager& instance() {
    static GamepadManager instance;
    return instance;
  }

private:
  ShiftRegister reg1_, reg2_;
  ControllerPtr controllers_[Constants::MAX_GAMEPADS];
  uint16_t lastStates_[Constants::MAX_GAMEPADS];
  int connectedGamepads_;       // Number of currently connected gamepads

  void initialize() {
    for (auto& controller : controllers_) {
      controller = nullptr;
    }
    for (auto& state : lastStates_) {
      state = 0xFFFF;  // All bits set to 1 (inactive)
    }
    DEBUG_LOG("GamepadManager initialized\n");
  }

  void setupBluePad32() {
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false);  // Disable virtual gamepad device
    DEBUG_LOG("Bluepad32 setup complete\n");
  }

  void setupLEDs() {
    pinMode(Pins::LED_P1, OUTPUT);
    pinMode(Pins::LED_P2, OUTPUT);
    digitalWrite(Pins::LED_P1, LOW);
    digitalWrite(Pins::LED_P2, LOW);
  }

  void updateLEDs() {
    digitalWrite(Pins::LED_P1, controllers_[0] != nullptr ? HIGH : LOW);
    digitalWrite(Pins::LED_P2, controllers_[1] != nullptr ? HIGH : LOW);
  }

  // Clears output for a specific player
  void clearPlayerOutput(int ctlIndex) {
    lastStates_[ctlIndex] = 0xFFFF;
    updateRegisters();
    DEBUG_LOG("Cleared output for player %d\n", ctlIndex + 1);
  }

  void initializeOutputs() {
    for (int i = 0; i < Constants::MAX_GAMEPADS; i++) {
      clearPlayerOutput(i);
    }
  }

  // Updates shift registers with current gamepad states
  void updateRegisters() {
    // First shift out the high byte (bits 15-8)
    for (int i = 7; i >= 0; i--) {
      reg1_.setBit(!!(lastStates_[0] & (1 << (i + 8))));
      reg2_.setBit(!!(lastStates_[1] & (1 << (i + 8))));
      
      digitalWrite(Pins::R1_SRCLK, HIGH);
      digitalWrite(Pins::R1_SRCLK, LOW);
    }

    // Then shift out the low byte (bits 7-0)
    for (int i = 7; i >= 0; i--) {
      reg1_.setBit(!!(lastStates_[0] & (1 << i)));
      reg2_.setBit(!!(lastStates_[1] & (1 << i)));
      
      digitalWrite(Pins::R1_SRCLK, HIGH);
      digitalWrite(Pins::R1_SRCLK, LOW);
    }

    // Latch the outputs simultaneously
    digitalWrite(Pins::R1_RCLK, HIGH);
    digitalWrite(Pins::R1_RCLK, LOW);
  }

  // Handles new controller connections
  void handleControllerConnection(ControllerPtr ctl) {
    for (int i = 0; i < Constants::MAX_GAMEPADS; i++) {
      if (controllers_[i] == nullptr) {
        DEBUG_LOG("Controller connected at index=%d\n", i);
        controllers_[i] = ctl;
        configureNewController(ctl, i);
        connectedGamepads_++;
        DEBUG_LOG("Total connected gamepads: %d\n", connectedGamepads_);
        return;
      }
    }
    DEBUG_LOG("No empty slots for new controller\n");
  }

  // Configures newly connected controllers
  void configureNewController(ControllerPtr ctl, int index) {
    auto properties = ctl->getProperties();
    DEBUG_LOG("Controller model: %s, VID=0x%04x, PID=0x%04x\n",
              ctl->getModelName().c_str(),
              properties.vendor_id,
              properties.product_id);

    // Play rumble effect to indicate successful connection
    ctl->playDualRumble(0, 250, 0x80, 0x40);
  }

  // Handles controller disconnections
  void handleControllerDisconnection(ControllerPtr ctl) {
    for (int i = 0; i < Constants::MAX_GAMEPADS; i++) {
      if (controllers_[i] == ctl) {
        DEBUG_LOG("Controller disconnected from index=%d\n", i);
        clearPlayerOutput(i);
        controllers_[i] = nullptr;
        connectedGamepads_--;
        DEBUG_LOG("Total connected gamepads: %d\n", connectedGamepads_);
        return;
      }
    }
  }

  // Main controller input processing loop
  void processControllers() {
    bool needsUpdate = false;
    for (int i = 0; i < Constants::MAX_GAMEPADS; ++i) {
      ControllerPtr controller = controllers_[i];
      if (controller && controller->isConnected()) {
        if (controller->hasData()) {
          if (controller->isGamepad()) {
            processGamepadInput(controller, i);
            needsUpdate = true;
          } else {
            DEBUG_LOG("Unsupported controller type\n");
          }
        }
      }
    }
    if (needsUpdate) {
      updateRegisters();
    }
  }

  // Processes input from a single gamepad
  void processGamepadInput(ControllerPtr ctl, int ctlIndex) {
    // Output format:
    // [15-12: unused (0000)]
    // [11: select button]
    // [10: start button]
    // [9-6: up/down/left/right]
    // [5-0: action buttons]

    // Process button states (bits 5-0)
    uint16_t buttonStates = mapButtons(ctl->buttons() & 0x3F);
    
    // Process D-pad and left analog stick (bits 9-6)
    uint8_t dpadState = ctl->dpad();
    uint8_t mappedDpad = mapDpadState(dpadState);
    uint8_t mappedLeftStick = mapLeftStickState(ctl->axisX(), ctl->axisY());
    uint16_t directionState = ((mappedDpad | mappedLeftStick) & 0x0F) << 6;

    // Process Start/Select buttons (bits 11-10)
    uint16_t startButton = (ctl->miscStart() ? 1 : 0) << 10;
    uint16_t selectButton = (ctl->miscSelect() ? 1 : 0) << 11;

    // Combine all states
    uint16_t combinedState = buttonStates | directionState | startButton | selectButton;
    
    // Invert for active-low output
    lastStates_[ctlIndex] = ~combinedState;

#if DEBUG_MODE
    logGamepadState(ctl);
#endif
  }

  // Maps analog stick values to digital directions
  static uint8_t mapLeftStickState(int32_t x, int32_t y) {
    uint8_t mapped = 0;
    bitWrite(mapped, 0, x >= Constants::STICK_DEAD_ZONE ? 1 : 0);   // Right
    bitWrite(mapped, 1, x <= -Constants::STICK_DEAD_ZONE ? 1 : 0);  // Left
    bitWrite(mapped, 2, y >= Constants::STICK_DEAD_ZONE ? 1 : 0);  // Up
    bitWrite(mapped, 3, y <= -Constants::STICK_DEAD_ZONE ? 1 : 0);   // Down
    return mapped;
  }

  // Maps D-pad states to consistent bit format
  static uint8_t mapDpadState(uint8_t dpadState) {
    uint8_t mapped = 0;
    bitWrite(mapped, 0, bitRead(dpadState, 2));  // Right
    bitWrite(mapped, 1, bitRead(dpadState, 3));  // Left
    bitWrite(mapped, 2, bitRead(dpadState, 1));  // Up
    bitWrite(mapped, 3, bitRead(dpadState, 0));  // Down
    return mapped;
  }

  static uint16_t mapButtons(uint16_t buttons) {
    uint16_t mapped = 0;
    bitWrite(mapped, 0, bitRead(buttons, 5));
    bitWrite(mapped, 1, bitRead(buttons, 4));
    bitWrite(mapped, 2, bitRead(buttons, 3));
    bitWrite(mapped, 3, bitRead(buttons, 2));
    bitWrite(mapped, 4, bitRead(buttons, 1));
    bitWrite(mapped, 5, bitRead(buttons, 0));
    return mapped;
  }
  // Logs detailed gamepad state for debugging
  static void logGamepadState(ControllerPtr ctl) {
    DEBUG_LOG(
      "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, "
      "brake: %4d, throttle: %4d, misc: 0x%02x\n",
      ctl->index(), ctl->dpad(), ctl->buttons(),
      ctl->axisX(), ctl->axisY(), ctl->axisRX(), ctl->axisRY(),
      ctl->brake(), ctl->throttle(), ctl->miscButtons());
  }
};

void setup() {
  Serial.begin(115200);

  // Print firmware information and Bluetooth address
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %02X:%02X:%02X:%02X:%02X:%02X\n",
                addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Initialize the GamepadManager
  GamepadManager::instance().setup();
  
  // Allow time for systems to stabilize
  delay(1000);
  
  Serial.println("v0.70 Started!");
#if DEBUG_MODE
  Serial.println("DEBUG MODE IS ENABLED!");
#endif
}

void loop() {
  // Update the GamepadManager state
  GamepadManager::instance().update();
  
  // Small delay to prevent overwhelming the system
  delay(1);
}