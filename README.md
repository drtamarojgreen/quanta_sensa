# QuantaSensa

**QuantaSensa** is an autonomous workflow AI agent framework designed for resilient, self-aware operation in constrained Linux environments. It is engineered to build, monitor, and maintain itself by executing controlled, scheduled tasks while adapting to resource availability and system stability. The framework emphasizes security by restricting network access, learning from local resources only, and operating without continuous loops or cloud dependencies.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Core Concepts](#core-concepts)
- [Architecture](#architecture)
- [Ecosystem Architecture](#ecosystem-architecture)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Agent Behavior and Scheduling](#agent-behavior-and-scheduling)
- [Logging and Debugging](#logging-and-debugging)
- [Extending QuantaSensa](#extending-quantasensa)
- [Security Considerations](#security-considerations)
- [Contributing](#contributing)
- [License](#license)

---

## 🔍 Overview

QuantaSensa is built to operate fully offline within a sandboxed Fedora Linux environment called **PrismQuanta**. The system comprises:

- A **C++ parent controller** responsible for scheduling, launching, and monitoring the agent process with fine-grained control and resource awareness.
- A **Python autonomous agent** that executes single, discrete actions per invocation by interpreting text commands and learning exclusively from local Linux man pages.
- Integration with a local large language model for inference, enabling advanced intent recognition and self-improving code generation without internet access.

The framework is designed for scenarios where security and stability are paramount. It avoids common vulnerabilities associated with internet-connected AI agents by design, making it suitable for critical or isolated systems.

---

## 🧠 Core Concepts

- **Autonomous Self-Improvement:** The agent is designed to incrementally build and refine its capabilities. It learns from its actions and the environment, using local resources like `man` pages to understand and utilize new commands. This process is gradual and controlled, ensuring that the agent's development does not compromise system stability.
- **Controlled Execution:** Actions are executed one at a time with enforced pauses. This "single-shot" execution model prevents continuous loops and runaway processes, which is critical for maintaining system stability, especially in resource-constrained environments.
- **Resource Awareness:** The C++ parent controller actively monitors system resources such as CPU and memory. This allows the framework to make informed decisions about when to run the agent and to adjust its behavior based on resource availability, preventing system overload.
- **Offline Learning:** The agent's knowledge acquisition is strictly limited to local resources. This "air-gapped" learning approach is a core security feature, as it eliminates the risks associated with retrieving information from the internet.
- **Robustness and Resilience:** The architecture is designed for high availability. It includes mechanisms for graceful crash recovery, state persistence, and strict process lifecycle management, ensuring that the agent can recover from failures and maintain a consistent state.

---

## 🏗️ Architecture

```
┌────────────────────┐
│ C++ Parent Control │
│ - Scheduling       │
│ - Process launch   │
│ - Resource monitor │
│ - ESC exit handler │
└─────────┬──────────┘
          │
          ▼
┌────────────────────┐
│   Python Agent    │
│ - LLM inference    │
│ - Single action    │
│ - Man page parser  │
│ - Logging          │
└────────────────────┘
```

### Components

#### **Parent Controller (C++)**
The C++ parent controller is the heart of the QuantaSensa framework. It runs on-demand and is responsible for the following:
- **Scheduling:** Checks if the current time is within an allowed window before launching the agent.
- **Process Management:** Launches the Python agent, waits for its completion, and captures its output.
- **Resource Monitoring:** Keeps an eye on system resources to ensure the agent does not cause instability.
- **User Control:** Listens for an ESC key press to allow the user to terminate the agent's operation gracefully.
- **Logging:** Records all significant events, including errors and state changes, for debugging and auditing purposes.

#### **Autonomous Agent (Python)**
The Python agent is the "brains" of the operation. It is designed to be lightweight and single-purpose:
- **Action Execution:** Performs one specific action per invocation. This could be a file operation, a shell command, or a self-modification task.
- **Intent Parsing:** Uses a local large language model to understand commands and determine the appropriate action to take.
- **Learning:** Reads and indexes local `man` pages to learn how to use new commands and expand its capabilities.
- **Logging:** Maintains a detailed log of its activities, which is essential for understanding its behavior and for debugging.

---

## 🏛️ Ecosystem Architecture

While `QuantaSensa` is the core execution agent, it is designed to be part of a larger, multi-component **PrismQuanta Ecosystem**. This ecosystem is a cohesive, autonomous system built on a philosophy of local-first operation, modularity, and ethical governance.

### Core Components

-   **QuantaLista (Strategist):** Manages the master project plan, breaking down high-level goals into a queue of tasks.
-   **QuantaPorto (Commander):** Translates tasks from `QuantaLista` into a structured language (PQL) and schedules them for execution.
-   **QuantaSensa (Field Agent):** The hands of the system, responsible for executing a single scheduled task resiliently.
-   **QuantaGlia (Librarian):** The knowledge manager, responsible for analyzing repositories and logs to spawn new improvement tasks.
-   **QuantaEthos (Conscience):** A mandatory validation service that provides a go/no-go decision for any potentially impactful command.

This architecture allows for a clear separation of concerns, from high-level strategy to low-level execution. For a complete overview of the integration strategy, communication protocols, and the phased roadmap, please see the full **[Ecosystem Integration Plan](./docs/integration_plan.md)**.

---

## ✨ Features

- **No `while(true)` loops:** Execution is intentionally non-continuous, triggered by the parent controller's scheduling logic to ensure deliberate, observable actions.
- **Scheduled Activity Windows:** The agent is configured to run only during specific, predefined hours (e.g., overnight) to minimize impact on system performance during peak usage.
- **Single Action Per Run:** Each invocation of the agent results in a single, atomic action, which simplifies debugging, reduces resource consumption, and prevents runaway processes.
- **User-Controlled Interrupt:** A built-in ESC key listener in the parent controller allows the user to safely interrupt the agent's execution cycle during the pause interval.
- **Local-Only Learning:** The agent is strictly sandboxed to learn from local system resources, primarily `man` pages. This design choice enhances security by prohibiting internet access.
- **Detailed and Segregated Logging:** Both the parent controller and the Python agent maintain their own detailed log files, with multiple verbosity levels, to provide a clear audit trail of system behavior.
- **Modular and Configurable Actions:** A simple, human-readable configuration file (e.g., YAML or JSON) maps recognized intents to specific, executable actions, making it easy to extend the agent's capabilities.
- **Crash Resilience and Recovery:** The parent controller is designed to detect abnormal agent termination. It can be configured to automatically restart the agent or quarantine it to prevent further issues.

---

## 🚀 Installation

### Prerequisites

- Fedora Linux environment (preferably in PrismQuanta sandbox)
- C++17 compatible compiler (e.g., `g++`)
- Python 3.8+
- `psutil` Python package (for optional resource monitoring)
- LLM model files stored locally

### Build Parent Controller

```bash
g++ -std=c++17 -o parent_controller src/main.cpp src/config.cpp src/logger.cpp -Iinclude -pthread
```

### Prepare Python Agent Environment

```bash
python3 -m venv env
source env/bin/activate
pip install psutil
```

---

## 💻 Usage

### Running the Parent Controller

To start the QuantaSensa framework, run the parent controller from the command line:
```bash
./parent_controller
```

The controller will then:
1. Check if the current time is within the allowed window.
2. If it is, the controller will launch the Python agent and wait for it to complete.
3. After the agent finishes, the controller will wait for a short period, during which it will listen for the ESC key.
4. All activities are logged to `parent.log`.

### Running the Python Agent (Standalone)

You can also run the Python agent on its own for testing or debugging purposes:
```bash
AGENT_DEBUG=1 python3 agent.py
```

When run in standalone mode, the agent will:
- Execute a single action based on its internal knowledge and the configured rules.
- Log detailed debug information to `agent.log`.

---

## ⚙️ Configuration

### Time Window Settings

To change the agent's allowed run hours, modify the `is_in_time_window()` function in `parent_controller.cpp`.

### Rules and Actions

The agent's behavior is defined by a set of rules in a configuration file (e.g., `rules.yaml`). These rules map intents to actions.

**Example `rules.yaml`:**

```yaml
create_note:
  action: append_to_file
  target: "~/quantasensa/notes.md"

list_dir:
  action: exec
  command: "ls -l"

system_info:
  action: exec
  command: "uname -a && df -h"
```

---

## ⏰ Agent Behavior and Scheduling

- The parent controller is the sole authority for scheduling, which prevents the agent from running in a continuous loop.
- The agent is designed to perform a single atomic action and then exit. This ensures that each of the agent's actions is deliberate and that its impact on the system is predictable.
- A 15-second pause is enforced by the parent controller between agent runs. This "cool-down" period provides a window for the user to interrupt the agent's operation.
- This single-shot, scheduled execution model is designed to be robust and stable, even on systems that are slow or prone to crashing.

---

## 📊 Logging and Debugging

### Log Files

- **Parent logs:** `parent.log`
  - Contains timestamps, log levels, process start/end events, errors, and other key events.
  
- **Agent logs:** `agent.log`
  - Captures the details of each action, including the start and end of the action, the output of any commands, and any exceptions that were encountered.

### Debug Mode

To enable debug logging in the agent, set the `AGENT_DEBUG` environment variable:
```bash
export AGENT_DEBUG=1
```

### Real-time Log Monitoring

You can monitor the logs of both the parent and the agent in real-time using the `tail` command:
```bash
tail -f parent.log agent.log
```

### Log Level Configuration

| Component | Log Levels | Debug Toggle |
|-----------|------------|--------------|
| Parent (C++) | ERROR, INFO, DEBUG | Hardcoded in code |
| Agent (Python) | ERROR, INFO, DEBUG | `AGENT_DEBUG` env var |

---

## 🔧 Extending QuantaSensa

- **Add new actions:** The easiest way to extend QuantaSensa is to add new rules to the configuration file. This allows you to map new intents to existing action types.
- **Enhanced task parsing:** For more complex tasks, you can extend the Python agent with new modules. This allows you to add custom logic for parsing and executing tasks.
- **Resource monitoring:** You can implement more sophisticated resource-aware scaling by adding monitoring capabilities to the parent controller and using this information to create feedback loops.
- **State persistence:** To improve crash recovery, you can incorporate checkpointing to save the agent's state periodically.

### Example Extension: Resource Monitoring

```cpp
// In parent_controller.cpp
void log_system_resources() {
    // Add CPU/memory usage logging
    logger.log(DEBUG, "CPU usage: " + get_cpu_usage());
    logger.log(DEBUG, "Memory usage: " + get_memory_usage());
}
```

---

## 🔒 Security Considerations

- **Network isolation:** The agent is designed to operate without any network access, which is a fundamental security feature.
- **Local-only learning:** The agent's learning is restricted to local system resources, which prevents it from being influenced by potentially malicious external sources.
- **Sandboxing:** The parent controller can be sandboxed using standard Linux security features like namespaces, cgroups, and seccomp filters to further limit its capabilities.
- **Privilege separation:** The agent should be run with the minimum required privileges to reduce its potential impact on the system.
- **Input validation:** All commands and file operations should be carefully validated before execution to prevent security vulnerabilities.

---

## 🤝 Contributing

Contributions are welcome! Please open an issue or submit a pull request for any of the following:

- New actions or examples of rules
- Improvements to the logging or monitoring capabilities
- Enhancements to the self-maintenance features
- Performance optimizations
- Security improvements

### Development Guidelines

1. Follow the existing code style and structure.
2. Add comprehensive logging for any new features.
3. Include unit tests where applicable.
4. Update the documentation to reflect any new configuration options.

---

## 📄 License

MIT License — see [LICENSE](LICENSE) for details.

---

**QuantaSensa** is a robust foundation for autonomous agents in constrained, secure environments. Designed to think locally, act deliberately, and maintain itself over time — all while respecting the limitations of its host system.

---

## 📚 Quick Reference

### Key Commands

| Command | Description |
|---------|-------------|
| `./parent_controller` | Start the main controller |
| `AGENT_DEBUG=1 python3 agent.py` | Run agent with debug logging |
| `tail -f *.log` | Monitor all log files |
| **ESC key** | Exit parent controller early |

### Important Files

- `parent_controller.cpp` - Main C++ controller source
- `agent.py` - Python autonomous agent
- `rules.yaml` - Action configuration
- `parent.log` - Controller logs
- `agent.log` - Agent activity logs