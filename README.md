# Luluk Rover - Omnidirectional FPV Robot
#### Video Demo:  <YOUR_YOUTUBE_VIDEO_LINK_HERE>
#### Description:

Luluk Rover is an omnidirectional, FPV (First Person View) robotic vehicle designed as my final project for CS50. The core idea behind this project was to integrate hardware engineering with full-stack web development to create a seamless remote-controlled experience over a local Wi-Fi network.

The project consists of two main hardware components working in tandem:
1. **ESP32 Microcontroller:** Acts as the main brain of the rover. It hosts an asynchronous web server that serves a responsive, dark-themed HTML/CSS/JavaScript dashboard. This dashboard captures user inputs (touch/clicks for movement and sliders for a robotic arm) and sends HTTP requests via the Fetch API back to the ESP32. The ESP32 then translates these requests into PWM signals to control four DC motors via L298N motor drivers and four servos forming a robotic arm.
2. **ESP32-CAM (AI-Thinker):** Operates on the same Wi-Fi network to provide a live MJPEG video stream directly to the web dashboard, enabling the FPV capability.

**Key Features:**
*   **Mecanum Wheel Kinematics:** The rover can move forward, backward, turn, and strafe diagonally without changing its orientation.
*   **Web-Based Control Dashboard:** A custom-built, mobile-responsive UI that includes a camera feed frame, D-Pad for movement, and sliders for robotic arm joint control (Base, Shoulder, Elbow, Gripper).
*   **Pre-programmed Macros:** The robotic arm includes specific automated animation sequences (e.g., "Angry", "Accept", "Reject", "Scan", and "Pickup") triggered from the dashboard.

**Hardware Used:**
*   1x ESP32 NodeMCU
*   1x ESP32-CAM (AI-Thinker module)
*   2x L298N Motor Drivers
*   4x TT DC Motors with Mecanum Wheels
*   4x SG90/MG995 Servos (for the arm mechanism)
*   1x Relay Module

**Files Overview:**
*   `ESP32_Controller.ino`: Contains the C++ logic for the main ESP32. It initializes the Wi-Fi connection, sets up the web server on port 80, stores the `index_html` interface in PROGMEM, and handles all routing (`/move`, `/servo`, `/macro`, `/speed`) to control the physical hardware.
*   `ESP32_CAM.ino`: Configures the camera sensor pins, connects to Wi-Fi, and streams the JPEG frames over a multipart HTTP response.

**Academic Honesty & AI Acknowledgement:**
In accordance with CS50's policies, I acknowledge that Artificial Intelligence tools (such as ChatGPT/Claude) were utilized during the development of this project. Specifically, AI was used to assist in generating the boilerplate HTML/CSS/JS for the web dashboard, optimizing the C++ motor control algorithms, and debugging Wi-Fi connectivity issues. All generated code was thoroughly reviewed, understood, and manually integrated by me into the final architecture. I fully comprehend the logic behind the web requests, hardware pin mappings, and overall system integration.
