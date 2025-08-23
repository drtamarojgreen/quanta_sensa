Feature: Python Agent Action Execution

  As the "brains" of the operation, the Python agent must correctly
  interpret and execute requested actions.

  Scenario: Execution of a known action
    Given the agent is requested to run the "list_directory" action
    When the agent's main function is called
    Then the "action_list_directory" function is executed
    And the agent logs a success message

  Scenario: Attempt to execute an unknown action
    Given the agent is requested to run the "fly_to_the_moon" action
    When the agent's main function is called
    Then no action function is executed
    And the agent logs an "Unknown action" error message

  Scenario: Action fails due to external command error
    Given the agent is requested to run the "list_directory" action
    And the underlying "ls -l" command will fail
    When the "action_list_directory" function is executed
    Then the function returns a failure status
    And the agent logs an error message containing the command's stderr
