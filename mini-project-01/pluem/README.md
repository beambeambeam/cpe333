# Pluem: Mini-Project 01

This directory contains the report and evidence for the Ubuntu 26.04.1 LTS ARM64 custom-kernel experiment completed in UTM on Apple Silicon.

## Report

- [Building and installing a custom Ubuntu Linux kernel](01-custom-kernel-build-and-installation.md)

The report follows the experiment chronologically from repository setup and source preparation through ABI modification, the `binary-generic` build, disk expansion, package validation, the lab-specific modules-package adjustment, installation, reboot, and final verification.

## Evidence

- [`evidence/screenshots/`](evidence/screenshots/) contains ten chronological screenshots and their original-name mapping.
- [`evidence/logs/`](evidence/logs/) contains reviewer-readable copies of the terminal evidence.
- The full build log and generated `.deb` packages remain in the original experiment workspace because they are large; their locations and checksums are recorded in the report.

## Final verified result

```text
uname -r
7.0.0-999-generic
```
