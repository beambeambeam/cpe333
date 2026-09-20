# CPE333 Mini-Project 01: Ubuntu Kernel Build

This directory contains the assignment statement, the Ubuntu kernel build guide, and the reports produced from the group's ARM64 virtual-machine experiments.

## Assignment references

- [Problem statement](problem-statements.md)
- [Ubuntu kernel build guide](how-to-build-an-ubuntu-linux-kernel.md)

## Report index

### Beam

Beam's report documents a separate Ubuntu 24.04.5 LTS ARM64 experiment that built the full `binary` target and booted `6.8.0-999-generic`.

1. [ARM64 VM setup](beam/01-ubuntu-kernel-vm-setup.md)
2. [Ubuntu Server installation](beam/02-ubuntu-server-installation.md)
3. [Kernel build preparation](beam/03-kernel-build-preparation.md)
4. [Kernel build and generated packages](beam/04-kernel-build-and-packages.md)
5. [Kernel installation and verification](beam/05-kernel-installation-and-verification.md)

### Pluem

Pluem's report documents an Ubuntu 26.04.1 LTS ARM64 experiment that built the `binary-generic` target and booted `7.0.0-999-generic`.

- [Complete custom-kernel build and installation report](pluem/01-custom-kernel-build-and-installation.md)
- [Pluem report and evidence index](pluem/README.md)

## Environment differences

The two report sets describe independent experiments. Their Ubuntu releases, source-package versions, kernel versions, storage layouts, build targets, package dependencies, and troubleshooting steps are therefore not expected to be identical. Each report preserves the commands and results from its own experiment rather than rewriting them to appear the same.

## Evidence policy

Screenshots and terminal logs are included only when they came from the actual experiments. Missing captures are identified explicitly. Large generated Debian packages and oversized raw build logs are not committed; reports provide package names, checksums, concise verification records, and the retained local evidence locations instead.
