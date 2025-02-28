# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

<<<<<<< HEAD
define Profile/Generic
  NAME:=Octeon SoC
endef

define Profile/Generic/Description
   Base packages for Octeon boards.
endef

$(eval $(call Profile,Generic))
=======
define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
endef

define Profile/Default/Description
   Base packages for Octeon boards.
endef

$(eval $(call Profile,Default))
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
