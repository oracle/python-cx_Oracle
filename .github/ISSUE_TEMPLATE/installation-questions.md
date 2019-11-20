---
name: Installation Questions
about: Use this for cx_Oracle installation questions
title: ''
labels: install & configuration
assignees: ''

---

# Do these first

- Review and follow the [installation instructions](https://cx-oracle.readthedocs.io/en/latest/user_guide/installation.html)

- Review the [troubleshooting tips](https://cx-oracle.readthedocs.io/en/latest/user_guide/installation.html#troubleshooting)

- If you have a `DPI-1047`, `DPI-1050` or `DPI-1072` error, review both the above again before opening an issue.

- **Google any errors**

# Answer the following questions

1. Describe the problem and show the error you have.
**Cut and paste text showing the command you ran.  No screenshots.  Use a gist for long screen output and logs: see https://gist.github.com/**.

2. Show the output of:

```
import sys
import platform

print("platform.platform:", platform.platform())
print("sys.maxsize > 2**32:", sys.maxsize > 2**32)
print("platform.python_version:", platform.python_version())
```

3. Show the directory listing where your Oracle client libraries are installed (e.g. the Instant Client directory).  Is it 64-bit or 32-bit?

4. Show what the `PATH` environment variable (on Windows) or `LD_LIBRARY_PATH` (on Linux) is set to?  On macOS, show what is in `~/lib`.

5. Show any Oracle environment variables set (e.g. ORACLE_HOME, ORACLE_BASE).
