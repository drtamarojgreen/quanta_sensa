# QuantaSensa Implementation Plan

## 1. Overview

This document outlines a detailed implementation plan for the QuantaSensa framework. The plan is based on the architecture described in the `README.md` and the existing source code. The goal is to create a robust, resilient, and secure autonomous agent framework.

## 2. Core Components

### 2.1. C++ Parent Controller (`parent_controller.cpp`)

The parent controller is the entry point of the application. It is responsible for scheduling, launching, and monitoring the Python agent.

- **[ ] Configuration:**
    -   [ ] Implement a configuration loader (e.g., from a `.ini` or `.yaml` file) to manage settings like:
        -   Agent script path
        -   Allowed time window
        -   Post-action wait time
        -   Log file path and level
- **[ ] Scheduling:**
    -   [ ] Refine the `is_in_time_window()` function to be more flexible (e.g., support multiple time windows, weekdays/weekends).
- **[ ] Agent Launching:**
    -   [ ] Enhance the `launch_agent()` function to:
        -   Pass command-line arguments to the agent (e.g., for different modes of operation).
        -   Capture and log `stdout` and `stderr` from the agent process.
- **[ ] Resource Monitoring:**
    -   [ ] Implement functions to monitor system resources (CPU, memory, disk space).
    -   [ ] The controller should log these metrics and potentially adjust the agent's behavior based on resource availability.
- **[ ] State Management:**
    -   [ ] Implement a mechanism to save and restore the application's state (e.g., last run time, agent status). This is crucial for crash recovery.
- **[ ] Error Handling:**
    -   [ ] Improve error handling to cover more edge cases (e.g., agent script not found, permission errors).

### 2.2. Python Agent (`agent.py`)

The Python agent performs the actual tasks. It is designed to be a single-action process, meaning it does one thing and then exits.

- **[ ] Action Dispatcher:**
    -   [ ] Create a central dispatcher that reads a command (e.g., from `stdin` or command-line arguments) and executes the corresponding action.
- **[ ] Action Library:**
    -   [ ] Develop a library of actions that the agent can perform. These actions should be modular and easy to extend. Examples include:
        -   `file_operation`: create, delete, modify files.
        -   `shell_command`: execute a shell command.
        -   `self_update`: update the agent's own code.
- **[ ] LLM Integration:**
    -   [ ] Integrate with a local LLM (like `llama.cpp`) to enable natural language understanding. The agent should be able to parse a command like "create a note about my new idea" and translate it into a specific action.
- **[ ] Learning Module:**
    -   [ ] Implement a module that allows the agent to learn from local resources, such as `man` pages. This could involve parsing `man` pages to understand how to use new commands.
- **[ ] Security:**
    -   [ ] Implement security measures to prevent the agent from performing dangerous actions. This could include a "sandbox" mode that restricts which commands can be executed.

### 2.3. Logging (`logger.cpp` and `logger.py`)

Logging is critical for monitoring and debugging the application.

- **[ ] C++ Logger:**
    -   [ ] Add log rotation to prevent log files from growing indefinitely.
    -   [ ] Improve performance by using a more efficient logging library or by implementing asynchronous logging.
- **[ ] Python Logger:**
    -   [ ] Unify the Python logger with the C++ logger. The agent should log to the same file as the parent controller, or to a separate file that is also monitored by the controller.
    -   [ ] The duplicate `logger.py` file should be removed and its functionality merged into `agent.py`.

## 3. Development Workflow

1.  **Phase 1: Core Functionality (M1)**
    -   Implement the basic C++ parent controller and Python agent.
    -   Establish the communication channel between the two components.
    -   Implement basic logging.
2.  **Phase 2: Action Library and LLM (M2)**
    -   Develop the action library for the Python agent.
    -   Integrate the LLM for natural language understanding.
3.  **Phase 3: Learning and Self-Improvement (M3)**
    -   Implement the learning module.
    -   Develop the agent's ability to self-update.
4.  **Phase 4: Hardening and Optimization (M4)**
    -   Improve security and error handling.
    -   Optimize performance.
    -   Add comprehensive tests.

## 4. Testing Strategy

-   **[ ] Unit Tests:** Each module should have its own set of unit tests.
-   **[ ] Integration Tests:** Test the interaction between the parent controller and the agent.
-   **[ ] End-to-End Tests:** Test the entire application in a simulated environment.
-   **[ ] Performance Tests:** Measure resource usage and execution time.

This plan provides a roadmap for developing the QuantaSensa framework. By following this plan, we can create a powerful and reliable autonomous agent.
