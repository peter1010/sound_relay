pkgbase=sound_relay
pkgname=('sound_relay' 'sound_player')
pkgver=1.0
pkgrel=3
pkgdesc="sound_relay"
arch=('any')
url="http:"
license=('GPL')
makedepends=('python')
depends=('systemd' 'opus' 'python')
source=()
install='sound_relay.install'

pkgver() {
    python ../version.py
}

check() {
    pushd ..
    python setup_sound_relay.py check
    python setup_sound_player.py check
    popd
}

package_sound_relay() {
    pushd ..
    DONT_START=1 python setup_sound_relay.py install --root=$pkgdir
    popd
}

package_sound_player() {
    pushd ..
    DONT_START=1 python setup_sound_player.py install --root=$pkgdir
    popd
}
