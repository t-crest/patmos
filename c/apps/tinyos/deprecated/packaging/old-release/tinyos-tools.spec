%define debug_package %{nil}

Summary: TinyOS tools 
Name: tinyos-tools
Version: 1.4.2
Release: 20120807
License: Please see source
Group: Development/System
URL: http://www.tinyos.net/
BuildRoot: %{_tmppath}/%{name}-root
Source0: %{name}-%{version}.tar.gz
# This makes cygwin happy
Provides: /bin/sh /bin/bash
Requires: nesc >= 1.3.4

%description
Tools for use with tinyos. Includes, for example: uisp, motelist, pybsl, mig,
ncc and nesdoc. The source for these tools is found in the TinyOS CVS
repository under tinyos-2.x/tools.

%prep
%setup -q -n %{name}-%{version}

%build
cd tools
./Bootstrap
NESCC_PREFIX=/usr TOSDIR=/opt/tinyos-2.x/tos ./configure --prefix=/usr
make

%install
rm -rf %{buildroot}
cd tools
make install prefix=%{buildroot}/usr

%clean
rm -rf $RPM_BUILD_DIR/%{name}-%{version}
rm -rf $RPM_SOURCE_DIR/%{name}-%{version}

%files
%defattr(-,root,root,-)
/usr/
%attr(4755, root, root) /usr/bin/uisp*

%post
if [ -z "$RPM_INSTALL_PREFIX" ]; then
  RPM_INSTALL_PREFIX=/usr
fi

# Install giveio (windows only)
if [ -f $RPM_INSTALL_PREFIX/lib/tinyos/giveio-install ]; then
  (cd $RPM_INSTALL_PREFIX/lib/tinyos; ./giveio-install --install)
fi
# Install the JNI;  we can't call tos-install-jni 
# directly because it isn't in the path yet. Stick
# a temporary script in /etc/profile.d and then delete.
jni=`$RPM_INSTALL_PREFIX/bin/tos-locate-jre --jni`
if [ $? -ne 0 ]; then
  echo "Java not found, not installing JNI code"
  exit 0
fi
%ifos linux
java=`$RPM_INSTALL_PREFIX/bin/tos-locate-jre --java`
tinyoslibdir=$RPM_INSTALL_PREFIX/lib/tinyos
bits=32
if [ $? -ne 0 ]; then
  echo "java command not found - assuming 32 bits"
elif file -L $java/java | grep -q 64-bit; then
  bits=64
fi
echo "Installing $bits-bit Java JNI code in $jni ... "
for lib in $tinyoslibdir/*-$bits.so; do 
  realname=`basename $lib | sed -e s/-$bits\.so/.so/`
  install $lib "$jni/$realname" || exit 1
done
%else
echo "Installing Java JNI code in $jni ... "
for lib in $RPM_INSTALL_PREFIX/lib/tinyos/*.dll; do 
  install --group=SYSTEM $lib "$jni" || exit 0
done
%endif
echo "done."

%preun
# Remove JNI code on uninstall

%changelog
* Sun Jul 29 2007 <pal@cs.stanford.edu> 1.2.4-2
- Add 64-bit support for Java VMs and deluge tools.
* Wed Jul 5 2006 <kwright@archrock.com> 1.2.2-1
* Thu Feb 9 2006 <david.e.gay@intel.com> 1.2.1-2
* Sat Feb 4 2006 <kwright@cs.berkeley.edu> 1.2.1-1
- 1.2.1
* Wed Aug 26 2005 <kwright@cs.berkeley.edu> 1.2.0-beta2.1
- includes dgay fixes for uisp and calling tos-locate-jre from post script
* Wed Aug 17 2005 <kwright@cs.berkeley.edu> 1.2.0-internal2.1
- include fixes/improvements to tos-locate-jre and switch prefix to /usr
* Fri Aug 12 2005  <kwright@cs.berkeley.edu> 1.2.0-internal1.1
- 1.2
* Wed Sep  3 2003  <dgay@barnowl.research.intel-research.net> 1.1.0-internal2.1
- All tools, no java
* Sun Aug 31 2003 root <kwright@cs.berkeley.edu> 1.1.0-internal1.1
- Initial build.
