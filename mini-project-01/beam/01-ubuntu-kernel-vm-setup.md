---
title: Ubuntu kernel mini-project – ARM64 VM setup
status: in-progress
research_date: 2026-09-10
tags:
  - mini-project-01
  - research
  - research-2026-09-10
  - UTM
  - Ubuntu
  - Ubuntu-24.04
  - ARM64
  - Linux-kernel
  - handoff
---

# Handoff: Ubuntu kernel mini-project

## Session focus

Start `mini-project-01` by creating an Ubuntu ARM64 virtual machine in UTM. The user has installed UTM and confirmed that the host uses ARM64.

## Assignment references

- Requirements: `mini-project-01/problem-statements.md`
- Kernel build procedure: `mini-project-01/how-to-build-an-ubuntu-linux-kernel.md`

The assignment requires compiling and installing an Ubuntu kernel, then submitting a step-by-step report. A VM is preferred so the original host/kernel remains safe.

## Decisions and research

- Use **Ubuntu Server 24.04 LTS ARM64** (`aarch64` / `ARMv8`). A graphical desktop is not required for the kernel build.
- ISO researched on 2026-09-10:
  - Direct ISO: https://download.cdimage.ubuntu.com/ubuntu/releases/24.04/release/ubuntu-24.04.4-live-server-arm64.iso
  - Official ARM page: https://ubuntu.com/download/server/arm
  - UTM Ubuntu guide: https://docs.getutm.app/guides/ubuntu/
  - Current Canonical kernel guide: https://documentation.ubuntu.com/kteam-docs/public/how-to/build-kernel.html
- In UTM select **Virtualize → Linux**, not **Emulate**.
- Suggested VM resources: 8 GB RAM, 4 CPU cores, and at least 50 GB disk. Leave enough RAM for macOS.

## Current state

Completed:

- UTM installed.
- ARM64 host confirmed by the user.
- Correct Ubuntu ARM64 ISO identified.
- UTM creation settings explained.

Not completed:

- ISO download.
- Ubuntu VM creation and installation.
- Kernel source download/build.
- Kernel package installation and reboot test.
- Report evidence collection.

## Next session

1. Download the ARM64 ISO above.
2. Create the UTM VM with the settings above.
3. Install Ubuntu Server. The commands below must run inside Ubuntu, not macOS.
4. Confirm the VM with `uname -m`, `uname -r`, `lsb_release -a`, and `df -h`.
5. Follow `mini-project-01/how-to-build-an-ubuntu-linux-kernel.md`:
   - enable `deb-src` in `ubuntu.sources`;
   - install build dependencies;
   - download the source matching the running kernel;
   - change the ABI number to `999`;
   - build the Debian packages;
   - install them and reboot;
   - verify the new version with `uname -r`.
6. Record screenshots/output for the report: VM settings, Ubuntu version, original kernel, changelog ABI, generated `.deb` files, install result, and final kernel version.
7. Preserve the original kernel and create a VM snapshot/copy before installing the custom packages.

## Suggested skills

- `handoff` – create the next session handoff after the VM/build work.
- `writing-for-agents` – keep future notes structured and easy for another agent to follow.

## Handoff completion

The session stopped before VM creation. The next agent should begin with the ISO download and UTM setup, then continue using the repository kernel-build guide.
