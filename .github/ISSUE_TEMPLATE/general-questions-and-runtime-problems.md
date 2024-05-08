---
name: Questions and Runtime Problems
about: For general cx_Oracle questions
title: ''
labels: question
assignees: ''

---

<!--

Thank you for using cx_Oracle.

The cx_Oracle driver was renamed to python-oracledb in May 2022.  It has a new
repository at https://github.com/oracle/python-oracledb.  The installation
instructions are at:
https://python-oracledb.readthedocs.io/en/latest/user_guide/installation.html

Update to python-oracledb, if possible.

Otherwise, review the cx_Oracle user manual: https://cx-oracle.readthedocs.io/en/latest/index.html

Please answer these questions so we can help you.

Use Markdown syntax, see https://docs.github.com/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax

GitHub issues that are not updated for a month may be automatically closed.  Feel free to update them at any time.

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

2. Describe the problem

<!-- Cut and paste text showing the command you ran.  No screenshots. -->

3. Include a runnable Python script that shows the problem.

<!--

Include all SQL needed to create the database schema.

Use a gist for long code: see https://gist.github.com/

Format code by using three backticks on a line before and after code snippets, for example:

```
import cx_Oracle
```

-->
