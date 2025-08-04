import logging
import os
import sys
import subprocess

# --- Logger Setup ---
def setup_logger():
    """Configures the logger for the agent."""
    debug_mode = os.getenv("AGENT_DEBUG", "0") == "1"
    log_level = logging.DEBUG if debug_mode else logging.INFO

    logging.basicConfig(
        level=log_level,
        format='[%(asctime)s] [%(levelname)s] %(message)s',
        handlers=[
            logging.FileHandler("agent.log"),
            logging.StreamHandler(sys.stdout)
        ]
    )
    logging.info("Logger initialized in %s mode.", "DEBUG" if debug_mode else "INFO")

# --- Action Implementations ---

def action_list_directory(params=None):
    """
    Executes the 'ls -l' command and logs the output.

    :param params: A dictionary of parameters (not used in this action).
    :return: True if successful, False otherwise.
    """
    action_name = "list_directory"
    logging.info(f"Executing action: {action_name}")
    try:
        # Using a more specific command for clarity
        command = ["ls", "-l", "."]
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=10,
            check=True # This will raise CalledProcessError on non-zero exit codes
        )
        logging.debug(f"'{' '.join(command)}' stdout:\n{result.stdout}")
        if result.stderr:
            logging.warning(f"'{' '.join(command)}' stderr:\n{result.stderr}")
        return True
    except FileNotFoundError:
        logging.error(f"Command 'ls' not found. Please ensure it is in the system's PATH.")
        return False
    except subprocess.CalledProcessError as e:
        logging.error(f"Action '{action_name}' failed with exit code {e.returncode}:\n{e.stderr}")
        return False
    except subprocess.TimeoutExpired:
        logging.error(f"Action '{action_name}' timed out after 10 seconds.")
        return False
    except Exception as e:
        logging.error(f"An unexpected error occurred during action '{action_name}': {e}")
        return False

# --- Action Dispatcher ---

# The dispatcher maps action names to the functions that implement them.
ACTION_DISPATCHER = {
    "list_directory": action_list_directory,
    # New actions can be added here.
    # "create_file": action_create_file,
}

def run_action(action_name, params=None):
    """
    Looks up and runs an action from the dispatcher.

    :param action_name: The name of the action to run.
    :param params: A dictionary of parameters for the action.
    """
    logging.info(f"Received request to run action: '{action_name}'")

    if action_name in ACTION_DISPATCHER:
        action_func = ACTION_DISPATCHER[action_name]

        log_action_start(action_name)
        success = action_func(params)
        log_action_end(action_name, success)
    else:
        logging.error(f"Unknown action requested: '{action_name}'")
        log_action_end(action_name, success=False)

# --- Logging Helpers ---

def log_action_start(action_name):
    logging.info(f"--- Action '{action_name}' START ---")

def log_action_end(action_name, success=True):
    status = "SUCCESS" if success else "FAILURE"
    logging.info(f"--- Action '{action_name}' END (Status: {status}) ---")

# --- Main Execution ---

def main():
    """The main entry point for the agent."""
    setup_logger()
    logging.info("Agent process started.")

    # In the future, the action name could come from command-line arguments
    # or another source provided by the parent controller.
    # For now, we hardcode it for demonstration purposes.
    action_to_run = "list_directory"

    run_action(action_to_run)

    logging.info("Agent process finished.")

if __name__ == "__main__":
    main()
