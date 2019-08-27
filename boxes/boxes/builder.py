import pathlib
import logging
import tempfile
import shutil
import os
import tarfile
from typing import List, Optional

import boxes.utils.cmd
import boxes.boxes


class Builder:
    def __init__(
        self,
        destination: pathlib.Path,
        chroot_path: Optional[pathlib.Path],
        release: str,
        extract: bool
    ):
        self._destination = destination
        self._extract = extract
        self._logger = logging.getLogger('builder')
        self._cmd = boxes.utils.cmd.CommandRunner(self._logger)
        self._release = release

        if chroot_path is None:
            self._workdir = pathlib.Path(
                tempfile.mkdtemp(prefix='sio2jail-box')
            )
            self._root = self._workdir / 'root'
            self._root.mkdir()
            self._do_prepare_chroot = True
        else:
            self._workdir = None
            self._root = chroot_path.resolve()
            if self._root.exists():
                self._do_prepare_chroot = False
            else:
                self._root.mkdir()
                self._do_prepare_chroot = True

    def __del__(self) -> None:
        if not self._workdir:
            return
        try:
            with self._cmd.sudo():
                self._cmd.run('chmod', '-R', 'a+rwX', str(self._workdir))
            self._logger.info("Deleting workdir %s", self._workdir)
            shutil.rmtree(str(self._workdir))
        except Exception:
            self._logger.exception("Failed to delete workdir %s", self._workdir)

    def run(self, boxes_names: List[str]):
        if self._do_prepare_chroot:
            self._prepare_chroot()
        for box in boxes_names:
            if '-' in box:
                box_name, box_version = box.split('-')
                box_dir_name = box_name + box_version.replace('.', '_')
            else:
                box_name, box_version = box, None
                box_dir_name = box_name
            self._build_box(box_name, box_version, box_dir_name)
            if self._extract:
                self._extract_box(box_dir_name + '.tar.gz')

    def _prepare_chroot(self) -> None:
        with self._cmd.sudo():
            self._cmd.run('debootstrap', self._release, str(self._root))
        with self._cmd.chroot(self._root):
            self._cmd.run('apt-get', 'install', '-y', '--allow-unauthenticated', 'apt-rdepends')

    def _build_box(self, box_name: str, box_version: Optional[str], box_dir_name: str) -> None:
        box = boxes.boxes.BOXES[box_name](self._logger, box_version)
        with self._cmd.chroot(self._root):
            box_dir = pathlib.Path('/') / box_dir_name
            self._cmd.run('rm', '-rf', str(box_dir))
            self._cmd.run('mkdir', '-pv', str(box_dir))

            for package_name in box.get_packages():
                self._cmd.run('apt-get', 'install', '-y', '--allow-unauthenticated', package_name)

            packages = self._build_dependency_tree(box.get_packages())
            for package in packages:
                self._extract_package(package, box_dir)

            box.finalize_box(self._cmd, box_dir)

        with self._cmd.sudo():
            self._compress_box(self._root / box_dir_name)

    def _build_dependency_tree(self, packages: List[str]) -> List[str]:
        deps = {}
        not_installed = set()
        for package in packages:
            current_package = None
            for line in self._cmd.run('apt-rdepends', '-p', package, return_stdout=True):
                line = line.strip()
                if 'Depends' in line:
                    deps.setdefault(current_package, set()).add(line.split()[1].strip())
                    if line.split()[-1].strip() == "[NotInstalled]":
                        not_installed.add(line.split()[1].strip())
                else:
                    current_package = line.strip()
        install_order = []
        visited = set()

        def visit(node):
            if node in visited:
                return
            visited.add(node)
            for child_node in deps.get(node, []):
                visit(child_node)
            if node not in not_installed:
                install_order.append(node)
        for node in deps.keys():
            visit(node)

        print(repr(deps))
        print(repr(install_order))
        return install_order

    def _extract_package(self, package: str, box_dir: pathlib.Path) -> None:
        package_file_name = self._cmd.run(
            'apt-get',
            'download',
            package,
            '--print-uris',
            return_stdout=True)[0].split()[1]
        self._cmd.run(
            'apt-get',
            'install',
            '--reinstall',
            '-y',
            '--allow-unauthenticated',
            '-d',
            package)
        self._cmd.run(
            'dpkg',
            '-X',
            str(pathlib.Path('/var/cache/apt/archives/') / package_file_name),
            str(box_dir))

    def _compress_box(self, box_dir: pathlib.Path) -> None:
        archive_name = self._destination / (box_dir.name + '.tar.gz')
        self._cmd.run(
            'tar',
            '-p',
            '-c',
            '-a',
            '-v',
            '-f',
            str(archive_name.resolve()),
            str(box_dir.name),
            cwd=box_dir.parent)
        self._cmd.run(
            'chown',
            os.getenv('USER') + ':',
            str(archive_name))

    def _extract_box(self, box_name: str) -> None:
        self._logger.info("Extracting %s", box_name)
        box_path = self._destination / box_name
        with tarfile.open(str(box_path), 'r:*') as f:
            f.extractall(str(self._destination))
