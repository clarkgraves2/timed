# 170D Mod-T C Networking 1B

# Daytime Server

Create daytime protocol server. Use RFC 867 as a reference document, with an extension allowing custom formats.

The server should be designed to support both TCP and UDP connections. Clients have the option to send a string defining the time format they are requesting (matching `strftime()` requirements). If client payload is empty, a default format should be returned.

Server should support packet sizes up to the standard MTU.

## DIcE Rubric

| Area  | Requirement  | Description  | Point Value  | Points Earned  | Justification  |
|-------|--------------|--------------|--------------|----------------|----------------|
| Documentation | User Guide | `README.md` should function as a quick user guide. It should include at minimum: <ul><li>how to build the project</li><li>how to run the project</li><li>how to interact with the server</li></ul> | 5 |  |  |
| Documentation | Design Plan | `design.md` is comprehensive, clear, and a *draft* commited/pushed before any code. It includes:<ul><li>Overview of the project</li><li>Key considerations</li><li>General flow</li><li>Priorities of work with milestones</li><li>Projected challenges</li></ul> | 5 |  |  |
| Documentation | Writeup | `writeup.md` is reflective and well-structured. It includes:<ul><li>Discussion of at least three challenges encountered</li><li>Analysis of successful strategies and implementation during project</li><li>Reflection of design plan, in regards to what should be emphasized more or less</li><li>Lessons learned from both challenges and successes</li></ul> | 5 |  |  |
| Documentation | Formatting | Source code follows course format requirements | 2 |  |  |
| Documentation | Formatting | Comments added where appropriate and aid in the understanding of logic | 1 |  |  |
| Implementation | Version Control | Project named `timed` with a default branch of `main` and all student developed files merge requested to `main` | 1 |  |  |
| Implementation | Version Control | Commits broken down into appropriate scopes | 2 |  |  |
| Implementation | Version Control | Commit messages simple and informative | 2 |  |  |
| Implementation | Architecture | Effective and efficient data structures and algorithms used | 5 |  |  |
| Implementation | Architecture | Code designed and constructed in a modular fashion | 5 |  |  |
| Implementation | Architecture | Generally sound decisions made with regards to architecture | 5 |  |  |
| Implementation | Testing | Public functions include robust unit tests | 3 |  |  |
| Implementation | Testing | All test code located in TLD `./test/` | 1 |  |  |
| Implementation | Testing | All automated tests pass under `make check` | 1 |  |  |
| Execution | Safety | Project avoids crashing or infinite loops, even on invalid input | 5 |  |  |
| Execution | Safety | `valgrind` reports no errors or warnings | 5 |  |  |
| Execution | Safety | Static analysis tools report no high-severity warnings related to security, undefined behavior, memory safety, or type safety. Minor warnings should be corrected where reasonable | 5 |  |  |
| Execution | Builds | Project compiles with no warnings using required flags | 2 |  |  |
| Execution | Builds | Project builds all build targets correctly | 2 |  |  |
| Execution | Performance | Project scales appropriately with input and data | 5 |  |  |
| Execution | Performance | Project executes in a timely manner | 1 |  |  |
| Execution | Requirements | All other requirements met | 22 |  |  |
| Execution | Discretionary |  | 10 |  |  |
| **Total** |  |  | 100 |  |  |

## Requirements

| Area  | Description  | Point Value  | Points Earned  | Justification  |
|-------|--------------|--------------|----------------|----------------|
| Critical Task | Outside code cited appropriately. |  |  |  |
| Critical Task | No third-party headers/libraries used without Program Manager or Senior Instructor approval |  |  |  |
| --- | --- | --- | --- | --- |
| Documentation | All documentation in `.md` format | inc |  |  |
| Documentation | All documentation located in `./doc/` at TLD of project | inc |  |  |
| Documentation | Project free of grammatical and spelling errors? | inc |  |  |
| Documentation | Non-code formatting consistent? | inc |  |  |
| --- | --- | --- | --- | --- |
| Implement | Server responds on correct ports for TCP and UDP | 11 |  |  |
| Implement | Server conforms to format requested | 11 |  |  |
| --- | --- | --- | --- | --- |
| Execute | Project must build and run on Ubuntu 22.04 LTS with Check 0.15 | inc |  |  |
| Execute | Project must build `timed` in TLD in response to `make` | inc |  |  |
| Execute | Project must build `timed` in TLD w/ debug symbols in response to `make debug` | inc |  |  |
| Execute | Project must build `timed` in TLD w/ profile symbols in response to `make profile` | inc |  |  |
| Execute | Project must build *and* run automated unittests in response to `make check` | inc |  |  |
| Execute | Project must clean *all* project-generated files in response to `make clean` | inc |  |  |
| Execute | Project must build against (`std=c99` with `-D_POSIX_C_SOURCE=200809L`) | inc |  |  |
| Execute | Project must build with `-Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal` | inc |  |  |
| Execute | Error suppression techniques (`NOLINT`, `type-casting`, `# type: ignore`, etc) shall be specific to the error they address and accompanied by a comment justifying their use.<br>Exception: `fprintf()` return code can be resolved without justification | inc |  |  |


# Feedback Comments