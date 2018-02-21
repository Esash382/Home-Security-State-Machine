# Home-Security-State-Machine
A program which implements a simple, configurable Home Security state machine.

This project uses C++ Rest SDK to implement a simple home security state machine.
The specifications are that the application should read configuration file on startup (JSON encoded). The file should contain definitions of states, transition events, and actions which have to be executed on transition. Simple HTTP REST interface which should be used to inject events.

Supported events:
- DISARMED
- ENTER_CODE
- SEND_OK_RESPONSE
- SEND_ERROR_RESPONSE
- ARMED
An event can have an optional parameter, for instance, a user code.

Actions:
The program supports 3 type of actions (methods):
- Log(text) - prints content of parameter into stdout
- ValidateCode(code) nextState - Action should validate code and set next state
- SendResponse(response_text) - Action sends response to pending REST request
It should be possible using the same definition file to set a process back from ARMED
state to DISARMED state and also define new states like HOME_AWAY, etc.

The project used many MSDN examples as a start to implement this state machine.

Workflow:
To arm the device:
- The server receives a request from a client to arm the device
- The server request code from the client
- The client provides the code
- The server checks if the code is valid and if valid, arms the device; if invalid, doesn't arm the device!

To disarm the device:
- The server receives a request from a client to disarm the device
- The server request code from the client
- The client provides the code
- The server checks if the code is valid and if valid, disarms the device; if invalid, doesn't disarm the device!
