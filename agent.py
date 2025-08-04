import logging
import os
import sys

DEBUG = os.getenv("AGENT_DEBUG", "0") == "1"

logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    format='[%(asctime)s] [%(levelname)s] %(message)s',
    handlers=[
        logging.FileHandler("agent.log"),
        logging.StreamHandler(sys.stdout)
    ]
)

def log_action_start(action):
    logging.info(f"Starting action: {action}")

def log_action_end(action, success=True):
    if success:
        logging.info(f"Completed action: {action}")
    else:
        logging.error(f"Action failed: {action}")

def main():
    logging.info("Agent started")

    # Example action
    action = "list directory"
    log_action_start(action)

    import subprocess
    try:
        output = subprocess.check_output("ls -l", shell=True, stderr=subprocess.STDOUT, timeout=10)
        logging.debug(output.decode())
        log_action_end(action, True)
    except Exception as e:
        logging.error(f"Exception during action: {e}")
        log_action_end(action, False)

    logging.info("Agent finished")

if __name__ == "__main__":
    main()
