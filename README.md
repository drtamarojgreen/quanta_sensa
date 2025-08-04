# QuantaSensa

**QuantaSensa** is an autonomous workflow AI agent framework designed for resilient, self-aware operation in constrained Linux environments. It is engineered to build, monitor, and maintain itself by executing controlled, scheduled tasks while adapting to resource availability and system stability. The framework emphasizes security by restricting network access, learning from local resources only, and operating without continuous loops or cloud dependencies.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Core Concepts](#core-concepts)
- [Architecture](#architecture)
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

- A **C++ parent controller** responsible for scheduling, launching, and monitoring the agent process with fine-grained control and resource awareness
- A **Python autonomous agent** that executes single, discrete actions per invocation by interpreting text commands and learning exclusively from local Linux man pages
- Integration with **llama.cpp** for local large language model inference, enabling advanced intent recognition and self-improving code generation without internet access

---

## 🧠 Core Concepts

- **Autonomous Self-Improvement:** The agent is designed to incrementally build and refine its capabilities based on observed behavior and resource constraints
- **Controlled Execution:** Actions are executed one at a time with enforced pauses, avoiding continuous loops and ensuring system stability
- **Resource Awareness:** The parent controller observes resource consumption and system health, allowing the agent to adapt or scale down workloads
- **Offline Learning:** Linux man pages serve as the sole source for learning system commands, emphasizing security by excluding web access
- **Robustness:** The architecture supports graceful crash recovery, state persistence, and strict process lifecycle management

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
Runs on-demand, checks the allowed time window, launches the agent, waits for completion, monitors resources, listens for ESC key to exit early, logs events with detailed debug info.

#### **Autonomous Agent (Python)**
Performs one action per invocation using llama.cpp for intent parsing, reads and indexes local man pages for learning, executes shell commands or scripts, and logs detailed activity and errors.

---

## ✨ Features

- **No `while(true)` loops:** Execution is triggered externally or by parent scheduling logic
- **Scheduled activity windows:** Agent runs only during predefined hours (e.g., night time)
- **Single action per run:** Ensures low resource consumption and avoids runaway processes
- **ESC key interrupt:** User can safely exit the parent controller during wait periods
- **Local-only learning:** No internet or external web access allowed; strict sandboxing
- **Detailed logging:** Both parent and agent maintain separate log files with multiple verbosity levels
- **Modular rule system:** YAML or JSON configuration maps recognized intents to local executable actions
- **Crash resilience:** Parent detects abnormal agent termination and can restart or quarantine

---

## 🚀 Installation

### Prerequisites

- Fedora Linux environment (preferably in PrismQuanta sandbox)
- C++17 compatible compiler (e.g., `g++`)
- Python 3.8+ with `llama-cpp-python` bindings installed
- `psutil` Python package (for optional resource monitoring)
- `llama.cpp` model files stored locally

### Build Parent Controller

```bash
g++ -std=c++17 -o parent_controller parent_controller.cpp -pthread
```

### Prepare Python Agent Environment

```bash
python3 -m venv env
source env/bin/activate
pip install llama-cpp-python psutil
```

---

## 💻 Usage

### Running the Parent Controller

```bash
./parent_controller
```

1. The controller checks if current time is in allowed window
2. If yes, launches the Python agent and waits for its completion
3. Listens for ESC key during a 15-second post-action wait
4. Logs all activity to `parent.log`

### Running the Python Agent (Standalone)

```bash
AGENT_DEBUG=1 python3 agent.py
```

- Executes one action based on internal knowledge base and llama.cpp model inference
- Logs detailed debug info to `agent.log`

---

## ⚙️ Configuration

### Time Window Settings

Modify `is_in_time_window()` in `parent_controller.cpp` to set allowed run hours.

### Rules and Actions

Configure `rules.yaml` (or `rules.json`) to map detected intents to local commands or scripts.

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

- The parent controller manages when and how often the agent runs, preventing continuous loops
- Agent performs a single atomic action each run and exits promptly
- Between runs, the parent enforces a 15-second pause, listening for user input to exit early
- This model ensures stability on slow or crash-prone Linux systems

---

## 📊 Logging and Debugging

### Log Files

- **Parent logs:** `parent.log`
  - Includes timestamps, log levels, process start/end, errors, and key events
  
- **Agent logs:** `agent.log`
  - Captures action start/end, output from commands, LLM inference details, and exceptions

### Debug Mode

Enable debug logging in the agent by setting environment variable:

```bash
export AGENT_DEBUG=1
```

### Real-time Log Monitoring

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

- **Add new actions:** Update the rules config with new intent-to-action mappings
- **Enhanced task parsing:** Extend the Python agent with modules for more complex execution logic
- **Resource monitoring:** Implement resource-aware scaling by adding monitoring in parent and feedback loops
- **State persistence:** Incorporate checkpointing to enhance crash recovery capabilities

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

- **Network isolation:** Agent strictly avoids any network access
- **Local-only learning:** Command execution relies only on local system resources and man pages
- **Sandboxing:** Parent controller can be sandboxed using Linux namespaces, cgroups, and seccomp filters
- **Privilege separation:** Run with minimal required permissions
- **Input validation:** All commands and file operations are validated before execution

---

## 🤝 Contributing

Contributions are welcome! Please open issues or pull requests for:

- New actions or rule examples
- Improved logging or monitoring capabilities
- Enhanced self-maintenance features
- Performance optimizations
- Security improvements

### Development Guidelines

1. Follow existing code style and structure
2. Add comprehensive logging for new features
3. Include unit tests where applicable
4. Update documentation for any new configuration options

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