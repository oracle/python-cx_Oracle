---
name: General Questions and Runtime Problems
about: For general cx_Oracle questions
title: ''
labels: question
assignees: ''

---

1. Review the [cx_Oracle Documentation](https://cx-oracle.readthedocs.io/en/latest/index.html).  Review your output and logs.  **Google any errors**

2. Describe the problem
**Cut and paste text showing the command you ran.  No screenshots.  Use a gist for long screen output and logs: see https://gist.github.com/**.

3. Include a runnable Python script that shows the problem.  Include all SQL needed to create the database schema.  Use Markdown syntax, see https://help.github.com/github/writing-on-github/basic-writing-and-formatting-syntax

   The more details you give, the more we can help.

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
