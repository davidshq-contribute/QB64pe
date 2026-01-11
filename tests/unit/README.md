# QB64-PE Unit Tests

This directory contains the unit test suite for QB64 Phoenix Edition compiler components.

## Quick Start

### Running Tests (Recommended Method)

**⚠️ Windows Users: Important Note**

On Windows, QB64 console applications open a **separate window** that may show **error dialogs requiring user interaction**. This is a Windows OS limitation with QB64.

**For automated/headless testing on Windows, use WSL:**
```bash
wsl bash tests/unit/run_tests_wsl.sh
```

See [WSL_SETUP.md](WSL_SETUP.md) for one-time setup instructions.

**Linux/Mac:**
```bash
./tests/unit/run_tests.sh
```

**Windows (with user interaction):**
```bash
./tests/unit/run_tests.sh    # Git Bash
tests\unit\run_tests.bat     # Command Prompt
```

These wrapper scripts will:
1. Compile the test runner if needed
2. Execute all unit tests
3. Display results in your current console
4. Return appropriate exit codes for CI/CD integration

### Running Tests (Direct Method)

If you prefer to run the test executable directly:

1. **Compile the test runner:**
   ```bash
   ./qb64pe -x tests/unit/test_runner.bas
   ```

2. **Run the tests:**
   ```bash
   ./test_runner.exe
   ```

3. **View results:**
   ```bash
   cat test_results.txt
   ```

**Note:** On Windows, running `test_runner.exe` directly will open a separate console window that closes immediately. The wrapper scripts handle this by reading `test_results.txt` and displaying it in your current console.

## Test Organization

The test suite is organized by component:

- **`test_runner.bas`** - Main test runner (compiles all test suites)
- **`build_utilities/`** - Build system integration tests
- **`code_generation/`** - C++ code emission tests
- **`const_eval/`** - Constant evaluation tests
- **`error_handling/`** - Error reporting tests
- **`file_utilities/`** - File I/O utility tests
- **`format/`** - Code formatting tests
- **`include_provider/`** - Include file system tests
- **`parser/`** - Parser component tests
- **`statevars/`** - Feature activation & state management tests
- **`string_utilities/`** - String manipulation tests
- **`symbol_table/`** - Hash table & symbol lookup tests
- **`type_system/`** - Type system tests

## Test Framework

The test infrastructure uses a custom framework defined in:

- **`test_framework.bi`** - Core test framework (assertions, test lifecycle)
- **`test_framework_declarations.bi`** - Framework type definitions
- **`test_framework_implementations.bas`** - Framework implementation
- **`test_state_manager.bi`** - Test state isolation system
- **`test_compiler_context.bi`** - Compiler state management
- **`test_component_utils.bi`** - Component testing utilities

### Writing Tests

Each test suite follows this structure:

```qbasic
SUB Test_YourFeature
    Test_Start "Description of what you're testing"

    DIM context AS TestStateContext
    TestState_Init context, "all"  ' Initialize test state

    DIM result AS LONG

    ' Your test code here
    result = Test_Assert&(condition, "Assertion message")

    ' Or use specific assertion functions
    result = Test_AssertEqual&(expected, actual, "Values should match")
    result = Test_AssertEqualString&("expected", actual$, "Strings should match")

    TestState_Cleanup context  ' Clean up test state
    Test_End result
END SUB

SUB RunYourTestSuite
    Test_YourFeature
    ' Add more test functions here
END SUB
```

## Test State Management

The test infrastructure provides state isolation to prevent tests from interfering with each other:

- **`TestState_Init`** - Initializes isolated compiler state
- **`TestState_Cleanup`** - Cleans up and resets state after test
- **State levels:** `"minimal"`, `"parser"`, `"all"` (choose based on what you're testing)

## Exit Codes

The wrapper scripts return appropriate exit codes for CI/CD integration:

- **0** - All tests passed
- **1** - One or more tests failed

## Continuous Integration

For CI/CD pipelines, use the wrapper scripts:

```yaml
# Example GitHub Actions workflow
- name: Run unit tests
  run: ./tests/unit/run_tests.sh
```

The scripts will:
- Automatically compile tests if needed
- Run all tests
- Display results
- Exit with appropriate status code

## Test Output

Tests write results to two locations:

1. **Console output** - Summary displayed by wrapper scripts
2. **`test_results.txt`** - Detailed results written to root directory

### Example Output

```
=== Test Summary ===
Total tests: 73
Passed: 73
Failed: 0
Skipped: 0

Total assertions: 73
Passed: 73
Failed: 0

ALL TESTS PASSED
```

## Known Issues

### Windows Console Window

On Windows, QB64 console applications open in a separate console window. This is a Windows OS behavior, not a QB64 issue. The wrapper scripts work around this by:

1. Running `test_runner.exe` in a new window (which closes automatically)
2. Reading the `test_results.txt` file
3. Displaying results in your current console

The `$CONSOLE:ONLY` directive is already present in `test_runner.bas` to ensure console-only mode.

## Adding New Tests

1. Create a new test file in the appropriate directory (e.g., `my_component/test_my_feature.bas`)
2. Include the test framework: `'$INCLUDE:'../test_framework.bi'`
3. Write your test functions following the pattern above
4. Add your test suite to `test_runner.bas`:
   ```qbasic
   '$INCLUDE:'my_component/test_my_feature.bas'

   ' In RunAllTests SUB:
   RunMyFeatureTests
   ```
5. Recompile and run tests

## Debugging Test Failures

When a test fails, the output shows:

```
=== Test Summary ===
Total tests: 10
Passed: 9
Failed: 1

Test: My Feature Test - FAILED
  Assertion failed: Expected value to be 5 but got 3
```

The detailed output section (if enabled) shows which specific assertion failed and why.

## Test Coverage

Current test coverage:

- ✅ Type system (UDT management, type validation)
- ✅ Symbol table (hash operations, lookup)
- ✅ Parser utilities (element extraction, string parsing)
- ✅ Code generation (C++ emission, structure)
- ✅ File utilities (file operations, path handling)
- ✅ String utilities (string manipulation)
- ✅ Include provider (file system abstraction)
- ✅ Error handling (error reporting, validation)
- ✅ State variables (feature activation, recompilation)
- ✅ Build utilities (build system integration)
- ✅ Format utilities (code formatting)

**Total:** 73 tests, 73 assertions, 100% passing

## Resources

- **Main documentation:** `../../docs/testing/testing.md`
- **Architecture:** `../../docs/ARCHITECTURE.md`
- **Build system:** `../../docs/build-system.md`
- **Refactoring log:** `../../docs/REFACTORING_LOG.md`

## Contributing

When adding new tests:

1. Follow the existing test structure
2. Use descriptive test names
3. Include assertion messages that clearly explain what failed
4. Clean up test state properly
5. Run the full test suite before committing

## Questions?

See the documentation files mentioned above or check `../../CLAUDE.md` for project overview and development patterns.
