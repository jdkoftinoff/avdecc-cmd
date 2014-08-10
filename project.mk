PROJECT=avdecc-cmd
PROJECT_NAME=avdecc-cmd
PROJECT_VERSION=20140808
PROJECT_EMAIL=jeffk@jdkoftinoff.com
PROJECT_LICENSE=private
PROJECT_MAINTAINER=jeffk@jdkoftinoff.com
PROJECT_COPYRIGHT=Copyright 2014 J.D. Koftinoff Software, Ltd.
PROJECT_DESCRIPTION=avdecc-cmd
PROJECT_WEBSITE=https://avb.statusbar.com/
PROJECT_IDENTIFIER=com.statusbar.avb.avdecc-cmd
TOP_LIB_DIRS+=. ../jdksavdecc-c

CONFIG_TOOLS+=
PKGCONFIG_PACKAGES+=

LDLIBS_MACOSX+=-lpcap
