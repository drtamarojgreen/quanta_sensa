import unittest
from unittest.mock import patch, MagicMock
import os
import subprocess

# This is a bit tricky. We need to test a script from another directory.
# We'll add the 'src' directory to the python path to allow the import.
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../src')))

# Now we can import the agent
import agent

class TestAgent(unittest.TestCase):

    def setUp(self):
        """Set up a mock logger for all tests."""
        # The agent's logger is configured globally, so we can patch it once.
        patcher = patch('agent.logging')
        self.addCleanup(patcher.stop)
        self.mock_logging = patcher.start()

    @patch('agent.action_list_directory')
    def test_run_action_dispatcher_success(self, mock_action):
        """Test that the dispatcher calls the correct action function."""
        agent.run_action("list_directory")
        mock_action.assert_called_once()

    def test_run_action_dispatcher_unknown_action(self):
        """Test that the dispatcher logs an error for an unknown action."""
        agent.run_action("non_existent_action")
        # Check that an error was logged
        self.mock_logging.error.assert_called_with("Unknown action requested: '%s'", "non_existent_action")

    @patch('subprocess.run')
    def test_action_list_directory_success(self, mock_subprocess_run):
        """Test the list_directory action on successful execution."""
        # Configure the mock to return a successful result
        mock_result = MagicMock()
        mock_result.stdout = "total 0\ndrwxr-xr-x 1 user user 0 Oct 27 10:00 mydir"
        mock_result.stderr = ""
        mock_subprocess_run.return_value = mock_result

        success = agent.action_list_directory()

        self.assertTrue(success)
        mock_subprocess_run.assert_called_once_with(
            ["ls", "-l", "."],
            capture_output=True,
            text=True,
            timeout=10,
            check=True
        )
        # Verify that stdout was logged at debug level
        self.mock_logging.debug.assert_called()

    @patch('subprocess.run')
    def test_action_list_directory_failure(self, mock_subprocess_run):
        """Test the list_directory action when the command fails."""
        # Configure the mock to raise a CalledProcessError
        mock_subprocess_run.side_effect = subprocess.CalledProcessError(
            returncode=1,
            cmd=["ls", "-l", "."],
            stderr="ls: cannot access '.': No such file or directory"
        )

        success = agent.action_list_directory()

        self.assertFalse(success)
        self.mock_logging.error.assert_called()


if __name__ == '__main__':
    unittest.main()
