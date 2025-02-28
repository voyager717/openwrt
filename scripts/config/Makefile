# SPDX-License-Identifier: GPL-2.0-only
# ===========================================================================
# OpenWrt configuration targets

.PHONY: clean all
all: conf mconf
clean:
<<<<<<< HEAD
	rm -f $(clean-files) $(hostprogs)

clean-files	:= *.o lxdialog/*.o *.moc qconf-moc.cc \
		   *conf-cfg # <- This should be removed after 23.05 is EOL
=======
	rm -f *.o lxdialog/*.o *.moc $(clean-files) conf mconf qconf nconf

# This clean-files definition is here to ensure that temporary files from the
# previous version are removed by make config-clean.
# It should be removed or emptied when this Makefile get updated again.
clean-files	:= zconf.tab.c zconf.lex.c zconf.hash.c .tmp_qtcheck
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

# ===========================================================================
# Variables needed by the upstream Makefile

<<<<<<< HEAD
export HOSTPKG_CONFIG=pkg-config
CONFIG_SHELL:=$(SHELL)
src:=$(CURDIR)
obj:=.
Q:=$(if $V,,@)
quiet:=$(if $V,,_silent)
include Kbuild.include
=======
# Avoids displaying 'UPD mconf-cfg' in an otherwise quiet make menuconfig
kecho:=true

CONFIG_SHELL:=$(SHELL)
srctree:=.
src:=.
obj:=.
Q:=$(if $V,,@)
cmd = $(cmd_$(1))
dot-target = $(dir $@).$(notdir $@)

# taken from ../Kbuild.include
define filechk
	$(Q)set -e;						\
	mkdir -p $(dir $@);					\
	trap "rm -f $(dot-target).tmp" EXIT;			\
	{ $(filechk_$(1)); } > $(dot-target).tmp;		\
	if [ ! -r $@ ] || ! cmp -s $@ $(dot-target).tmp; then	\
		$(kecho) '  UPD     $@';			\
		mv -f $(dot-target).tmp $@;			\
	fi
endef
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

### Stripped down upstream Makefile follows:
# ===========================================================================
# object files used by all kconfig flavours
<<<<<<< HEAD
common-objs	:= confdata.o expr.o lexer.lex.o menu.o parser.tab.o \
		   preprocess.o symbol.o util.o
=======
common-objs	:= confdata.o expr.o lexer.lex.o parser.tab.o preprocess.o \
		   symbol.o util.o
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

$(obj)/lexer.lex.o: $(obj)/parser.tab.h
HOSTCFLAGS_lexer.lex.o	:= -I $(srctree)/$(src)
HOSTCFLAGS_parser.tab.o	:= -I $(srctree)/$(src)

# conf: Used for defconfig, oldconfig and related targets
<<<<<<< HEAD
hostprogs	+= conf
conf-objs	:= conf.o $(common-objs)

# nconf: Used for the nconfig target based on ncurses
hostprogs	+= nconf
nconf-objs	:= nconf.o nconf.gui.o $(common-objs)

HOSTLDLIBS_nconf       = $(call read-file, $(obj)/nconf-libs)
HOSTCFLAGS_nconf.o     = $(call read-file, $(obj)/nconf-cflags)
HOSTCFLAGS_nconf.gui.o = $(call read-file, $(obj)/nconf-cflags)

$(obj)/nconf: | $(obj)/nconf-libs
$(obj)/nconf.o $(obj)/nconf.gui.o: | $(obj)/nconf-cflags

# mconf: Used for the menuconfig target based on lxdialog
hostprogs	+= mconf
=======
hostprogs-y	+= conf
conf-objs	:= conf.o $(common-objs)

# nconf: Used for the nconfig target based on ncurses
hostprogs-y	+= nconf
nconf-objs	:= nconf.o nconf.gui.o $(common-objs)

HOSTLDLIBS_nconf	= $(shell . $(obj)/nconf-cfg && echo $$libs)
HOSTCFLAGS_nconf.o	= $(shell . $(obj)/nconf-cfg && echo $$cflags)
HOSTCFLAGS_nconf.gui.o	= $(shell . $(obj)/nconf-cfg && echo $$cflags)

$(obj)/nconf.o $(obj)/nconf.gui.o: $(obj)/nconf-cfg

# mconf: Used for the menuconfig target based on lxdialog
hostprogs-y	+= mconf
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
lxdialog	:= $(addprefix lxdialog/, \
		     checklist.o inputbox.o menubox.o textbox.o util.o yesno.o)
mconf-objs	:= mconf.o $(lxdialog) $(common-objs)

<<<<<<< HEAD
HOSTLDLIBS_mconf = $(call read-file, $(obj)/mconf-libs)
$(foreach f, mconf.o $(lxdialog), \
  $(eval HOSTCFLAGS_$f = $$(call read-file, $(obj)/mconf-cflags)))

$(obj)/mconf: | $(obj)/mconf-libs
$(addprefix $(obj)/, mconf.o $(lxdialog)): | $(obj)/mconf-cflags

# qconf: Used for the xconfig target based on Qt
hostprogs	+= qconf
qconf-cxxobjs	:= qconf.o qconf-moc.o
qconf-objs	:= images.o $(common-objs)

HOSTLDLIBS_qconf         = $(call read-file, $(obj)/qconf-libs)
HOSTCXXFLAGS_qconf.o     = -std=c++11 -fPIC $(call read-file, $(obj)/qconf-cflags)
HOSTCXXFLAGS_qconf-moc.o = -std=c++11 -fPIC $(call read-file, $(obj)/qconf-cflags)
$(obj)/qconf: | $(obj)/qconf-libs
$(obj)/qconf.o $(obj)/qconf-moc.o: | $(obj)/qconf-cflags

quiet_cmd_moc = MOC     $@
      cmd_moc = $(call read-file, $(obj)/qconf-bin)/moc $< -o $@

$(obj)/qconf-moc.cc: $(src)/qconf.h FORCE | $(obj)/qconf-bin
	$(call if_changed,moc)

targets += qconf-moc.cc

# check if necessary packages are available, and configure build flags
cmd_conf_cfg = $< $(addprefix $(obj)/$*conf-, cflags libs bin); touch $(obj)/$*conf-bin

$(obj)/%conf-cflags $(obj)/%conf-libs $(obj)/%conf-bin: $(src)/%conf-cfg.sh
	$(call cmd,conf_cfg)

clean-files += *conf-cflags *conf-libs *conf-bin
=======
HOSTLDLIBS_mconf = $(shell . $(obj)/mconf-cfg && echo $$libs)
$(foreach f, mconf.o $(lxdialog), \
  $(eval HOSTCFLAGS_$f = $$(shell . $(obj)/mconf-cfg && echo $$$$cflags)))

$(addprefix $(obj)/, mconf.o $(lxdialog)): $(obj)/mconf-cfg

# qconf: Used for the xconfig target based on Qt
hostprogs-y	+= qconf
qconf-cxxobjs	:= qconf.o
qconf-objs	:= images.o $(common-objs)

HOSTLDLIBS_qconf	= $(shell . $(obj)/qconf-cfg && echo $$libs)
HOSTCXXFLAGS_qconf.o	= $(shell . $(obj)/qconf-cfg && echo $$cflags)

$(obj)/qconf.o: $(obj)/qconf-cfg $(obj)/qconf.moc

quiet_cmd_moc = MOC     $@
      cmd_moc = $(shell . $(obj)/qconf-cfg && echo $$moc) -i $< -o $@

$(obj)/%.moc: $(src)/%.h $(obj)/qconf-cfg
	$(call cmd,moc)

# check if necessary packages are available, and configure build flags
filechk_conf_cfg = $(CONFIG_SHELL) $<

$(obj)/%conf-cfg: $(src)/%conf-cfg.sh FORCE
	$(call filechk,conf_cfg)

clean-files += *conf-cfg
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

# ===========================================================================
# OpenWrt rules and final adjustments that need to be made after reading the
# full upstream Makefile

FORCE:

ifdef BUILD_SHIPPED_FILES
shipped-files := lexer.lex.c parser.tab.c parser.tab.h
clean-files += $(shipped-files)

.SECONDARY: $(shipped-files)

%.tab.c %.tab.h: %.y
	bison -l -d -b $* $<

%.lex.c: %.l
	flex -L -o$@ $<
endif

<<<<<<< HEAD
define link_rule
$(1): LDLIBS+=$$(HOSTLDLIBS_$(1))
$(1): $($(1)-objs) $$($(1)-cxxobjs)
$(if $($(1)-cxxobjs),	$(CXX) $$(LDFLAGS) -o $$@ $$^ $$(LDLIBS))
all-objs += $($(1)-objs)
all-cxxobjs += $($(1)-cxxobjs)
endef

all-objs:=
all-cxxobjs:=
$(foreach f,$(hostprogs),$(eval $(call link_rule,$f)))


$(foreach f,$(sort $(all-objs)), \
  $(eval $f: CFLAGS+=$$(HOSTCFLAGS_$f)))

$(foreach f,$(sort $(all-cxxobjs)), \
  $(eval $f: CXXFLAGS+=$$(HOSTCXXFLAGS_$f)))
=======
$(foreach f, mconf.o $(lxdialog), \
  $(eval $f: CFLAGS+=$$(HOSTCFLAGS_$f)))

$(obj)/lexer.lex.o: CFLAGS += $(HOSTCFLAGS_lexer.lex.o)
$(obj)/parser.tab.o: CFLAGS += $(HOSTCFLAGS_parser.tab.o)
$(obj)/qconf.o: CXXFLAGS+=$(HOSTCXXFLAGS_qconf.o)

conf: $(conf-objs)

# The *conf-cfg file is used (then filtered out) as the first prerequisite to
# avoid sourcing it before the script is built, when trying to compute CFLAGS
# for the actual first prerequisite.  This avoids errors like:
# '/bin/sh: ./mconf-cfg: No such file or directory'
mconf: mconf-cfg $(mconf-objs)
	$(CC) -o $@ $(filter-out mconf-cfg,$^) $(HOSTLDLIBS_mconf)

nconf: nconf-cfg $(nconf-objs)
	$(CC) -o $@ $(filter-out nconf-cfg,$^) $(HOSTLDLIBS_nconf)

qconf: qconf-cfg $(qconf-cxxobjs) $(qconf-objs)
	$(CXX) -o $@ $(filter-out qconf-cfg,$^) $(HOSTLDLIBS_qconf)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
