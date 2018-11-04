# Maintainer: CyrIng <labs[at]cyring[dot]fr>
# Contributor: CyrIng <labs[at]cyring[dot]fr>
pkgname=corefreq-git
pkgver=1.37.0
pkgrel=1
pkgdesc="CoreFreq, a processor monitoring software with a kernel module inside."
arch=('x86_64')
url='http://github.com/cyring/CoreFreq'
license=('GPL2')
depends=('dkms')
makedepends=('git' 'sed')
source=('git+https://github.com/cyring/CoreFreq.git')
md5sums=('SKIP')
install=${pkgname}.install

_gitname=CoreFreq

build() {
	cd ${srcdir}/${_gitname}
	make -w -j1
}

package() {
	
	install -dm755 "${pkgdir}/usr/src"
	cd ${srcdir}/${_gitname}
	install -Dm755 corefreqd "${pkgdir}/usr/bin/corefreqd"
	install -Dm644 corefreqd.service "${pkgdir}/usr/lib/systemd/system/corefreqd.service"
	install -m755 corefreq-cli "${pkgdir}/usr/bin/corefreq-cli"
	mkdir "${pkgdir}/usr/src/${pkgname}-${pkgver}/"
	install -Dm644 dkms.conf "${pkgdir}/usr/src/${pkgname}-${pkgver}/dkms.conf"
	cp --no-preserve=ownership Makefile *.h *.c dkms.conf \
		"${pkgdir}/usr/src/${pkgname}-${pkgver}/"
}

pkgver() {
    cd "${srcdir}/${_gitname}"
    echo "$(sed -nE 's/#define\s+COREFREQ_VERSION\s+"([0-9\.]+)"/\1/p' coretypes.h).r$(git rev-list --count HEAD).g$(git rev-parse --short HEAD)"
}
