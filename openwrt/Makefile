# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=relay-serial-com
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/lixingcong/relay-serial-com/releases/download/v$(PKG_VERSION)
PKG_MD5SUM:=91b8a1f203858aee0d5b9380a65d0b23

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(BUILD_VARIANT)/$(PKG_NAME)

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/relay-serial-com/Default
	SECTION:=net
	CATEGORY:=Network
	TITLE:=packet relay utils
	URL:=https://github.com/lixingcong/relay-serial-com
	DEPENDS:= +bluez-libs
endef

Package/relay-serial-com = $(call Package/relay-serial-com/Default)

define Package/relay-serial-com/description
redir traffic amond bluetooth, serial and TCP
endef

# turn on or off modules
CONFIGURE_ARGS += --enable-bluetooth=yes --enable-serial=yes
CONFIGURE_ARGS += --disable-ssp

define Package/relay-serial-com/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/main $(1)/usr/bin/relay-serial-com
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/phone_{recv,send} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/serial_{recv,send} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/blue_{recv,send} $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.{a,so*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,relay-serial-com))
