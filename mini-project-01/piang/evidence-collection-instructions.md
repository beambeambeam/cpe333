# Image-Only Evidence Collection Instructions

## Scope

Collect evidence for the custom Ubuntu 26.04 Server kernel project running in **direct QEMU/KVM**. Evidence files must be images only, preferably PNG files.

Store the screenshots in:

```text
piang/evidence/
```

Do not submit terminal `.txt`, `.log`, or command-output files as evidence. Each screenshot must show the command and the important result together.

This follows the existing repository style in [`beam/media/`](../beam/media/) and [`pluem/evidence/screenshots/`](../pluem/evidence/screenshots/): numbered screenshots with descriptive filenames and report captions.

## Screenshot rules

- Capture the QEMU display window, the guest terminal, or the host terminal that launched QEMU.
- Show the command and its output in the same image.
- Keep the terminal text readable; split a crowded checkpoint into two images.
- Use the exact versions, package names, and paths shown by the VM.
- Do not fabricate, retype, or edit command output.
- Crop only empty space or sensitive information; never crop out the command or result needed for verification.
- Do not show passwords, private keys, tokens, or unrelated host data.
- If a command genuinely fails, preserve the failure screenshot and capture the recovery separately.

If the guest is already running, do not repeat completed build steps. Capture the current state first, then continue from the next missing checkpoint.

## Required screenshots

### 01 — QEMU/KVM guest environment

Filename:

```text
01-qemu-kvm-guest-environment.png
```

Inside the Ubuntu guest, show the OS, virtualization, architecture, initial kernel, and resources:

```bash
echo '=== Ubuntu guest ==='
cat /etc/os-release
echo '=== Virtualization ==='
systemd-detect-virt
echo '=== Initial kernel and architecture ==='
uname -a
uname -m
echo '=== Resources ==='
nproc
free -h
df -hT /
```

The expected virtualization result is normally `kvm` or `qemu`. If the QEMU launch command is available on the host, an optional supporting image may show:

```bash
ps -ef | grep '[q]emu-system-x86_64'
```

Use the optional filename `01a-qemu-launch-command.png`.

### 02 — Source repositories and build dependencies

Filename:

```text
02-source-repositories-and-dependencies.png
```

Show source repositories, a successful package update, and the required build tools:

```bash
echo '=== Source repositories ==='
grep -RInE '^(Types:|URIs:|Suites:|Components:)' \
  /etc/apt/sources.list /etc/apt/sources.list.d 2>/dev/null
echo '=== Package update and dependencies ==='
sudo apt update
sudo apt install -y fakeroot llvm libncurses-dev dwarves
```

The relevant configuration must show source packages, such as `deb-src` or `Types: deb deb-src`.

### 03 — Kernel source ready

Filename:

```text
03-kernel-source-ready.png
```

Show the source package matching the running kernel and the extracted source tree:

```bash
echo '=== Kernel source workspace ==='
uname -r
ls -ld ~/kernel-work/linux-*/
du -sh ~/kernel-work/linux-*/
```

After entering the actual source directory, also show successful preparation:

```bash
cd ~/kernel-work/linux-<actual-version>
chmod a+x debian/scripts/* debian/scripts/misc/*
fakeroot debian/rules clean
```

Replace `<actual-version>` with the directory created by `apt source`.

### 04 — ABI changed to `999`

Filename:

```text
04-kernel-abi-999.png
```

Capture the original first changelog line before editing and the updated line after editing. Show both in one readable image or use:

```text
04a-kernel-abi-before.png
04b-kernel-abi-after.png
```

The first entry should show only the intended ABI change, for example from `X.Y.Z-30.30` to `X.Y.Z-999.30`. Preserve the kernel version, revision, Ubuntu series, and other changelog content.

### 05 — Kernel build in progress

Filename:

```text
05-kernel-build-progress.png
```

Show the QEMU guest actively compiling the kernel:

```bash
cd ~/kernel-work/linux-<actual-version>
CONCURRENCY_LEVEL=2 fakeroot debian/rules binary
```

If compilation takes several screens, use `05-kernel-build-progress-02.png`, `05-kernel-build-progress-03.png`, and so on.

### 06 — Successful build and generated packages

Filename:

```text
06-kernel-build-success-and-packages.png
```

Show the build completion and package files in the parent directory:

```bash
cd ~/kernel-work
ls -lh *.deb
```

The package filenames must contain the custom ABI and include the headers, image, and modules packages for the selected flavour.

### 07 — Installation and pre-reboot verification

Filename:

```text
07-pre-reboot-installation.png
```

Show package installation, installed custom packages, boot files, and the GRUB entry:

```bash
cd ~/kernel-work
sudo dpkg -i ./linux-headers-*.deb \
  ./linux-image-unsigned-*.deb \
  ./linux-modules-*.deb
echo '=== Installed custom packages ==='
dpkg -l | grep '999'
echo '=== Boot files ==='
ls -lh /boot
echo '=== GRUB entry ==='
grep -n '999' /boot/grub/grub.cfg 2>/dev/null
```

If installation genuinely fails, use `07a-installation-error.png` for the error and keep this checkpoint for the successful retry.

### 08 — Custom kernel running after reboot

Filename:

```text
08-kernel-running-after-reboot.png
```

After rebooting the QEMU guest, show the running kernel and installed custom packages:

```bash
echo '=== Running kernel ==='
uname -r
uname -a
echo '=== Custom packages ==='
dpkg -l | grep '999'
```

The `uname -r` value must match the custom ABI and flavour in the package filenames. A stock value such as `7.0.0-30-generic` proves only that the original kernel is running; label that image as baseline or pre-installation evidence, not custom-kernel proof.

### 09 — Final kernel files and filesystem verification

Filename:

```text
09-final-kernel-verification.png
```

Show the modules directory, boot image, initrd, and available root storage for the running version:

```bash
echo '=== Running kernel files ==='
ls -ld /lib/modules/$(uname -r)
ls -lh /boot/vmlinuz-$(uname -r) /boot/initrd.img-$(uname -r)
echo '=== Root filesystem ==='
df -hT /
```

## Optional recovery screenshots

Keep real problems visible and describe their recovery in the report. Examples:

```text
05a-build-failed-disk-space.png
05b-disk-space-recovered.png
07a-installation-error.png
```

The recovery screenshot must show the successful result. Do not present an error screenshot as proof of completion.

## Recommended folder layout

```text
piang/
├── evidence-collection-instructions.md
├── qemu-kvm-ubuntu-26.04-kernel-build-tutorial.md
└── evidence/
    ├── 01-qemu-kvm-guest-environment.png
    ├── 02-source-repositories-and-dependencies.png
    ├── 03-kernel-source-ready.png
    ├── 04-kernel-abi-999.png
    ├── 05-kernel-build-progress.png
    ├── 06-kernel-build-success-and-packages.png
    ├── 07-pre-reboot-installation.png
    ├── 08-kernel-running-after-reboot.png
    └── 09-final-kernel-verification.png
```

## Final checklist

- [ ] Every submitted evidence file is an image.
- [ ] Ubuntu 26.04 Server and QEMU/KVM are visible.
- [ ] Source repositories and build dependencies are shown.
- [ ] The matching kernel source and preparation are shown.
- [ ] The ABI change to `999` is visible.
- [ ] Build progress and successful package generation are shown.
- [ ] Installation and pre-reboot verification are shown.
- [ ] Post-reboot `uname -r` proves the custom kernel is running.
- [ ] The running kernel’s modules, boot image, initrd, and filesystem are verified.
- [ ] Every image has a descriptive filename and a matching report caption.
