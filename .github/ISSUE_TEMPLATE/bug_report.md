---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug
assignees: ''

---

<!--

Thank you for using cx_Oracle.

See https://www.oracle.com/corporate/security-practices/assurance/vulnerability/reporting.html for how to report security issues

The cx_Oracle driver was renamed to python-oracledb in May 2022.  It has a new
repository at https://github.com/oracle/python-oracledb.  The installation
instructions are at:
https://python-oracledb.readthedocs.io/en/latest/user_guide/installation.html

Update to python-oracledb, if possible, and submit your bug report to the
python-oracledb repository.

No further releases under the cx_Oracle namespace are planned.

Otherwise, please answer these questions so we can help you.

Use Markdown syntax, see https://docs.github.com/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax

-->

1. What versions are you using?

<!--

Give your database version.


Also run Python and show the output of:

    import sys
    import platform

    print("platform.platform:", platform.platform())
    print("sys.maxsize > 2**32:", sys.maxsize > 2**32)
    print("platform.python_version:", platform.python_version())

And:

    import cx_Oracle
    print("cx_Oracle.version:", cx_Oracle.version)
    print("cx_Oracle.clientversion:", cx_Oracle.clientversion())

-->

2. Is it an error or a hang or a crash?

3. What error(s) or behavior you are seeing?

<!--

Cut and paste text showing the command you ran.  No screenshots.

Use a gist for long screen output and logs: see https://gist.github.com/

-->

4. Include a runnable Python script that shows the problem.

<!--

Include all SQL needed to create the database schema.

Format code by using three backticks on a line before and after code snippets, for example:

```
import cx_Oracle
```

-->
