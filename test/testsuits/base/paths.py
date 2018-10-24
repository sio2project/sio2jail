import os

BIN_PATH = os.environ.get(
        "SIO2JAIL_BUILD_PATH", os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../build/"))

SOURCE_PATH = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "../../../")

TEST_BIN_PATH = os.path.join(
        BIN_PATH, './test/src/')

SIO2JAIL_BIN_PATH = os.path.join(
        BIN_PATH, './src/sio2jail')

BOXES_PATH = os.path.join(
        BIN_PATH, './boxes/')
