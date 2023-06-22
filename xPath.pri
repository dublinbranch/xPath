DISTFILES += \
	$$PWD/README.md 
	
HEADERS += \
	$$PWD/xml.h
	
SOURCES += \
	$$PWD/xml.cpp
	
LIBS += -lxml2
LIBS += -ltidy #zypper in libtidy-devel
