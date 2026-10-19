# Separate settings and logs for automated exports

Set `SEAMLY_DATA_DIR` to an absolute, writable directory before starting
Seamly2D. The program stores its INI settings under `settings/` and logs under
`logs/seamly2d/` within that directory. This keeps batch export jobs independent
of an operator's interactive settings and log files.

For example, in PowerShell:

```powershell
$env:SEAMLY_DATA_DIR = 'E:/seamly-jobs/job-001/state'
seamly2d --basename skirt-50 --destination E:/seamly-jobs/job-001/output `
    --format 20 --exportOnlyDetails --gsize 50 --gheight 176 skirt.sm2d
```

An invalid directory stops the program with exit status 73 before it creates
the application or opens the user's settings. The existing locations remain
the default when the variable is unset. This option applies to Seamly2D;
SeamlyMe does not use it. It does not redirect pattern files, export files, or
the operating system's temporary directory. Set `TEMP` and `TMP` separately
when an automated job also needs a private temporary directory.
