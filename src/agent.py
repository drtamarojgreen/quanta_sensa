import logging
import os
import sys
import time

# --- Constants ---
EVENT_PIPE_PATH = "/tmp/prismquanta_events.pipe"

# --- Logger Setup ---
def setup_logger():
    """Configures the logger for the agent."""
    logging.basicConfig(
        level=logging.DEBUG,
        format='[%(asctime)s] [PythonAgent] [%(levelname)s] %(message)s',
        handlers=[
            logging.FileHandler("agent.log", mode='w'),
            logging.StreamHandler(sys.stdout)
        ]
    )

# --- Event Handling ---
def handle_event(event_line):
    """
    Parses and handles a single event line from the pipe.
    """
    try:
        event_type, event_path = event_line.strip().split(':', 1)
        logging.info(f"Received event: Type='{event_type}', Path='{event_path}'")

        if event_type == "CREATE" and "actions/pending" in event_path:
            logging.info(f"A new script was added to the pending queue: {event_path}")

    except ValueError:
        logging.warning(f"Received a malformed event line: '{event_line.strip()}'")
    except Exception as e:
        logging.error(f"An error occurred while handling event '{event_line.strip()}': {e}")


# --- Main Execution ---
def main():
    """The main entry point for the agent service."""
    setup_logger()
    logging.info("Agent service started. PID: %d", os.getpid())

    logging.debug(f"Waiting for event pipe: {EVENT_PIPE_PATH}")
    while not os.path.exists(EVENT_PIPE_PATH):
        time.sleep(0.1)

    logging.info(f"Event pipe found. Opening for reading...")

    try:
        with open(EVENT_PIPE_PATH, "r") as pipe:
            logging.info("Pipe opened successfully. Listening for events...")
            while True:
                line = pipe.readline()
                if line:
                    handle_event(line)
                else:
                    logging.info("Write-end of pipe closed. Shutting down agent.")
                    break
    except Exception as e:
        logging.error(f"An error occurred with the event pipe: {e}", exc_info=True)
    finally:
        logging.info("Agent service finished.")

if __name__ == "__main__":
    main()
