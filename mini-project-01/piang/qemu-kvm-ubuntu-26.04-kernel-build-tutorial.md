# Building and Installing a Custom Ubuntu 26.04 Kernel with QEMU/KVM

This tutorial describes a repeatable setup for compiling and installing a custom Ubuntu Linux kernel inside an Ubuntu 26.04 Server virtual machine running on QEMU/KVM.

If the Ubuntu image and guest are already running, begin at [Guest baseline](#guest-baseline). The project-specific kernel procedure is also documented in [`how-to-build-an-ubuntu-linux-kernel.md`](../how-to-build-an-ubuntu-linux-kernel.md).

## 1. Host prerequisites

Run these checks on the Linux host:

```bash
qemu-system-x86_64 --version
qemu-img --version
test -e /dev/kvm && echo 'KVM is available'
ls -l /dev/kvm
```

Install the host tools if they are missing. On an Ubuntu host:

```bash
sudo apt update
sudo apt install -y qemu-kvm qemu-utils ovmf
```

The host account must be allowed to access `/dev/kvm`. Log out and back in after changing group membership if required:

```bash
sudo usermod -aG kvm "$USER"
```

Do not use `-enable-kvm` unless `/dev/kvm` is available. Without KVM acceleration, a full kernel build can be impractically slow.

## 2. Create the QEMU virtual disk

Choose a workspace on the host with enough free space. Kernel builds can use tens of gigabytes temporarily; allocate at least 60–80 GB to the guest.

```bash
VM_DIR=~/vms/cpe333-ubuntu26.04
VM_DISK="$VM_DIR/ubuntu26.04-kernel.qcow2"

mkdir -p "$VM_DIR"
qemu-img create -f qcow2 "$VM_DISK" 80G
qemu-img info "$VM_DISK"
```

The QCOW2 image is sparse: the virtual capacity is 80 GB, while the host initially stores only the space that has been used.

Create the image only once. Re-running `qemu-img create` on an existing path can destroy the VM disk.

## 3. Install Ubuntu 26.04 Server

Set the path to the Ubuntu Server ISO on the host:

```bash
ISO_IMAGE=~/Downloads/ubuntu-26.04-live-server-amd64.iso
```

Start the installer with QEMU/KVM:

```bash
qemu-system-x86_64 \
  -name cpe333-ubuntu26.04 \
  -enable-kvm \
  -machine q35 \
  -cpu host \
  -smp 8 \
  -m 6G \
  -drive file="$VM_DISK",if=virtio,format=qcow2 \
  -cdrom "$ISO_IMAGE" \
  -boot menu=on \
  -nic user,model=virtio \
  -display gtk
```

Recommended installer choices:

- Ubuntu Server 26.04;
- the default `amd64` architecture for an x86-64 QEMU/KVM host;
- an ordinary user account for the kernel work;
- OpenSSH Server if remote terminal access is useful; and
- enough root storage for the source tree, build output, and generated packages.

When installation finishes, shut down the guest cleanly. Start it again without the ISO:

```bash
qemu-system-x86_64 \
  -name cpe333-ubuntu26.04 \
  -enable-kvm \
  -machine q35 \
  -cpu host \
  -smp 8 \
  -m 6G \
  -drive file="$VM_DISK",if=virtio,format=qcow2 \
  -boot order=c \
  -nic user,model=virtio \
  -display gtk
```

If the guest was installed with UEFI, add persistent OVMF variables to both launch commands. Copy the variables file once for this VM and use the copy; do not write directly to the system-wide template:

```bash
cp /usr/share/OVMF/OVMF_VARS_4M.fd "$VM_DIR/OVMF_VARS.fd"
```

Add these options to the QEMU command when the files exist:

```bash
-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
-drive if=pflash,format=raw,file="$VM_DIR/OVMF_VARS.fd"
```

## 4. Guest baseline

Run these commands inside the Ubuntu guest before changing anything. They identify the guest and record the initial stock kernel:

```bash
echo '=== OS ==='
cat /etc/os-release
echo '=== Virtualization ==='
systemd-detect-virt
echo '=== Initial kernel ==='
uname -a
uname -m
echo '=== Resources ==='
nproc
free -h
df -hT /
```

For this project, the expected virtualization result is normally `kvm` or `qemu`, and the architecture is normally `x86_64`/`amd64`.

Take the first evidence screenshot here. See [`evidence-collection-instructions.md`](evidence-collection-instructions.md) for the required image filename.

## 5. Enable source repositories and install dependencies

Ubuntu kernel source packages require source repositories. Inspect the active configuration:

```bash
grep -RInE '^(Types:|URIs:|Suites:|Components:)' \
  /etc/apt/sources.list /etc/apt/sources.list.d 2>/dev/null
```

For deb822 files, the relevant entry should include both binary and source types:

```text
Types: deb deb-src
```

For traditional sources-list entries, the source line begins with `deb-src`. After enabling the source entries, install the dependencies:

```bash
sudo apt update
sudo apt build-dep -y linux linux-image-unsigned-$(uname -r)
sudo apt install -y fakeroot llvm libncurses-dev dwarves
```

If `apt build-dep` reports that no source package is available, fix the `deb-src` configuration, run `sudo apt update` again, and retry. Capture the successful configuration as an image.

## 6. Download and prepare the matching kernel source

Download the source package matching the kernel currently running in the guest:

```bash
mkdir -p ~/kernel-work
cd ~/kernel-work
apt source linux-image-unsigned-$(uname -r)
ls -ld linux-*/
du -sh linux-*/
```

Enter the directory printed by `ls -ld`:

```bash
cd ~/kernel-work/linux-<actual-version>
chmod a+x debian/scripts/*
chmod a+x debian/scripts/misc/*
fakeroot debian/rules clean
```

Replace `<actual-version>` with the directory actually created by `apt source`.

## 7. Change the Ubuntu ABI number

Use a custom ABI so the development kernel can coexist with the stock Ubuntu kernel and remain distinguishable. Preserve the original first changelog line before editing:

```bash
head -n 1 debian.master/changelog
```

Edit only the ABI portion of the first entry. For example:

```text
Before: linux (X.Y.Z-30.30) resolute; urgency=medium
After:  linux (X.Y.Z-999.30) resolute; urgency=medium
```

Keep the kernel version, Ubuntu series, and revision unchanged. Verify the edited line:

```bash
head -n 1 debian.master/changelog
```

Take screenshots before and after the edit, or show both lines in one readable screenshot. Do not change unrelated changelog entries.

## 8. Build the kernel packages

Check storage before starting. The build directory can grow substantially:

```bash
df -hT /
```

For an 8-vCPU, 6-GB guest, two concurrent jobs are a conservative starting point:

```bash
cd ~/kernel-work/linux-<actual-version>
CONCURRENCY_LEVEL=2 fakeroot debian/rules binary
```

The build can take a long time. Do not close QEMU or power off the guest while it is compiling.

When it finishes, inspect the packages in the parent directory:

```bash
cd ~/kernel-work
ls -lh *.deb
```

The generated package names should contain the custom ABI, for example:

```text
linux-headers-X.Y.Z-999-generic_*.deb
linux-image-unsigned-X.Y.Z-999-generic_*.deb
linux-modules-X.Y.Z-999-generic_*.deb
```

Use the exact package names and flavour produced by the guest. Take screenshots of build completion and the generated package list.

If the build fails because the guest disk is full, preserve that screenshot, shut down cleanly, enlarge the virtual disk using the appropriate partition/filesystem procedure, and document the recovery. Do not describe a failed attempt as a successful build.

## 9. Install the generated packages

From the directory containing the generated packages, install the required headers, image, and modules packages:

```bash
cd ~/kernel-work
sudo dpkg -i ./linux-headers-*.deb \
  ./linux-image-unsigned-*.deb \
  ./linux-modules-*.deb
```

If `dpkg` reports a dependency problem, keep the error visible, resolve it according to the project procedure, and capture the successful retry separately.

Verify the installed files before rebooting:

```bash
dpkg -l | grep '999'
ls -lh /boot
grep -n '999' /boot/grub/grub.cfg 2>/dev/null
```

## 10. Reboot and verify the custom kernel

Reboot the guest normally:

```bash
sudo update-grub
sudo reboot
```

If GRUB does not select the new kernel automatically, select it from **Advanced options for Ubuntu**. After the guest starts, verify the running version:

```bash
uname -r
uname -a
dpkg -l | grep '999'
ls -ld /lib/modules/$(uname -r)
ls -lh /boot/vmlinuz-$(uname -r) /boot/initrd.img-$(uname -r)
```

The `uname -r` result must contain the custom ABI and match the generated package names. A value such as `7.0.0-30-generic` proves that the stock kernel is running; it does not prove that the custom kernel has booted. Record the exact value shown by the guest.

## 11. QEMU lifecycle and recovery

Use a graceful guest shutdown when possible:

```bash
sudo poweroff
```

Do not delete the QCOW2 file to recover from a kernel mistake. Keep the stock Ubuntu kernel installed so it remains a fallback. If the guest is powered off, make a QCOW2 snapshot before a risky next step:

```bash
qemu-img snapshot -c before-custom-kernel "$VM_DISK"
qemu-img snapshot -l "$VM_DISK"
```

Snapshots are not a substitute for the evidence screenshots. Keep the command output and final running-kernel proof visible in the guest screenshots.

## 12. Evidence and report

The submission report should describe the actual commands, versions, package names, failures, and recovery steps used in this VM. Store only PNG screenshots in `piang/evidence/` and reference them from the report. Use the image-only checklist in [`evidence-collection-instructions.md`](evidence-collection-instructions.md).
