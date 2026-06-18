export CONFIG:=Release

PKG_CONFIG:=$(shell which pkg-config)

# NetworkManager/libnm-glib dropped: wifi status now goes through nmcli and
# config through nmtui (see Source/WifiStatusNM.cpp), so we no longer link NM.
PKG_CONFIG_PACKAGES = \
	alsa \

export PKG_CONFIG_CFLAGS=$(foreach pkg, $(PKG_CONFIG_PACKAGES), $(shell $(PKG_CONFIG) --cflags $(pkg)))
# -li2c: the i2c_smbus_* helpers moved out of <linux/i2c-dev.h> into libi2c
# (modern i2c-tools); it has no pkg-config file, so append it directly.
export PKG_CONFIG_LDFLAGS=$(foreach pkg, $(PKG_CONFIG_PACKAGES), $(shell $(PKG_CONFIG) --libs $(pkg))) -li2c



all:
	cd Builds/LinuxMakefile && $(MAKE)

clean:
	cd Builds/LinuxMakefile && $(MAKE) clean

wifitest:
	cd Builds/LinuxMakefile && $(MAKE) -f UnitTests.mk ../../build/$(CONFIG)/wifitest

devinstall:
	killall pocket-home ;\
	sudo cp build/$(CONFIG)/pocket-home /usr/bin/pocket-home && \
	sudo cp -R assets/* /usr/share/pocket-home
