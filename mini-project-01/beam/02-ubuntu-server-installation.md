---
title: Ubuntu Server installation in UTM
status: installation-complete
research_date: 2026-09-13
tags:
  - mini-project-01
  - UTM
  - Ubuntu-24.04.4
  - ARM64
  - Ubuntu-Server
  - installation
  - research
  - research-2026-09-13
  - evidence
---

# Journey: installing Ubuntu Server in UTM

## Goal

The goal was to prepare an isolated ARM64 Ubuntu environment for `mini-project-01`. The project requires compiling and installing a new Ubuntu kernel, so the work is being done inside a UTM virtual machine instead of directly on the Mac.

Project references:

- Requirements: `../problem-statements.md`
- Kernel build procedure: `../how-to-build-an-ubuntu-linux-kernel.md`

## 1. Choosing the Ubuntu image

The host machine uses ARM64. We therefore chose **Ubuntu Server 24.04.4 LTS ARM64**, also called `aarch64` or `ARMv8`. Ubuntu Server is sufficient because the kernel build uses the terminal and does not require a graphical desktop.

ISO used:

```text
ubuntu-24.04.4-live-server-arm64.iso
```

Download sources:

- Direct ISO: https://download.cdimage.ubuntu.com/ubuntu/releases/24.04/release/ubuntu-24.04.4-live-server-arm64.iso
- Official ARM page: https://ubuntu.com/download/server/arm

## 2. Creating the UTM virtual machine

In UTM we selected **Virtualize → Linux**, selected the ARM64 ISO, and left hardware OpenGL disabled. QEMU was used as the virtualization engine.

![UTM display output configuration](media/01-utm-display-output.png)

![UTM Linux boot configuration with the ARM64 ISO](media/02-utm-linux-iso.png)

The virtual machine was configured with 8 GB of memory and 4 CPU cores. Display output was enabled so the Ubuntu installer could be used in the VM window. Hardware OpenGL was left disabled because it is not needed for Ubuntu Server.

![UTM hardware configuration](media/03-utm-hardware.png)

A shared directory was not needed. The kernel source and build files will stay inside the Ubuntu virtual disk.

![UTM shared directory left empty](media/04-utm-shared-directory.png)

The final UTM summary showed an ARM64 machine with a 64 GB virtual disk.

![UTM summary](media/05-utm-summary-top.png)

![UTM summary details](media/06-utm-summary-details.png)

Final VM settings:

| Setting | Value |
| --- | --- |
| Architecture | ARM64 (aarch64) |
| Engine | QEMU |
| Memory | 8 GB |
| CPU | 4 cores |
| Virtual disk | 64 GB |
| Hardware OpenGL | Disabled |
| Shared directory | Not configured |

## 3. Starting the Ubuntu installer

The VM booted successfully from the ISO. We selected **Try or Install Ubuntu Server** from the GRUB menu.

![Ubuntu GRUB boot menu](media/07-ubuntu-grub.png)

The installer language was set to English.

![Ubuntu installer language selection](media/08-ubuntu-language.png)

The installer offered a newer installer version. We selected the option to update the installer before continuing. This updated the installer only; it did not change the Ubuntu release selected for the VM.

![Ubuntu installer update prompt](media/09-ubuntu-installer-update.png)

The keyboard layout was left at English (US).

![Ubuntu keyboard configuration](media/10-ubuntu-keyboard.png)

## 4. Configuring the installation

We selected the normal **Ubuntu Server** installation rather than the minimized installation. Third-party drivers were not selected.

![Ubuntu installation type](media/11-ubuntu-install-type.png)

UTM supplied a network interface through DHCP. The installer received the address `192.168.64.9` on `enp0s1`, confirming that networking was available.

![Ubuntu network configuration](media/12-ubuntu-network.png)

No proxy was required, so the proxy field was left blank.

![Ubuntu proxy configuration](media/13-ubuntu-proxy.png)

The installer tested and accepted the ARM64 Ubuntu mirror:

```text
http://ports.ubuntu.com/ubuntu-ports
```

![Ubuntu ARM64 archive mirror](media/14-ubuntu-mirror.png)

## 5. Preparing enough disk space

The VM has a 64 GB virtual disk. Ubuntu's default LVM layout initially assigned about 30 GB to `/` and left the rest free inside the volume group. Since kernel compilation needs considerable free space, we edited `ubuntu-lv` and increased it to 60 GB.

![Ubuntu guided storage configuration](media/15-ubuntu-guided-storage.png)

The logical volume remained formatted as ext4 and mounted at `/`.

![Expanded Ubuntu logical volume](media/16-ubuntu-lvm-edit.png)

The installer then displayed the formatting confirmation. This operation affected only the UTM virtual disk.

![Storage formatting confirmation](media/17-ubuntu-storage-confirmation.png)

## 6. Creating the Ubuntu account

We created the local user `beam` and named the server `ubuntu-live-server-lab`. The password was entered twice and was not recorded in this document.

![Ubuntu user profile configuration](media/18-ubuntu-profile.png)

Ubuntu Pro was skipped because it is not required for the project.

![Ubuntu Pro skipped](media/19-ubuntu-pro.png)

OpenSSH was also skipped. It can be installed later if remote access is needed.

![OpenSSH configuration](media/20-ubuntu-ssh.png)

No featured server snaps were installed. They are unrelated to the kernel build.

![Featured server snaps](media/21-ubuntu-snaps.png)

## 7. Installing Ubuntu

The installer formatted the virtual disk, copied the system files, configured the bootloader, and installed the default Ubuntu kernel.

![Ubuntu system installation in progress](media/22-ubuntu-installing.png)

The installation completed successfully.

![Ubuntu installation complete](media/23-ubuntu-install-complete.png)

During reboot, UTM could not automatically unmount the installer CD-ROM. This was not a failed Ubuntu installation. We ejected the ISO from UTM and pressed Enter.

![Installer media unmount message](media/24-ubuntu-eject-issue.png)

## 8. First successful login

After the ISO was removed, the VM booted from its virtual disk and accepted the `beam` login.

![First login to Ubuntu Server](media/25-ubuntu-login.png)

The initial VM state was:

```text
Ubuntu 24.04.4 LTS
Kernel: 6.8.0-139-generic
Architecture: aarch64
Hostname: ubuntu-live-server-lab
Root filesystem: 58.76 GB
Network address: 192.168.64.9
```

The running kernel is still the stock Ubuntu kernel. This confirms that the VM is ready for the custom-kernel phase.

## 9. Next chapter: build the custom kernel

Before changing the kernel, create a UTM snapshot or duplicate the working VM.

Run the following commands **inside Ubuntu**:

```bash
sudo apt update
sudo apt full-upgrade -y
sudo reboot
```

After logging in again, record the environment:

```bash
uname -m
uname -r
cat /etc/os-release
df -h /
nproc
```

Then continue with `../how-to-build-an-ubuntu-linux-kernel.md`:

1. Enable `deb-src` in `ubuntu.sources`.
2. Install the kernel build dependencies.
3. Download the source matching the running kernel.
4. Change the ABI number to `999`.
5. Build the Debian kernel packages.
6. Install the generated packages.
7. Reboot and verify the new kernel with `uname -r`.
