#!/bin/sh
# macOS helper: runs the project in an Ubuntu VM via Lima.
#   ./vm.sh setup         create VM + install kernel headers and build tools
#   ./vm.sh shell         open a shell in the VM, in this folder
#   ./vm.sh make [TARGET] run make in the VM (all, load, unload, demo, ...)
#   ./vm.sh run CMD...    run any command in the VM, in this folder
#   ./vm.sh stop|delete
set -eu
VM=cpe333
DIR=$(realpath "$(dirname "$0")")  # real on-disk case: the VM mount is case-sensitive

case "${1:-}" in
setup)
	command -v limactl >/dev/null || brew install lima
	limactl list -q | grep -qx "$VM" ||
		limactl create --name="$VM" --cpus=4 --memory=4 \
			--mount-only "$DIR:w" --yes template:ubuntu-24.04
	limactl start "$VM" 2>/dev/null || true
	limactl shell "$VM" sh -c 'sudo apt-get -o DPkg::Lock::Timeout=600 update -q &&
		sudo DEBIAN_FRONTEND=noninteractive apt-get -o DPkg::Lock::Timeout=600 install -yq \
			build-essential linux-headers-$(uname -r) python3'
	;;
shell) limactl shell --workdir "$DIR" "$VM" ;;
make)  shift; limactl shell --workdir "$DIR" "$VM" make "$@" ;;
run)   shift; limactl shell --workdir "$DIR" "$VM" "$@" ;;
stop)  limactl stop "$VM" ;;
delete) limactl delete -f "$VM" ;;
*) sed -n '2,8p' "$0"; exit 2 ;;
esac
