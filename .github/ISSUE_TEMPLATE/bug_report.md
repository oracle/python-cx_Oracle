---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug
assignees: ''

---

**See https://www.oracle.com/corporate/security-practices/assurance/vulnerability/reporting.html for how to report security issues**.

For other issues:

1. Is it an error or a hang or a crash?

2. What error(s) you are seeing?
**Cut and paste text showing the command you ran.  No screenshots.  Use a gist for long screen output and logs: see https://gist.github.com/**.

3. Include a runnable Python script that shows the problem.
Include all SQL needed to create the database schema.

4. Show the output of:

```
import sys
import platform

print("platform.platform:", platform.platform())
print("sys.maxsize > 2**32:", sys.maxsize > 2**32)
print("platform.python_version:", platform.python_version())
```

And:

```
import cx_Oracle
print("cx_Oracle.version:", cx_Oracle.version)
print("cx_Oracle.clientversion:", cx_Oracle.clientversion())
```

5. What is your Oracle Database version?
