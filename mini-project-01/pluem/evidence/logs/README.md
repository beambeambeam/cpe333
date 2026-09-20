# Terminal Evidence

These files are exact copies of terminal evidence captured during the Mini-Project 01 experiment. They are ordered chronologically and are linked from the report.

| File | Evidence represented |
| --- | --- |
| `01-source-repositories.txt` | Enabled `deb-src` repositories and available Linux source versions |
| `02-build-dep-dry-run.txt` | Dependency-resolution dry run |
| `03-build-dependencies.log` | Kernel build-dependency installation |
| `04-extra-build-tools.log` | Installation of additional build tools |
| `05-build-environment.txt` | Original kernel, architecture, toolchain, memory, swap, and disk space |
| `06-kernel-source-download.log` | Source-package download and extraction |
| `07-kernel-source-ready.txt` | Exact source version, downloaded files, changelog, and initial source size |
| `08-abi-modification.txt` | One-line ABI change from `31` to `999` |
| `09-kernel-clean.log` | Pre-build clean step |
| `11-kernel-build-success.txt` | Successful build summary and generated package list |
| `12-post-build-clean.log` | Post-build cleanup |
| `13-package-sha256.txt` | SHA256 values of the original generated packages |
| `14-modules-package-adjustment-sha256.txt` | Separate SHA256 values for the original and lab-adjusted modules packages |
| `15-kernel-install-dry-run.log` | Installation dry run: two installs and no removals |
| `16-kernel-install.log` | Package installation, DKMS warning, initramfs generation, and GRUB update |
| `17-pre-reboot-verification.txt` | Installed packages, boot files, and GRUB entry before reboot |
| `18-final-kernel-verification.txt` | Running custom kernel, installed packages, boot files, and final filesystem state |
| `19-modules-package-metadata-verification.txt` | Read-only comparison of the preserved original and lab-adjusted modules-package metadata |

The full `10-kernel-build-generic.log` remains at `~/cpe333-kernel-work/pluem/evidence/10-kernel-build-generic.log`. It is approximately 7.8 MB and was not copied into Git. Build completion is demonstrated by `11-kernel-build-success.txt`, the package checksums, the installation log, the screenshots, and the final post-reboot verification.

The original backup `changelog.before-cpe333` also remains in the experiment workspace. Its relevant first line and the exact one-line diff are included in `08-abi-modification.txt`.
