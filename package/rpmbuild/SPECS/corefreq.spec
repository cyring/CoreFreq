%define MODULE corefreqk

Summary: CoreFreq
Name: corefreq
Version: 1.37
Release: 1
License: GPL2
URL: https://github.com/cyring/CoreFreq
Group: System
Packager: CyrIng
Requires: dkms
ExclusiveArch: x86_64

%description
CoreFreq is a CPU monitoring software designed for the 64-bits Processors,
including architectures Intel Atom, Core2, Xeon, Phi, Nehalem, Westmere,
SandyBridge, IvyBridge, Haswell, Broadwell, Skylake, Kaby Lake, Coffee Lake,
AMD K8, Zen

%prep
rm -fr %{name}-%{version}
git clone %{url}.git %{name}-%{version} && rm -fr %{name}-%{version}/.git

%build

%install
install -Dm 0644 %{name}-%{version}/Makefile \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/Makefile
install -Dm 0644 %{name}-%{version}/package/dkms.conf \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/package/dkms.conf
install -Dm 0755 %{name}-%{version}/package/scripter.sh \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/package/scripter.sh
install -m 0644 %{name}-%{version}/*.c %{name}-%{version}/*.h \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/
install -Dm 0644 %{name}-%{version}/package/corefreqd.service \
	${RPM_BUILD_ROOT}%{_libdir}/systemd/system/corefreqd.service

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%attr(0644, root, root) %{_prefix}/src/%{MODULE}-%{version}/Makefile
%attr(0644, root, root)%{_prefix}/src/%{MODULE}-%{version}/package/dkms.conf
%attr(0755, root, root)%{_prefix}/src/%{MODULE}-%{version}/package/scripter.sh
%attr(0644, root, root)%{_prefix}/src/%{MODULE}-%{version}/*.c
%attr(0644, root, root)%{_prefix}/src/%{MODULE}-%{version}/*.h
%attr(0644, root, root)%{_libdir}/systemd/system/corefreqd.service

%pre

%post
ln -s %{_prefix}/src/%{MODULE}-%{version}/package/dkms.conf \
	%{_prefix}/src/%{MODULE}-%{version}/dkms.conf
dkms add -q -m %{MODULE} -v %{version}
dkms build -q %{MODULE}/%{version}
dkms install -q %{MODULE}/%{version}

echo -e '\n[ \033[1;36;40mCoreFreq\033[0m Starting Instructions\033[0m ]\n'\
	'\n1: load the kernel module\n'\
	' # \033[1;36;40mmodprobe corefreqk\033[0m\n'\
	'\n2: start the daemon\n'\
	' # \033[1;36;40msystemctl start corefreqd\033[0m\n'\
	'\n3: run the client\n'\
	' $ \033[1;36;40mcorefreq-cli\033[0m\n'

%postun
if [ "$(dkms status -m %{MODULE})" ]; then
	modprobe -r %{MODULE} 2>/dev/null
	if [ $? -eq 0 ]; then
		echo "Module [%{MODULE}] unloaded.\n"
	fi

	dkms remove -q %{MODULE}/%{version} --all
fi

%changelog
* Thu Nov 8 2018 CyrIng <labs@cyring.fr>
- Initial RPM release
