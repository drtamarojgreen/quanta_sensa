import unittest
from unittest.mock import patch
import os
import sys

# Add the 'src' directory to the python path to allow the import.
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../src')))

# Now we can import the agent
import agent

class TestAgentEventHandling(unittest.TestCase):

    def setUp(self):
        """Set up a mock logger before each test."""
        patcher = patch('agent.logging')
        self.addCleanup(patcher.stop)
        self.mock_logging = patcher.start()

    def test_handle_create_event(self):
        """Test handling of a valid CREATE event."""
        event_line = "CREATE:/path/to/new_file.txt\n"
        agent.handle_event(event_line)

        # Verify that the event was logged correctly
        self.mock_logging.info.assert_any_call("Received event: Type='CREATE', Path='/path/to/new_file.txt'")

    def test_handle_modify_event(self):
        """Test handling of a valid MODIFY event."""
        event_line = "MODIFY:/path/to/another_file.py\n"
        agent.handle_event(event_line)

        self.mock_logging.info.assert_any_call("Received event: Type='MODIFY', Path='/path/to/another_file.py'")

    def test_handle_delete_event(self):
        """Test handling of a valid DELETE event."""
        event_line = "DELETE:/path/to/deleted_file.log\n"
        agent.handle_event(event_line)

        self.mock_logging.info.assert_any_call("Received event: Type='DELETE', Path='/path/to/deleted_file.log'")

    def test_handle_special_action_event(self):
        """Test that a special log message is created for new pending actions."""
        event_line = "CREATE:actions/pending/new_script.sh\n"
        event_path = "actions/pending/new_script.sh"
        agent.handle_event(event_line)

        self.mock_logging.info.assert_any_call("Received event: Type='CREATE', Path='actions/pending/new_script.sh'")
        self.mock_logging.info.assert_any_call(f"A new script was added to the pending queue: {event_path}")

    def test_handle_malformed_event(self):
        """Test handling of a malformed event line without a colon."""
        event_line = "THIS IS A BAD EVENT\n"
        agent.handle_event(event_line)

        # Verify that a warning was logged
        self.mock_logging.warning.assert_called_with(f"Received a malformed event line: '{event_line.strip()}'")

    def test_handle_empty_event(self):
        """Test handling of an empty event line."""
        event_line = "\n"
        # This should not raise an error, but might log a warning depending on implementation.
        # In our case, strip() will make it empty and split will fail.
        agent.handle_event(event_line)
        self.mock_logging.warning.assert_called_with(f"Received a malformed event line: ''")


if __name__ == '__main__':
    unittest.main()
