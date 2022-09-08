include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt_sub
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/mqtt_sub
	DEPENDS:=+libmosquitto +libsqlite3 +libuci +libjson-c +libargp
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=MQTT Subscriber service
endef

define Package/mqtt_sub/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/config $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt_sub $(1)/usr/bin
	$(INSTALL_BIN) ./files/mqtt_sub.init $(1)/etc/init.d/mqtt_sub
	$(INSTALL_CONF) ./files/mqtt_sub.config $(1)/etc/config/mqtt_sub
endef

$(eval $(call BuildPackage,mqtt_sub))