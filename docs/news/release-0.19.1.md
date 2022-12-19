# Version 0.19.1 Release

This version contains changes in how the test profiles are named
since the original name was misleading. Specifically, it was
unclear when talking about the profile test files if the user
wanted to refer about the conan profile instead. To avoid this
confusion we decided to rename the files to a more understandable
name.
This release also contains a fix to detect when the user wants
to execute the docker tests in a non-tty service like Jenkins. If
a non tty execution is detected then the tests will be triggered
without allocating a pseudo-TTY when using the docker run command.

For the entire changelog, see the Git commit history.
