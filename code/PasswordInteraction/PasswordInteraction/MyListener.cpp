#include "stdafx.h"
#include "MyListener.h"
#include <WindowsX.h>
#include "atlbase.h"
#include "atlstr.h"
void OutputDebugPrintf(const char* strOutputString, ...);



void MyListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void MyListener::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
 
}

void MyListener::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  std::cout << "Disconnected" << std::endl;
}

void MyListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

void MyListener::onFrame(const Controller& controller) {
  // Get the most recent frame and report some basic information
  const Frame frame = controller.frame();
  std::cout << "Frame id: " << frame.id()
            << ", timestamp: " << frame.timestamp()
            << ", hands: " << frame.hands().count()
            << ", extended fingers: " << frame.fingers().extended().count() << std::endl;

  HandList hands = frame.hands();
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    // Get the first hand
    const Hand hand = *hl;
    std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
    std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
              << ", palm position: " << hand.palmPosition() << std::endl;
    // Get the hand's normal vector and direction
    const Vector normal = hand.palmNormal();
    const Vector direction = hand.direction();

    // Calculate the hand's pitch, roll, and yaw angles
    std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
              << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
              << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;

    // Get the Arm bone
    Arm arm = hand.arm();
    std::cout << std::string(2, ' ') <<  "Arm direction: " << arm.direction()
              << " wrist position: " << arm.wristPosition()
              << " elbow position: " << arm.elbowPosition() << std::endl;

    // Get fingers
    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Finger finger = *fl;
      std::cout << std::string(4, ' ') << finger.type()
                << " finger, id: " << finger.id()
                << ", length: " << finger.length()
                << "mm, width: " << finger.width() << std::endl;

		if(finger.type()==1)
		{
			Vector stabilizedPosition = finger.stabilizedTipPosition();
			std::cout <<"position.x"<<stabilizedPosition.x<<"    position.y"<<stabilizedPosition.y<<"    position.z"<<stabilizedPosition.z<< std::endl;
			//OutputDebugPrintf("position.x=%f    position.y=%f    position.z=%f",stabilizedPosition.x,stabilizedPosition.y,stabilizedPosition.z);
			void Capture(float x,float y ,float z);
			Capture(stabilizedPosition.x,stabilizedPosition.y,stabilizedPosition.z);

		}

	 
    }
  }

  if (!frame.hands().isEmpty()) {
    std::cout << std::endl;
  }

}

void MyListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

void MyListener::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void MyListener::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
    std::cout << "  isSmudged:" << (devices[i].isSmudged() ? "true" : "false") << std::endl;
    std::cout << "  isLightingBad:" << (devices[i].isLightingBad() ? "true" : "false") << std::endl;
  }
}

void MyListener::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void MyListener::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}

void MyListener::onServiceChange(const Controller& controller) {
  std::cout << "Service Changed" << std::endl;
}

void MyListener::onDeviceFailure(const Controller& controller) {
  std::cout << "Device Error" << std::endl;
  const Leap::FailedDeviceList devices = controller.failedDevices();

  for (FailedDeviceList::const_iterator dl = devices.begin(); dl != devices.end(); ++dl) {
    const FailedDevice device = *dl;
    std::cout << "  PNP ID:" << device.pnpId();
    std::cout << "    Failure type:" << device.failure();
  }
}

void MyListener::onLogMessage(const Controller&, MessageSeverity s, int64_t t, const char* msg) {
  switch (s) {
  case Leap::MESSAGE_CRITICAL:
    std::cout << "[Critical]";
    break;
  case Leap::MESSAGE_WARNING:
    std::cout << "[Warning]";
    break;
  case Leap::MESSAGE_INFORMATION:
    std::cout << "[Info]";
    break;
  case Leap::MESSAGE_UNKNOWN:
    std::cout << "[Unknown]";
  }
  std::cout << "[" << t << "] ";
  std::cout << msg << std::endl;
}