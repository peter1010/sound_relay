#!/usr/bin/env python3

##
# Copyright (c) 2014 Peter Leese
#
# Licensed under the GPL License. See LICENSE file in the project root for full license information.  
##


import os
import subprocess
import sys

from distutils.core import setup
from distutils.command import install, build

from version import VERSION

def find_c_src():
    c_src = "c_src"
    for i in range(5):
        if os.path.exists(c_src):
            break
        c_src = os.path.join("..", c_src)
    else:
        sys.exit(1)
    return c_src



class my_install(install.install):
    def run(self):
        retVal = super().run()
        if self.root is None or not self.root.endswith("dumb"):
            if not os.getenv("DONT_START"):
                print("Setup.py starting the services")
                from py_src import service
                service.start_service()
        return retVal


class my_build(build.build):
    def run(self):
        retVal = super().run()
        subprocess.call(['make', '-C', find_c_src()])
        return retVal


def get_play_rtp_exe(do_compile=True):
   return os.path.join(find_c_src(), "__%s__" % os.uname().machine, "play_rtp")


setup(
    name='sound_player',
    version=VERSION,
    description="Sound Player",
    url='https://github.com/peter1010/sound_relay',
    author='Peter1010',
    author_email='peter1010@localnet',
    license='GPL',
    package_dir={'sound_player': 'py_src' },
    packages=['sound_player'],
    data_files=[
        ('/usr/bin/', (get_play_rtp_exe(),))
    ],
    cmdclass={'install': my_install, 'build': my_build}
)
