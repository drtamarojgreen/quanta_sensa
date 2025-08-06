#!/bin/bash

# Integration Test for QuantaSensa Controller

# --- Setup ---
CONTROLLER="./parent_controller"
ACTIONS_PENDING="actions/pending"
QUEUE_COMPLETED="queue/completed"
TASK_FILE="task-123.sh"
RESULT_FILE="result-${TASK_FILE}.json"

# Clean up previous runs
rm -f $ACTIONS_PENDING/*
rm -f $QUEUE_COMPLETED/*
rm -f parent.log

echo "--- Starting Integration Test ---"

# 1. Create a sample task script
echo "echo 'Hello from task-123!'" > "$ACTIONS_PENDING/$TASK_FILE"
echo "Created task file: $ACTIONS_PENDING/$TASK_FILE"

# 2. Run the controller in the background
echo "Starting controller in the background..."
$CONTROLLER &
CONTROLLER_PID=$!

# Give the controller a moment to start up and find the script
# The controller polls every 5 seconds, so we should wait at least that long.
echo "Waiting for 7 seconds..."
sleep 7

# 3. Stop the controller
echo "Stopping controller (PID: $CONTROLLER_PID)..."
kill $CONTROLLER_PID
# Wait a moment for it to shut down cleanly
sleep 1

# 4. Check for the result file
echo "Checking for result file: $QUEUE_COMPLETED/$RESULT_FILE"
if [ ! -f "$QUEUE_COMPLETED/$RESULT_FILE" ]; then
    echo "!!! TEST FAILED: Result file was not created. !!!"
    echo "--- Log file (parent.log) ---"
    cat parent.log
    exit 1
fi

echo "Result file found."

# 5. Check the content of the result file
echo "Checking content of result file..."
if ! grep -q '"status": "completed"' "$QUEUE_COMPLETED/$RESULT_FILE"; then
    echo "!!! TEST FAILED: Result file does not contain 'status: completed'. !!!"
    echo "--- Result file content ---"
    cat "$QUEUE_COMPLETED/$RESULT_FILE"
    echo "--- Log file (parent.log) ---"
    cat parent.log
    exit 1
fi

echo "Result content is correct."

# 6. Check that the action script was removed
if [ -f "$ACTIONS_PENDING/$TASK_FILE" ]; then
    echo "!!! TEST FAILED: Action script was not removed from pending. !!!"
    exit 1
fi
echo "Action script was correctly removed."


echo "--- Integration Test PASSED! ---"
exit 0
