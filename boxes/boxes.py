#!/usr/bin/env python3

import argparse
import logging
import pathlib

import boxes.boxes.base
import boxes.downloader
import boxes.builder


def main() -> None:
    args = parse_args()
    if args.action == 'download':
        boxes.downloader.Downloader(
            args.url,
            args.destination,
            args.extract
        ).run()
    elif args.action == 'build':
        boxes.builder.Builder(
            args.destination,
            args.chroot_path,
            args.release,
            args.extract
        ).run(args.boxes or boxes.boxes.DEFAULT_BOXES)
    else:
        raise NotImplementedError()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    actions = parser.add_subparsers(
        title='action',
        dest='action')

    parser_download = actions.add_parser(
        'download',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser_download.add_argument(
        '-u', '--url',
        help="Base boxes repository location",
        metavar='URL',
        type=str,
        # TODO: Change url to downloads as soon as these boxes are
        # be finished and uploaded there.
        default='https://hitagi.dasie.mimuw.edu.pl/files/boxes/')
    parser_download.add_argument(
        '-d', '--destination',
        help="Destination directory",
        metavar='DIR',
        type=pathlib.Path,
        default=pathlib.Path('.'))
    parser_download.add_argument(
        '-x', '--extract',
        help="Extract after downloading",
        action='store_true')

    parser_build = actions.add_parser(
        'build',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser_build.add_argument(
        '-d', '--destination',
        help="Destination directory",
        metavar='DIR',
        type=pathlib.Path,
        default=pathlib.Path('.'))
    parser_build.add_argument(
        '-x', '--extract',
        help="Extract after building",
        action='store_true')
    parser_build.add_argument(
        '-p', '--chroot-path',
        help="Chroot to use, will be created if needed, uses temporary directory by default",
        type=pathlib.Path)
    parser_build.add_argument(
        '-r', '--release',
        help="Debian release to use as when creating new chroot",
        default="stable",
        type=str)
    parser_build.add_argument(
        'boxes',
        help="Name of a box to build, optinally suffixed with version e.g 'python' or 'python-3.7'",
        metavar='BOX',
        nargs='*')

    args = parser.parse_args()
    if args.action is None:
        parser.error('argument action: argument is required')
    return args


def configure_logging() -> None:
    fmt = logging.Formatter("%(asctime)s\t%(levelname)s\t%(message)s", "%Y-%m-%d %H:%M:%S")

    handler = logging.StreamHandler()
    handler.setFormatter(fmt)

    root_logger = logging.getLogger()
    root_logger.setLevel(logging.DEBUG)
    root_logger.addHandler(handler)

    logging.getLogger('urllib3').setLevel(logging.INFO)


if __name__ == "__main__":
    configure_logging()
    try:
        main()
    except Exception:
        logging.exception("Exception occurred")
        raise
