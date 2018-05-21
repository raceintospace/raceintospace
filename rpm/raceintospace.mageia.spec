%define git 20121224
%define rel 0
%define gitrel 0

Name:		raceintospace
Version:	1.0
%if %{git}
Source0:         %{name}-%{git}.tar.bz2
%else
Source0:		http://sourceforge.net/projects/raceintospace/files/Race Into Space/1.0/raceintospace-1.0.tar.gz
%endif
Source1:	make-git-snapshot.sh
%if %{git}
Release:        %mkrel %{rel}.git%{git}.%{gitrel}
%else
Release:	%mkrel %{rel}
%endif
Summary:	Race into space strategy game
License:	GPLV2+
Group:		Games/Strategy
Url:		http://www.raceintospace.org/
Patch0:		raceintospace-linux-build.patch
Patch1:		raceintospace-physfs-2.0.patch
Patch2:		raceintospace-jsoncpp.patch
Source2: raceintospace-music.tar.bz2

BuildRequires: SDL-devel
BuildRequires: cmake
BuildRequires: boost-devel
BuildRequires: png-devel
BuildRequires: libogg-devel
BuildRequires: libvorbis-devel
#BuildRequires: protobuf-devel
BuildRequires: libtheora-devel
BuildRequires: physfs-devel
BuildRequires: jsoncpp-devel

%description
Race Into Space a free software version of the Liftoff! board game by
Fritz Bronner, originally developed by Strategic Visions and published
by Interplay in 1993 and 1994 as Buzz Aldrin's Race Into Space.

%prep
%if %{git}
%setup0 -q -n raceintospace-%{git}
%else
%setup0 -q -n raceintospace-1.0release
%endif
%apply_patches
%setup -q -T -D -c -a 2 -n raceintospace-%{git}

%build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=%{_prefix} \
-DCMAKE_BUILD_TYPE=RelWithDebInfo
%make

%install
cd build
DESTDIR=%{buildroot} cmake -P cmake_install.cmake

%files
%{_bindir}/raceintospace
%{_datadir}/raceintospace



