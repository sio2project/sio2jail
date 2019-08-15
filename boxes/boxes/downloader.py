import datetime
import hashlib
import logging
import pathlib
import urllib.parse
import tarfile
from typing import Iterator, Tuple

import requests


class Downloader:
    MANIFEST_FILE_NAME = 'manifest.txt'

    def __init__(self, url: str, destination: pathlib.Path, extract: bool):
        self._url = url
        self._destination = destination
        self._extract = extract
        self._logger = logging.getLogger('downloader')

    def run(self) -> None:
        self._destination.mkdir(parents=True, exist_ok=True)
        self._logger.info("Downloading started")
        manifest = list(self._fetch_manifest())
        for index, (box_hash, box_name) in enumerate(manifest):
            self._logger.info("%d/%d Processing box %s hash %s", index + 1, len(manifest), box_name, box_hash)
            local_box_hash = self._get_box_hash(box_name)
            if local_box_hash == box_hash:
                self._logger.info("Box %s already downloaded", box_name)
            else:
                self._fetch_box(box_name)
            local_box_hash = self._get_box_hash(box_name)
            self._logger.info("Box %s has hash %s", box_name, local_box_hash)
            if local_box_hash != box_hash:
                raise Exception("Failed to verify box %s", box_name)
            if self._extract:
                self._extract_box(box_name)
        self._logger.info("Downloading finished")

    def _fetch_manifest(self) -> Iterator[Tuple[str, str]]:
        manifest_url = urllib.parse.urljoin(self._url, Downloader.MANIFEST_FILE_NAME)
        r = requests.get(manifest_url)
        if not r.ok:
            raise Exception("Failed to download manifest file: {} {}".format(
                r.status_code, r.reason))
        for line in r.iter_lines():
            box_hash, box_name = line.decode('utf-8').split()
            yield box_hash, box_name

    def _fetch_box(self, box_name: str) -> None:
        box_url = urllib.parse.urljoin(self._url, box_name)
        with requests.get(box_url, stream=True) as r:
            if not r.ok:
                raise Exception("Failed to download box {} from {}: {} {}".format(
                    box_name, box_url, r.status_code, r.reason))
            total_length, local_length, next_notify = int(r.headers.get('Content-Length', -1)), 0, 0.1
            self._logger.info("Downloading box %s from %s", box_name, box_url)
            start_time = datetime.datetime.now()
            with (self._destination / box_name).open('wb') as f:
                for chunk in r.iter_content(chunk_size=8192):
                    f.write(chunk)
                    local_length += len(chunk)
                    if total_length > 0 and local_length / total_length >= next_notify:
                        next_notify = (10 * local_length // total_length) / 10 + 0.1
                        speed = local_length / (datetime.datetime.now() - start_time).total_seconds()
                        self._logger.info(
                            "Downloading box %3d%% (%8.3fMiB/%.3fMiB %.3fMiB/s)",
                            int(100 * local_length / total_length),
                            local_length / 1024 / 1024,
                            total_length / 1024 / 1024,
                            speed / 1024 / 1024)

    def _extract_box(self, box_name: str) -> None:
        self._logger.info("Extracting %s", box_name)
        box_path = self._destination / box_name
        with tarfile.open(str(box_path), 'r:*') as f:
            f.extractall(str(self._destination))

    def _get_box_hash(self, box_name: str) -> str:
        box_path = self._destination / box_name
        if not box_path.exists():
            return None
        box_hash = hashlib.sha256()
        with box_path.open('rb') as box_file:
            while True:
                chunk = box_file.read(4096)
                if not chunk:
                    break
                box_hash.update(chunk)
        return box_hash.hexdigest()
