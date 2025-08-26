#!/bin/bash

# Integration Test for QuantaSensa Event System

# --- Setup ---
CONTROLLER="./parent_controller"
AGENT_LOG="agent.log"
PARENT_LOG="parent.log"
EVENT_PIPE="/tmp/prismquanta_events.pipe"
TEST_DIR="src"
TEST_FILE="$TEST_DIR/integration_test_file.txt"

# Clean up previous runs and ensure directories exist
rm -f "$TEST_FILE"
rm -f "$AGENT_LOG"
rm -f "$PARENT_LOG"
rm -f "$EVENT_PIPE"
mkdir -p actions/pending
mkdir -p src # Ensure src also exists

echo "--- Starting Event Integration Test ---"

# 1. Start the controller in the background, capturing its stdout
echo "Starting controller in the background..."
$CONTROLLER > controller_stdout.log 2>&1 &
CONTROLLER_PID=$!

# 2. Wait for the agent to be ready
# We'll wait for the agent to log that it's listening on the pipe.
# Since the agent's stdout is now mixed with the controller's, we check the combined log.
echo "Waiting for agent to open event pipe..."
timeout 10s grep -q "Pipe opened successfully. Listening for events..." <(tail -f controller_stdout.log)
if [ $? -ne 0 ]; then
    echo "!!! TEST FAILED: Agent did not start listening on the pipe in time. !!!"
    echo "--- Controller stdout Log ---"
    cat controller_stdout.log
    echo "--- Parent Log ---"
    cat "$PARENT_LOG"
    kill $CONTROLLER_PID
    exit 1
fi
echo "Agent is ready."

# 3. Create a file to trigger an event
echo "Creating test file to trigger event: $TEST_FILE"
echo "test content" > "$TEST_FILE"

# 4. Check the agent's log for the event
echo "Waiting for 3 seconds for event to be processed..."
sleep 3

echo "Checking agent log for event..."
if ! grep -q "Received event: Type='CREATE', Path='$TEST_FILE'" "controller_stdout.log"; then
    echo "!!! TEST FAILED: Agent log does not contain the expected CREATE event. !!!"
    echo "--- Controller stdout Log ---"
    cat controller_stdout.log
    echo "--- Parent Log ---"
    cat "$PARENT_LOG"
    kill $CONTROLLER_PID
    rm -f "$TEST_FILE"
    exit 1
fi

echo "Event correctly received by agent."

# --- Teardown ---
echo "Cleaning up..."
kill $CONTROLLER_PID
rm -f "$TEST_FILE"
rm -f "$EVENT_PIPE"

echo "--- Event Integration Test PASSED! ---"
exit 0
