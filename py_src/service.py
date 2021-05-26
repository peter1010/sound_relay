##
# Copyright (c) 2014 Peter Leese
#
# Licensed under the GPL License. See LICENSE file in the project root for full license information.  
##

import sys
import grp
import pwd
import subprocess
import os


def create_group(group):
    try:
        info = grp.getgrnam(group)
        gid = info.gr_gid
    except KeyError:
        subprocess.call(["groupadd", "-r", group])
        info = grp.getgrnam(group)
        gid = info.gr_gid
    return gid


def create_user(user, home, gid):
    try:
        info = pwd.getpwnam(user)
        uid = info.pw_uid
    except KeyError:
        subprocess.call(["useradd", "-r", "-g", str(gid),
                        "-s", "/bin/false", "-d", home, user])
        info = pwd.getpwnam(user)
        uid = info.pw_uid
    return uid


def make_dir(path, uid, gid):
    if not os.path.exists(path):
        os.mkdir(path)
    if uid is not None:
        os.chown(path, uid, gid)
        for f in os.listdir(path):
            os.chown(os.path.join(path, f), uid, gid)


def start_relay_service():
    subprocess.call(["systemctl", "enable", "sound_relay.socket"])
    subprocess.call(["systemctl", "start", "sound_relay.socket"])


def stop_relay_service():
    subprocess.call(["systemctl", "stop", "sound_relay.socket"])
    subprocess.call(["systemctl", "disable", "sound_relay.socket"])

if __name__ == "__main__":
    if sys.argv[1] == "start_relay":
        start_relay_service()
    elif sys.argv[1] == "stop_relay":
        stop_relay_service()
