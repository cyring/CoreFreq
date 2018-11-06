# Maintainer: CyrIng <labs[at]cyring[dot]fr>
# Contributor: CyrIng <labs[at]cyring[dot]fr>
_gitname=CoreFreq
pkgname=corefreq
pkgver=1.37
pkgrel=0.1
pkgdesc="CoreFreq, a processor monitoring software with a kernel module inside."
arch=('x86_64')
url='https://github.com/cyring/CoreFreq'
license=('GPL2')
depends=('dkms')
makedepends=('git' 'sed')
source=(git+${url}.git)
md5sums=('SKIP')
install=${pkgname}.install

package() {
	cd ${srcdir}/${_gitname}
	make DESTDIR=${pkgdir} dkms_install service_install
}

pkgver() {
   cd "${srcdir}/${_gitname}"
   echo "$(sed -nE 's/#define\s+COREFREQ_VERSION\s+"([0-9\.]+)"/\1/p' coretypes.h).r$(git rev-list --count HEAD).g$(git rev-parse --short HEAD)"
}
