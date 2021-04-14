---
name: Installation Problems
about: Use this for cx_Oracle installation questions
title: ''
labels: install & configuration
assignees: ''

---

<!--

Thank you for using cx_Oracle.

Do these before creating a new issue:

    Review and follow the Installation Instructions: https://cx-oracle.readthedocs.io/en/latest/user_guide/installation.html

    Review the troubleshooting tips: https://cx-oracle.readthedocs.io/en/latest/user_guide/installation.html#troubleshooting

    Review the user manual: https://cx-oracle.readthedocs.io/en/latest/index.html

If you have a `DPI-1047`, `DPI-1050` or `DPI-1072` error, re-review the links above.

Google any errors.

Then please answer these questions so we can help you.

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

-->

2. Describe the problem

<!-- Cut and paste text showing the command you ran.  No screenshots. -->

3. Show the directory listing where your Oracle Client libraries are installed (e.g. the Instant Client directory).  Is it 64-bit or 32-bit?

4. Show what the `PATH` environment variable (on Windows) or `LD_LIBRARY_PATH` (on Linux) is set to?

5. Show any Oracle environment variables set (e.g. ORACLE_HOME, ORACLE_BASE).
