Feature: Parent Controller Script Processing

  As the main control program, the Parent Controller must reliably
  find, execute, and manage scripts from a file-based queue system.

  Scenario: Successful execution of a pending script
    Given a script file exists in the "actions/pending" directory
    And the "quanta-ethos" validator will approve the script
    When the parent controller polls for scripts
    Then the script is moved from "pending" to "in_progress"
    And the script's content is validated by "quanta-ethos"
    And the script is executed
    And a result file is created in the "queue/completed" directory
    And the script file is removed from the "in_progress" directory

  Scenario: Handling of a script that fails validation
    Given a script file exists in the "actions/pending" directory
    And the "quanta-ethos" validator will reject the script
    When the parent controller polls for scripts
    Then the script is moved from "pending" to "in_progress"
    And the script's content is validated by "quanta-ethos"
    And the script is moved from "in_progress" to the "actions/failed" directory
    And a result file is created in the "queue/failed" directory indicating validation failure

  Scenario: Handling of a script that fails during execution
    Given a script file exists in the "actions/pending" directory
    And the "quanta-ethos" validator will approve the script
    And the script will exit with a non-zero status code
    When the parent controller polls for scripts
    Then the script is executed
    And the script is moved from "in_progress" to the "actions/failed" directory
    And a result file is created in the "queue/failed" directory with the script's exit code and stderr
