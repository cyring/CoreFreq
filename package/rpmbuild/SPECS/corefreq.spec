%define MODULE corefreqk
%define SERVICE corefreqd.service

Summary: CoreFreq
Name: corefreq
Version: 1.37
Release: 0.1
License: GPL2
URL: https://github.com/cyring/CoreFreq
Group: System
Packager: CyrIng
Requires: dkms
BuildRequires: dkms
ExclusiveArch: x86_64

%description
CoreFreq is a CPU monitoring software designed for the 64-bits Processors,
including architectures Intel Atom, Core2, Xeon, Phi, Nehalem, Westmere,
SandyBridge, IvyBridge, Haswell, Broadwell, Skylake, Kaby Lake, Coffee Lake,
AMD K8, Zen

%prep
rm -fr %{name}-%{version}
git clone %{url}.git %{name}-%{version} && rm -fr %{name}-%{version}/.git

%install
install -Dm 0644 %{name}-%{version}/package/dkms.conf \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/package/dkms.conf
ln -sr ${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/package/dkms.conf \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/dkms.conf
install -Dm 0755 %{name}-%{version}/package/scripter.sh \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/package/scripter.sh
install -Dm 0644 %{name}-%{version}/Makefile \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/Makefile
install -m 0644 %{name}-%{version}/*.c %{name}-%{version}/*.h \
	${RPM_BUILD_ROOT}%{_prefix}/src/%{MODULE}-%{version}/
install -Dm 0644 %{name}-%{version}/package/%{SERVICE} \
	${RPM_BUILD_ROOT}%{_prefix}/lib/systemd/system/%{SERVICE}

%clean
if [ "${RPM_BUILD_ROOT}" != "/" ]; then
	rm -rf ${RPM_BUILD_ROOT}
fi

%files
%defattr(-,root,root)
%attr(0755,root,root) %dir %{_prefix}/src/%{MODULE}-%{version}
%attr(0755,root,root) %dir %{_prefix}/src/%{MODULE}-%{version}/package
%attr(0644,root,root) %{_prefix}/src/%{MODULE}-%{version}/package/dkms.conf
%attr(-,root,root) %{_prefix}/src/%{MODULE}-%{version}/dkms.conf
%attr(0755,root,root) %{_prefix}/src/%{MODULE}-%{version}/package/scripter.sh
%attr(0644,root,root) %{_prefix}/src/%{MODULE}-%{version}/Makefile
%attr(0644,root,root) %{_prefix}/src/%{MODULE}-%{version}/*.c
%attr(0644,root,root) %{_prefix}/src/%{MODULE}-%{version}/*.h
%attr(0644,root,root) %{_prefix}/lib/systemd/system/%{SERVICE}

%post
dkms add -q -m %{MODULE} -v %{version} --rpm_safe_upgrade \
&& dkms build -q %{MODULE}/%{version} \
&& dkms install -q %{MODULE}/%{version} \
&& echo -e '\n[ \033[1;36;40mCoreFreq\033[0m Starting Instructions\033[0m ]\n'\
	'\n1: load the kernel module\n'\
	' # \033[1;36;40mmodprobe corefreqk\033[0m\n'\
	'\n2: start the daemon\n'\
	' # \033[1;36;40msystemctl start corefreqd\033[0m\n'\
	'\n3: run the client\n'\
	' $ \033[1;36;40mcorefreq-cli\033[0m\n'
exit $?

%preun
systemctl status %{SERVICE} 1>/dev/null 2>&1
if [ $? -eq 0 ]; then
	systemctl stop %{SERVICE} \
	|| systemctl disable %{SERVICE} \
	&& echo "Service [%{SERVICE}] uninstalled."
fi
lsmod 2>/dev/null | grep %{MODULE} 1>/dev/null 2>&1
if [ $? -eq 0 ]; then
	modprobe -r %{MODULE} 2>/dev/null \
	&& echo "Module [%{MODULE}] unloaded."
fi
dkms remove -q %{MODULE}/%{version} --all --rpm_safe_upgrade
exit $?

%changelog
* Fri Nov 9 2018 CyrIng <labs@cyring.fr>
- Initial RPM release
