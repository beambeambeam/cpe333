---
title: Building and installing a custom Ubuntu Linux kernel
status: completed
research_date: 2026-09-17
tags:
  - mini-project-01
  - Ubuntu-26.04.1
  - ARM64
  - Linux-kernel
  - kernel-build
  - kernel-installation
  - kernel-verification
  - evidence
---

# CPE333 Mini-Project 01: Building and Installing a Custom Ubuntu Linux Kernel

## 1. Objective

The objective of this mini-project was to compile and install a custom Ubuntu Linux kernel on an ARM64 virtual machine. The custom kernel used ABI `999` so that it could coexist with the original Ubuntu kernel, `7.0.0-31-generic`, and the original kernel could remain available as a fallback.

This report follows the procedure in [`problem-statements.md`](../problem-statements.md) and [`how-to-build-an-ubuntu-linux-kernel.md`](../how-to-build-an-ubuntu-linux-kernel.md). The commands, versions, package names, and results below are taken from the actual experiment and its evidence files.

| Result | Verified value |
| --- | --- |
| Source package | `linux 7.0.0-31.31` |
| Original running kernel | `7.0.0-31-generic` |
| Custom ABI and flavour | `7.0.0-999-generic` |
| Build target | `binary-generic` |
| Architecture | `arm64` / `aarch64` |
| Build result | Exit code `0` |
| Installed custom packages | Image and lab-adjusted modules package |
| Final boot proof | `uname -r` returned `7.0.0-999-generic` |

## 2. Repository and report context

The experiment and the original report were prepared from the fork `Palapluem/cpe333`, with `beambeambeam/cpe333` configured as the upstream repository. The working branch was:

```text
pluem/mini-project-01
```

The branch started from upstream commit `fbaa29e` (`docs: add PS05 virtual memory lab`). No source-code changes were made to the Ubuntu kernel source in this repository. This file is the Mini-Project 01 report for the `pluem` section.

## 3. Build environment

The experiment was performed in an Ubuntu virtual machine running in UTM on an Apple Silicon Mac with an M5 processor.

| Item | Value |
| --- | --- |
| Operating system | Ubuntu 26.04.1 LTS (Resolute Raccoon) |
| Architecture | `arm64` / `aarch64` |
| Virtualisation | UTM on Apple Silicon Mac (M5) |
| Virtual CPUs | 4 |
| Memory | Approximately 7.2 GiB |
| Swap | 4 GiB |
| Kernel before the experiment | `7.0.0-31-generic` |
| Initial virtual disk | Approximately 50 GB |
| Final virtual disk | 64 GB virtual disk; approximately 62 GB available to `/` after resizing |

The initial build environment evidence showed approximately 29–31 GB free on `/` after installing the dependencies. The source tree itself occupied approximately 1.7 GB before compilation.

> **Evidence note:** The separate build-environment screenshot named `02_build_environment_ready.png` was not present in the supplied ZIP. The original terminal snapshot is included as [`05-build-environment.txt`](evidence/logs/05-build-environment.txt); no replacement screenshot was fabricated.

## 4. Enabling source repositories and installing dependencies

The Ubuntu source repositories were enabled by adding `deb-src` entries to `/etc/apt/sources.list.d/ubuntu.sources`. Before changing the file, a backup was saved at:

```text
/root/cpe333-backups/ubuntu.sources.before-cpe333
```

The enabled repositories covered the `resolute`, `resolute-updates`, `resolute-backports`, and `resolute-security` suites. The available Linux source package included version `7.0.0-31.31`.

The build dependencies were installed according to the repository guide:

```bash
sudo apt update
sudo apt build-dep -y linux linux-image-unsigned-$(uname -r)
sudo apt install -y fakeroot llvm libncurses-dev dwarves
```

The dependency installation also provided the required ARM64 build toolchain. The installed tools included GCC `15.2.0`, GNU Make `4.4.1`, LLVM `21.1.8`, and `dwarves` `1.31`.

**Figure 1. Source repositories enabled.**

![Figure 1. Source repositories enabled.](evidence/screenshots/01-source-repositories-enabled.png)

Evidence files:

- [`01-source-repositories.txt`](evidence/logs/01-source-repositories.txt)
- [`02-build-dep-dry-run.txt`](evidence/logs/02-build-dep-dry-run.txt)
- [`03-build-dependencies.log`](evidence/logs/03-build-dependencies.log)
- [`04-extra-build-tools.log`](evidence/logs/04-extra-build-tools.log)

These files are exact copies of the evidence captured in the experiment workspace.

## 5. Obtaining and preparing the kernel source

The source package corresponding to the running kernel was downloaded into the experiment workspace. The resulting source directory was:

```text
~/cpe333-kernel-work/pluem/linux-7.0.0
```

The source package version was:

```text
linux 7.0.0-31.31
```

The source workspace also contained the downloaded Debian source archives and the evidence directory. The source tree was approximately 1.7 GB before the build.

The source preparation commands were:

```bash
cd ~/cpe333-kernel-work/pluem/linux-7.0.0

chmod a+x debian/scripts/*
chmod a+x debian/scripts/misc/*
fakeroot debian/rules clean
```

The cleanup was completed before the first compilation attempt.

**Figure 2a. Kernel source workspace after the source download.**

![Figure 2a. Kernel source workspace after the source download.](evidence/screenshots/02-kernel-source-download.png)

**Figure 2b. Source package target, original changelog, and source-tree size.**

![Figure 2b. Source package target, original changelog, and source-tree size.](evidence/screenshots/03-kernel-source-ready.png)

Evidence files:

- [`06-kernel-source-download.log`](evidence/logs/06-kernel-source-download.log)
- [`07-kernel-source-ready.txt`](evidence/logs/07-kernel-source-ready.txt)
- [`09-kernel-clean.log`](evidence/logs/09-kernel-clean.log)

## 6. Changing the kernel ABI

To distinguish the custom kernel from the installed Ubuntu kernel, only the first line of `debian.master/changelog` was changed. The original and modified lines were:

```text
Original:  linux (7.0.0-31.31) resolute; urgency=medium
Modified:  linux (7.0.0-999.31) resolute; urgency=medium
```

No other changelog entry was changed. The original changelog was preserved in the evidence workspace as `changelog.before-cpe333`.

**Figure 3. Kernel ABI changed from `31` to `999`.**

![Figure 3. Kernel ABI changed from `31` to `999`.](evidence/screenshots/04-kernel-abi-999.png)

The exact one-line diff is recorded in [`08-abi-modification.txt`](evidence/logs/08-abi-modification.txt).

## 7. Selecting the build target and compiling the kernel

The ARM64 packaging configuration provided the `generic` and `generic-64k` flavours. The `generic` flavour was selected for this experiment. The build did not use the full `binary` target and did not build both flavours; the exact target was:

```text
binary-generic
```

The virtual machine had 4 vCPUs, approximately 7.2 GiB of RAM, and 4 GiB of swap. To reduce resource pressure, compilation was limited to two concurrent jobs. Debug-symbol package generation was disabled for this lab build.

The main build command was:

```bash
CONCURRENCY_LEVEL=2 fakeroot debian/rules binary-generic
```

The build output was recorded in:

```text
~/cpe333-kernel-work/pluem/evidence/10-kernel-build-generic.log
```

The first build attempt was deliberately stopped with `Ctrl+C` before the root filesystem was exhausted. That attempt returned exit code `130`.

### 7.1 Disk-space problem and recovery

During compilation, the `debian/build` directory grew to approximately 25 GB. Available space on the original approximately 50 GB root filesystem fell to approximately 3.8 GB. Continuing at that point risked an uncontrolled failure caused by a full filesystem, so the build was interrupted safely.

The UTM virtual disk was then expanded from approximately 50 GB to 64 GB. Inside the guest, the existing root partition and filesystem were extended with:

```bash
sudo growpart /dev/vda 2
sudo resize2fs /dev/vda2
```

The root filesystem remained ext4. After resizing, `/dev/vda2` was approximately 62.9 GB and the root filesystem had approximately 16 GB free at that point. The existing `debian/build` state was retained.

The same build target was resumed without running `clean`:

```bash
CONCURRENCY_LEVEL=2 fakeroot debian/rules binary-generic
```

The resumed output was appended to the existing build log. The resumed build completed successfully with exit code `0`.

**Figure 4a. Kernel compilation in progress.**

![Figure 4a. Kernel compilation in progress.](evidence/screenshots/05-kernel-compilation-progress-01.png)

**Figure 4b. Continued compilation with the build-resource monitor.**

![Figure 4b. Continued compilation with the build-resource monitor.](evidence/screenshots/06-kernel-compilation-progress-02.png)

The second compilation capture contains transient `libfakeroot internal error: payload not recognized!` messages from the live build output. They did not prevent the resumed build from completing: the authoritative build result was exit code `0` and the expected packages were generated.

**Figure 5. Successful `binary-generic` build.**

![Figure 5. Successful `binary-generic` build.](evidence/screenshots/07-kernel-build-success.png)

After the successful build, the build tree was cleaned and APT's local cache was cleared:

```bash
fakeroot debian/rules clean
sudo apt clean
```

The source tree returned to approximately 1.7 GB, `debian/build` was removed, and approximately 40 GB of disk space was free again.

## 8. Generated kernel packages

The successful `binary-generic` build generated the following ARM64 packages. They were saved under `~/cpe333-kernel-work/pluem/packages/`.

| Package | Purpose in this experiment |
| --- | --- |
| `linux-bpf-dev_7.0.0-999.31_arm64.deb` | BPF development files |
| `linux-buildinfo-7.0.0-999-generic_7.0.0-999.31_arm64.deb` | Build information for the generic flavour |
| `linux-headers-7.0.0-999-generic_7.0.0-999.31_arm64.deb` | Kernel headers for the generic flavour |
| `linux-image-unsigned-7.0.0-999-generic_7.0.0-999.31_arm64.deb` | Custom unsigned kernel image |
| `linux-modules-7.0.0-999-generic_7.0.0-999.31_arm64.deb` | Kernel modules for the generic flavour |
| `linux-tools-7.0.0-999-generic_7.0.0-999.31_arm64.deb` | Kernel tools for the generic flavour |

The SHA256 values recorded for the original build artifacts were:

```text
b626614c97b7cabdabae9560232d833923ee5ed3adfdfc38c38cf4025850244c  linux-bpf-dev_7.0.0-999.31_arm64.deb
0d158c0301a58c83433928ca8ae5e28c12f88cdb43035e62573db72173deb20a  linux-buildinfo-7.0.0-999-generic_7.0.0-999.31_arm64.deb
8d0c1e0f751aa2ff4b3c82de2f91c4556b95c61f93c852478d0e1d2de0f83384  linux-headers-7.0.0-999-generic_7.0.0-999.31_arm64.deb
c9e60a86e4a0f9f27178e9236e52dd1312ab54dba132a218e000369568e252fc  linux-image-unsigned-7.0.0-999-generic_7.0.0-999.31_arm64.deb
fca91dcc0f01d81e0e3a4dd48653dcc98022f5edc3ea903850e096443d1e460c  linux-modules-7.0.0-999-generic_7.0.0-999.31_arm64.deb
b26b1199f5abcd2817a23dc3e6ccf83a87e8b98d4a8d51d09e6e842ce9ccfe3f  linux-tools-7.0.0-999-generic_7.0.0-999.31_arm64.deb
```

These values are also recorded in [`13-package-sha256.txt`](evidence/logs/13-package-sha256.txt).

## 9. Package dependency issue and controlled lab adjustment

Before installation, the generated package metadata was inspected. The original modules package contained this dependency:

```text
linux-main-modules-zfs-7.0.0-999-generic, wireless-regdb
```

The custom ABI `999` did not have a corresponding official ZFS package in the configured Ubuntu repositories. The VM used an ext4 root filesystem and did not use ZFS. The environment also had no ZFS pool in use. Therefore, installing the original modules package as-is would require a package that was unavailable and unnecessary for this VM.

The original compiled package was preserved unchanged at:

```text
~/cpe333-kernel-work/pluem/packages/linux-modules-7.0.0-999-generic_7.0.0-999.31_arm64.deb
```

Its SHA256 value remained:

```text
fca91dcc0f01d81e0e3a4dd48653dcc98022f5edc3ea903850e096443d1e460c
```

A separate copy was created for this lab installation at:

```text
~/cpe333-kernel-work/pluem/lab-adjusted/linux-modules-7.0.0-999-generic_7.0.0-999.31_cpe333lab_arm64.deb
```

Only the copied package's `DEBIAN/control` metadata was adjusted. The unavailable custom-ABI ZFS dependency was removed, `wireless-regdb` was retained, and the following marker was added:

```text
X-CPE333-Lab-Adjustment: removed unavailable custom-ABI ZFS dependency
```

The adjusted package was rebuilt with:

```bash
dpkg-deb --root-owner-group --build <adjusted-package-directory> <adjusted-package-file>
```

The adjusted package metadata was:

```text
Package: linux-modules-7.0.0-999-generic
Version: 7.0.0-999.31
Architecture: arm64
Depends: wireless-regdb
X-CPE333-Lab-Adjustment: removed unavailable custom-ABI ZFS dependency
```

The adjusted package SHA256 value was:

```text
c99e48259957d8ff429d67bf5ce41b35fd1789171d23d364f75447ee60106dea
```

This was a lab-specific package metadata workaround. It was not part of the original kernel compilation output, and it does not represent a claim that ZFS was compiled, installed, or used. The original build artifact and its hash were retained for comparison and reproducibility.

The preserved packages were checked again during report QA. Their relevant metadata is recorded side by side in [`19-modules-package-metadata-verification.txt`](evidence/logs/19-modules-package-metadata-verification.txt).

The generated headers package also depended on the missing base package `linux-headers-7.0.0-999`. The headers package was therefore not fully installed for this boot test. It was retained as a generated build artifact, but it was not represented as an installed dependency in the final verification.

## 10. Installing the custom kernel

The installation was tested with a dry run first. The dry run planned exactly two package installations and no removals:

```text
linux-modules-7.0.0-999-generic
linux-image-unsigned-7.0.0-999-generic
```

The actual installation used the lab-adjusted modules package and the original generated image package. It completed with exit code `0`.

The existing `7.0.0-31-generic` packages were retained. This left the original kernel available as a fallback in GRUB.

During installation:

- `/boot/vmlinuz` was updated to point to `vmlinuz-7.0.0-999-generic`.
- `/boot/initrd.img` was updated to point to `initrd.img-7.0.0-999-generic`.
- `dracut` generated `/boot/initrd.img-7.0.0-999-generic`.
- GRUB found both `7.0.0-999-generic` and `7.0.0-31-generic`.
- A GRUB entry was created for `Ubuntu, with Linux 7.0.0-999-generic`.

The DKMS post-install hook reported that automatic module installation was skipped because the headers for `7.0.0-999-generic` did not appear to be installed. This warning did not block installation or boot; the kernel image and the custom kernel modules package were installed successfully for this test.

**Figure 6. Custom kernel package installation and boot files.**

![Figure 6. Custom kernel package installation and boot files.](evidence/screenshots/08-pre-reboot-installation-verification.png)

Evidence files:

- [`14-modules-package-adjustment-sha256.txt`](evidence/logs/14-modules-package-adjustment-sha256.txt)
- [`15-kernel-install-dry-run.log`](evidence/logs/15-kernel-install-dry-run.log)
- [`16-kernel-install.log`](evidence/logs/16-kernel-install.log)
- [`17-pre-reboot-verification.txt`](evidence/logs/17-pre-reboot-verification.txt)

## 11. Reboot and final verification

Before reboot, the running kernel was still `7.0.0-31-generic`, while the custom image and modules packages were installed and the GRUB entry existed. This confirmed that installation completed before changing the running kernel.

After reboot, the running kernel was verified with:

```bash
uname -r
```

The result was:

```text
7.0.0-999-generic
```

The full kernel information also confirmed the ARM64 architecture:

```text
Linux wisit-ubuntu-macos-Apple-Virtualization-Generic-Platform 7.0.0-999-generic #31 SMP PREEMPT_DYNAMIC Thu Sep 17 18:28:17 +07 2026 aarch64 GNU/Linux
```

The installed custom packages were:

```text
linux-image-unsigned-7.0.0-999-generic  7.0.0-999.31  arm64
linux-modules-7.0.0-999-generic        7.0.0-999.31  arm64
```

The boot files were present:

```text
/boot/System.map-7.0.0-999-generic
/boot/config-7.0.0-999-generic
/boot/initrd.img-7.0.0-999-generic
/boot/vmlinuz-7.0.0-999-generic
```

The final filesystem status was approximately 62 GB total, 21 GB used, and 39 GB free on `/`. The EFI partition was mounted at `/boot/efi` with approximately 1.1 GB free.

**Figure 7. Custom kernel running after reboot.**

![Figure 7. Custom kernel running after reboot.](evidence/screenshots/09-kernel-boot-verification.png)

The final proof of a successful compile, installation, and boot is recorded in [`18-final-kernel-verification.txt`](evidence/logs/18-final-kernel-verification.txt):

```text
Running Kernel:
7.0.0-999-generic
```

**Figure 8. Final filesystem and EFI-partition verification.**

![Figure 8. Final filesystem and EFI-partition verification.](evidence/screenshots/10-final-filesystem-verification.png)

## 12. Differences from the reference guide

The repository guide describes the general Ubuntu procedure. The following experiment-specific differences are intentional and are supported by the evidence:

| Topic | Reference procedure | Procedure used in this experiment | Reason |
| --- | --- | --- | --- |
| Build target | `fakeroot debian/rules binary` | `CONCURRENCY_LEVEL=2 fakeroot debian/rules binary-generic` | The ARM64 package defined both `generic` and `generic-64k`; only the required `generic` flavour was built, with concurrency limited for the VM. |
| Interrupted build | Clean before rebuilding | Resume the same target without `clean` after expanding the disk | The interruption was deliberate, the build tree remained intact, and no source or configuration change was made. |
| Modules package | Install the generated package directly | Preserve the original package and install a separately marked lab-adjusted copy | The generated package required an unavailable custom-ABI ZFS package even though this ext4 VM did not use ZFS. |
| Headers | Install generated headers | Do not claim or perform a complete headers installation | The flavour headers depended on the missing base package `linux-headers-7.0.0-999`; the boot test required only the image and modules packages. |
| DKMS | Normally rebuild external modules when headers are available | Record the skipped-module warning | The missing custom headers caused DKMS to skip automatic modules, but did not prevent package installation or boot. |

The original `linux-modules` package was not modified in place, ZFS was not used, and the report does not describe the adjusted package as an original build artifact.

## 13. Evidence index and screenshot inventory

The original terminal evidence remains at:

```text
~/cpe333-kernel-work/pluem/evidence/
```

Reviewer-readable copies are included in [`evidence/logs/`](evidence/logs/). The directory contains files `01` through `09`, `11` through `18`, and the read-only metadata comparison in file `19`. The full `10-kernel-build-generic.log` is retained only in the experiment workspace because it is approximately 7.8 MB; the repository includes its concise success record, [`11-kernel-build-success.txt`](evidence/logs/11-kernel-build-success.txt), together with the build screenshots, generated-package hashes, and final installation evidence.

The complete original evidence inventory is:

```text
01-source-repositories.txt
02-build-dep-dry-run.txt
03-build-dependencies.log
04-extra-build-tools.log
05-build-environment.txt
06-kernel-source-download.log
07-kernel-source-ready.txt
08-abi-modification.txt
09-kernel-clean.log
10-kernel-build-generic.log
11-kernel-build-success.txt
12-post-build-clean.log
13-package-sha256.txt
14-modules-package-adjustment-sha256.txt
15-kernel-install-dry-run.log
16-kernel-install.log
17-pre-reboot-verification.txt
18-final-kernel-verification.txt
```

The original built packages remain under `~/cpe333-kernel-work/pluem/packages/`. The lab-adjusted modules package remains under `~/cpe333-kernel-work/pluem/lab-adjusted/`. The large `.deb` artifacts are not committed to this coursework repository; their recorded SHA256 values allow the original and adjusted modules packages to be distinguished.

The supplied screenshots are now stored in the repository at:

```text
mini-project-01/pluem/evidence/screenshots/
```

The directory contains ten renamed screenshots covering source setup, source preparation, ABI modification, compilation, package generation, installation verification, boot verification, and final filesystem verification. The original capture names and their new report names are mapped in `evidence/screenshots/README.md`.

The only planned screenshot that was not included in the supplied ZIP is the separate build-environment capture. Its absence is stated in Section 3 and is covered by the included text evidence. No screenshot path or image content has been invented.

## 14. Conclusion

The Ubuntu `7.0.0-31.31` source package was prepared on an ARM64 Ubuntu 26.04.1 LTS VM, its ABI was changed to `999`, and the `generic` kernel flavour was built with the `binary-generic` target using two concurrent jobs. The build survived a controlled disk-space interruption, resumed without cleaning, and completed successfully after the virtual disk was expanded.

The original modules package was preserved, while a separately marked lab-adjusted copy was used to work around the unavailable custom-ABI ZFS dependency on this ext4 VM. The custom image and adjusted modules package installed successfully, the original kernel remained as a fallback, and the VM booted the custom kernel. The final verification was:

```text
uname -r -> 7.0.0-999-generic
```
