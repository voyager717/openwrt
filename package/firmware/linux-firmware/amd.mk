<<<<<<< HEAD
Package/amd64-microcode = $(call Package/firmware-default,AMD64 CPU microcode,,LICENSE.amd-ucode)
=======
Package/amd64-microcode = $(call Package/firmware-default,AMD64 CPU microcode,@TARGET_x86)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Package/amd64-microcode/install
	$(INSTALL_DIR) $(1)/lib/firmware/amd-ucode
	$(CP) \
		$(PKG_BUILD_DIR)/amd-ucode/*.bin \
		$(1)/lib/firmware/amd-ucode
endef

$(eval $(call BuildPackage,amd64-microcode))

<<<<<<< HEAD
Package/amdgpu-firmware = $(call Package/firmware-default,AMDGPU Video Driver firmware,,LICENSE.amdgpura)
=======
Package/amdgpu-firmware = $(call Package/firmware-default,AMDGPU Video Driver firmware)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Package/amdgpu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/amdgpu
	$(CP) \
		$(PKG_BUILD_DIR)/amdgpu/*.bin \
		$(1)/lib/firmware/amdgpu
endef

$(eval $(call BuildPackage,amdgpu-firmware))

<<<<<<< HEAD
Package/radeon-firmware = $(call Package/firmware-default,Radeon Video Driver firmware,,LICENSE.radeon)
=======
Package/radeon-firmware = $(call Package/firmware-default,Radeon Video Driver firmware)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
define Package/radeon-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/radeon
	$(CP) \
		$(PKG_BUILD_DIR)/radeon/*.bin \
		$(1)/lib/firmware/radeon
endef

$(eval $(call BuildPackage,radeon-firmware))
