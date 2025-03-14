# UDP Plant Simulation Project

This project was developed for the Communication Protocols course (ENG10048) at UFRGS. The objective was to implement a client-server system using UDP/IP to simulate and control a plant with a single tank. The plant consists of an input valve, an output valve, and a level sensor, all normalized.

## Protocol Specification

The communication protocol between client and server follows a specific message format:

- **OpenValve**: `OpenValve#<seq>#<value>!`
  - `<value>`: Percentage of how much the valve should open.
  - Response: `Open#<seq>!`

- **CloseValve**: `CloseValve#<seq>#<value>!`
  - `<value>`: Percentage of how much the valve should close.
  - Response: `Close#<seq>!`

- **GetLevel**: `GetLevel!`
  - Returns the current tank level.
  - Response: `Level#<value>!`

- **CommTest**: `CommTest!`
  - Tests communication.
  - Response: `Comm#OK!`

- **SetMax**: `SetMax#<value>!`
  - Sets the maximum outflow rate.
  - Response: `Max#<value>!`

- **Start**: `Start!`
  - Starts or restarts the plant simulator.
  - Response: `Start#OK!`

- **Error**: If the server receives an unrecognized command.
  - Response: `Err!`

## System Architecture

The system is divided into two modules:

### Client Module
- **Control Thread**: Accesses the current tank level and controls the input valve opening via UDP.
- **Graphical Display Thread**: Displays the current tank level and the valve opening percentage.
  - Periodicity: 50ms

### Server Module
- **Plant Simulation Thread**: Simulates the plant behavior.
  - Periodicity: 10ms
- **Graphical Display Thread**: Displays the current tank level and the state of the input and output valves.
  - Periodicity: 50ms
- **UDP Server Thread**: Receives client commands and updates the plant parameters.

## Control Target

The goal of the control system is to maintain the tank level at 80%, with a maximum overshoot of 2%.

## Plant Model

The plant model simulates the behavior of the tank’s input and output flow, adjusting the tank level based on valve control and flow rates over time.

## Packet Loss Management

The project was implemented on a channel with a high packet loss rate. To handle this, the system was designed to ensure message reliability by:

- Storing sent messages and waiting for acknowledgment from the server.
- Checking the response for each sent message to confirm the execution of commands.
- Resending messages when acknowledgments were not received within a specific time frame, ensuring no command was lost due to network issues.
