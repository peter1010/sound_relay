pkgbase=sound_relay
pkgname=('sound_relay')
pkgver=1.0
pkgrel=2
pkgdesc="sound_relay"
arch=('any')
url="http:"
license=('GPL')
makedepends=('python')
depends=('systemd' 'opus' 'python')
source=()
install='sound_relay.install'

pkgver() {
    python ../setup.py -V
}

check() {
    pushd ..
    python setup.py check
    popd
}

package() {
    pushd ..
    DONT_START=1 python setup.py install --root=$pkgdir
    popd
}

