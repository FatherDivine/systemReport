# systemReport

Reports simple system configuration. Manually hand-coded in c99 on a headless centOS 6.7 server while updating to Rocky 9.x
Compiling (in centOS):<br>

```bash
gcc -Wall -g -std=c99 systemReport.c -o systemReport
```
<br>

# CentOS C/GCC Compatibility Reference

A quick reference guide for the differences in C standards and GCC compiler versions across legacy CentOS releases.
<<br><br>

## Compiler & Standard Defaults

| CentOS Version | Base GCC Version | Default C Standard | Key C Features / Notes |
| :--- | :--- | :--- | :--- |
| **6.7 / 6.8** | **4.4.7** | `gnu89` (C89 + ext) | Lacks full C99 (e.g., no mid-block declarations). No C11. |
| **7 (Latest)** | **4.8.5** | `gnu90` (C90 + ext) | Full C99 support; preliminary C11. Improved C++11 support. |
| **8 (Last)** | **8.5.0** | `gnu11` (C11 + ext) | Full C11 support. Includes `-fstack-clash-protection` by default. |
<br><br>

## Key Coding Differences

### 1. C Standards Baseline
*   **CentOS 6:** Often requires the `-std=c99` flag for modern basics like `for (int i = 0; ...)`.
*   **CentOS 8:** C11 is the baseline. You get `<stdatomic.h>` and `_Generic` expressions without extra flags.

### 2. Modernizing with Toolsets
Because base compilers in CentOS 6/7 are aged, developers typically use these methods to get newer GCC versions (like GCC 9, 10, or 11):
*   **CentOS 6 & 7:** Use **Software Collections (SCL)** to install "Developer Toolsets" (e.g., `yum install devtoolset-9`). This requires the `scl enable` command to activate.
*   **CentOS 8:** Uses **AppStreams**, allowing you to switch versions easily via `dnf install gcc-toolset-11` without the complex SCL syntax.

### 3. Library Compatibility (glibc)
*   **Forward Compatibility:** Code compiled on CentOS 6 will generally run on 7 or 8.
*   **Backward Compatibility:** Programs compiled on CentOS 8 will **not** run on CentOS 7 or 6 because they require a newer version of `glibc` than those systems provide.

## Security & Optimization
*   **CentOS 8 (GCC 8+):** Includes AVX-512 support for modern CPUs and superior static analysis for catching bugs at compile-time.
*   **Status:** CentOS 6 and 7 are **End-of-Life (EOL)** and no longer receive security patches for their compilers or system libraries.
