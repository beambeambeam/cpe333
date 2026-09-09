# How to build an Ubuntu Linux kernel

If you have patches you need to apply to the Ubuntu Linux kernel, or you want to change some kernel configurations, you may need to build your kernel from source. Follow these steps to customise and build the Ubuntu Linux kernel.

> **Important:** Kernels built using this method are not intended for use in production.

## Prerequisites

- This guide supports Xenial Xerus and newer.
- It is recommended to have at least 8 GB of RAM and 30 GB of free disk space on the build machine.

If this is the first time you are building a kernel on your system, you will need to [set up the build environment](#set-up-build-environment) and [install the required packages](#install-required-packages).

Otherwise, skip ahead to [obtaining the source for an Ubuntu release](#obtain-the-source-for-an-ubuntu-release).

## Set up build environment

To build an Ubuntu kernel, you will need to enable the necessary source repositories in the `sources.list` or `ubuntu.sources` file.

See [How to enable kernel source package repositories](https://ubuntu.com/kernel/docs/how-to/source-code/enable-source-repositories/) for details.

## Install required packages

To install the required packages and build dependencies, run:

```bash
sudo apt update && \
    sudo apt build-dep -y linux linux-image-unsigned-$(uname -r) && \
    sudo apt install -y fakeroot llvm libncurses-dev dwarves
```

## Obtain the source for an Ubuntu release

There are different ways to get the kernel sources, depending on the kernel version you want to modify.

### Get kernel source for the version installed on the build machine

Use the `apt source` command to get the source code for the kernel version currently running on your build machine:

```bash
apt source linux-image-unsigned-$(uname -r)
```

This downloads and unpacks the kernel source files to your current working directory.

```text
<working_directory>
├── linux-X.Y.Z/
│   └── *
├── linux_X.Y.Z-*.diff.gz
├── linux_X.Y.Z-*.dsc
└── linux_X.Y.Z.orig.tar.gz
```

### Get kernel source for other versions

Use Git to get the source code for other kernel versions. See [How to obtain kernel source for an Ubuntu release using Git](https://ubuntu.com/kernel/docs/how-to/source-code/obtain-kernel-source-git/) for detailed instructions.

## Prepare the kernel source

Once you have the kernel source, go to the kernel source working directory, such as `linux-6.8.0`, and run the following commands to ensure you have a clean build environment and the necessary scripts have execute permissions:

```bash
cd <kernel_source_working_directory>

chmod a+x debian/scripts/* && \
    chmod a+x debian/scripts/misc/* && \
    fakeroot debian/rules clean
```

## Modify ABI number

You should modify the kernel version number to avoid conflicts and to differentiate the development kernel from the kernel released by Canonical.

To do so, modify the ABI number – the number after the dash following the kernel version – to `999` in the first line of the `<kernel_source_working_directory>/debian.master/changelog` file.

For example, modify the ABI number to `999` for Noble Numbat:

```text
linux (6.8.0-999.48) noble; urgency=medium
```

If you are building something other than the generic Ubuntu Linux kernel, modify the ABI number in the `<kernel_source_working_directory>/debian.<derivative>/changelog` file instead.

## Modify kernel configuration

(Optional) To enable or disable features using the kernel configuration, run:

```bash
cd <kernel_source_working_directory>
fakeroot debian/rules editconfigs
```

This invokes the `menuconfig` interface so you can edit specific configuration files related to the Ubuntu kernel package. You will need to explicitly respond with `Y` or `N` when making configuration changes to avoid errors later in the build process.

> **Note:** If you do not have the compiler toolchain installed for each architecture supported by the kernel being configured, you will see errors that the configurations for those uninstalled architectures are missing. These can be ignored as long as you do not intend to build binaries for those architectures.

## Customize the kernel

(Optional) Add any firmware, binary blobs, or patches as needed.

## Build the kernel

You are now ready to build the kernel.

```bash
cd <kernel_source_working_directory>

fakeroot debian/rules clean && \
    fakeroot debian/rules binary
```

> **Note:** Run `fakeroot debian/rules clean` to clean the build environment each time before you recompile the kernel after making changes to the kernel source or configuration.

If the build is successful, several `.deb` binary package files will be produced in the directory one level above the kernel source working directory.

For example, building a kernel with version `6.8.0-999.48` on an x86-64 system will produce the following `.deb` packages, among others:

- `linux-headers-6.8.0-999_6.8.0-999.48_all.deb`
- `linux-headers-6.8.0-999-generic_6.8.0-999.48_amd64.deb`
- `linux-image-unsigned-6.8.0-999-generic_6.8.0-999.48_amd64.deb`
- `linux-modules-6.8.0-999-generic_6.8.0-999.48_amd64.deb`

## Install the new kernel

Install all the Debian packages generated in the previous step, either on your build system or on a different target system with the same architecture, using `dpkg -i`. Then reboot:

```bash
cd <kernel_source_working_directory>/../
sudo dpkg -i linux-headers-<kernel version>*_all.deb
sudo dpkg -i linux-headers-<kernel version>-<generic or derivative>*.deb
sudo dpkg -i linux-image-unsigned-<kernel version>-<generic or derivative>*.deb
sudo dpkg -i linux-modules-<kernel version>-<generic or derivative>*.deb
sudo reboot
```

## Test the new kernel

Run any necessary tests to confirm that your changes and customisations have taken effect. You should also confirm that the newly installed kernel version matches the value in the `<kernel_source_working_directory>/debian.master/changelog` file by running:

```bash
uname -r
```

_Source: [How to build an Ubuntu Linux kernel](https://ubuntu.com/kernel/docs/how-to/develop-customise/build-kernel/)_
