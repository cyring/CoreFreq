# Maintainer: CyrIng <labs[at]cyring[dot]fr>
# Contributor: CyrIng <labs[at]cyring[dot]fr>
pkgbase=CoreFreq
pkgname=('corefreq')
pkgver=1.0.0
pkgrel=1
pkgdesc='Processor Core Monitoring'
arch=('x86_64')
url='http://github.com/cyring/CoreFreq'
license=('GPL2')
makedepends=('git' 'linux-headers')
source=('git+https://github.com/cyring/CoreFreq.git')
md5sums=('SKIP')
install=intelfreq.install

build() {
	cd ${srcdir}/${pkgbase}
	make -w -j1
}

package_corefreq() {
	pkgdesc='IntelFreq kernel module sources'
	depends=('dkms' 'gcc' 'make')
	install -dm755 "${pkgdir}/usr/src"
	pushd .
	cd ${srcdir}/${pkgbase}
	install -Dm755 corefreqd "${pkgdir}/usr/bin/corefreqd"
	install -Dm644 corefreqd.service "${pkgdir}/usr/lib/systemd/system/corefreqd.service"
	install -m755 corefreq-cli "${pkgdir}/usr/bin/corefreq-cli"
	mkdir "${pkgdir}/usr/src/${pkgbase}-${pkgver}/"
	install -Dm644 dkms.conf "${pkgdir}/usr/src/${pkgbase}-${pkgver}/dkms.conf"
	cp --no-preserve=ownership Makefile *.h *.c dkms.conf *.install \
		"${pkgdir}/usr/src/${pkgbase}-${pkgver}/"
	popd
}
