########################################################################
# Pointwise - Proprietary software product of Pointwise, Inc.
#             Copyright (c) 1995-2012 Pointwise, Inc.
#             All rights reserved.
#
# module.mk for src\plugins\CaeUnsRecomb2DPoly plugin
########################################################################

########################################################################
########################################################################
#
#                   DO NOT EDIT THIS FILE
#
# To simplify SDK upgrades, the standard module.mk file should NOT be edited.
#
# If you want to modify a plugin's build process, you should rename
# modulelocal-sample.mk to modulelocal.mk and edit its settings.
#
# See the comments in modulelocal-sample.mk for more information.
#
#                   DO NOT EDIT THIS FILE
#
########################################################################
########################################################################

CaeUnsRecomb2DPoly_LOC := $(PLUGINS_LOC)/CaeUnsRecomb2DPoly
CaeUnsRecomb2DPoly_LIB := CaeUnsRecomb2DPoly$(DBG_SUFFIX)
CaeUnsRecomb2DPoly_CXX_LOC := $(CaeUnsRecomb2DPoly_LOC)
CaeUnsRecomb2DPoly_OBJ_LOC := $(PLUGINS_OBJ_LOC)/CaeUnsRecomb2DPoly

CaeUnsRecomb2DPoly_FULLNAME := lib$(CaeUnsRecomb2DPoly_LIB).$(SHLIB_SUFFIX)
CaeUnsRecomb2DPoly_FULLLIB := $(PLUGINS_DIST_DIR)/$(CaeUnsRecomb2DPoly_FULLNAME)

CaeUnsRecomb2DPoly_DEPS = \
	$(NULL)

MODCXXFILES := \
	runtimeWrite.cxx CaeUnsRecomb2DPoly.cxx \
	$(NULL)

# IMPORTANT:
# Must recompile the shared/XXX/.cxx files for each plugin. These .cxx files
# include the plugin specific settings defined in the ./CaeUnsRecomb2DPoly/*.h
# files.
CaeUnsRecomb2DPoly_SRC := \
	$(PLUGINS_RT_PWPSRC) \
	$(PLUGINS_RT_PWGMSRC) \
	$(PLUGINS_RT_CAEPSRC) \
	$(patsubst %,$(CaeUnsRecomb2DPoly_CXX_LOC)/%,$(MODCXXFILES))

CaeUnsRecomb2DPoly_SRC_CXX := $(filter %.cxx,$(MODCXXFILES))

# place the .o files generated from shared sources in the plugin's
# OBJ folder.
CaeUnsRecomb2DPoly_OBJ := \
	$(patsubst %.cxx,$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o,$(PLUGINS_RT_PWPFILES)) \
	$(patsubst %.cxx,$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o,$(PLUGINS_RT_PWGMFILES)) \
	$(patsubst %.cxx,$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o,$(PLUGINS_RT_CAEPFILES)) \
    $(patsubst %.cxx,$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o,$(CaeUnsRecomb2DPoly_SRC_CXX)) \
	$(NULL)

# To allow over-rides, search FIRST for headers in the local module's folder.
# For example, a site.h file in the local module's folder will preempt the
# file .../src/plugins/site.h
CaeUnsRecomb2DPoly_INCL = \
	-I$(CaeUnsRecomb2DPoly_LOC) \
	$(PLUGINS_RT_INCL) \
	$(NULL)

CaeUnsRecomb2DPoly_LIBS = \
	$(NULL)

CaeUnsRecomb2DPoly_MAINT_TARGETS := \
    CaeUnsRecomb2DPoly_info \
    CaeUnsRecomb2DPoly_install \
    CaeUnsRecomb2DPoly_installnow \
    CaeUnsRecomb2DPoly_uninstall


########################################################################
# Get (OPTIONAL) locally defined make targets. If a plugin developer wants
# to extend a plugin's make scheme, they should create a modulelocal.mk file
# in the plugin's base folder. To provide for future SDK upgrades, the standard
# module.mk file should NOT be directly edited.
#
ifneq ($(wildcard $(CaeUnsRecomb2DPoly_LOC)/modulelocal.mk),)
    CaeUnsRecomb2DPoly_DEPS += $(CaeUnsRecomb2DPoly_LOC)/modulelocal.mk
    include $(CaeUnsRecomb2DPoly_LOC)/modulelocal.mk
endif


# merge in plugin private settings

CaeUnsRecomb2DPoly_OBJ += \
    $(patsubst %.cxx,$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o,$(filter %.cxx,$(CaeUnsRecomb2DPoly_CXXFILES_PRIVATE))) \
	$(NULL)

CaeUnsRecomb2DPoly_SRC += \
	$(patsubst %,$(CaeUnsRecomb2DPoly_CXX_LOC)/%,$(CaeUnsRecomb2DPoly_CXXFILES_PRIVATE)) \
	$(NULL)

CaeUnsRecomb2DPoly_INCL += $(CaeUnsRecomb2DPoly_INCL_PRIVATE)
CaeUnsRecomb2DPoly_LIBS += $(CaeUnsRecomb2DPoly_LIBS_PRIVATE)
CaeUnsRecomb2DPoly_CXXFLAGS += $(CaeUnsRecomb2DPoly_CXXFLAGS_PRIVATE)
CaeUnsRecomb2DPoly_LDFLAGS += $(CaeUnsRecomb2DPoly_LDFLAGS_PRIVATE)
CaeUnsRecomb2DPoly_MAINT_TARGETS += $(CaeUnsRecomb2DPoly_MAINT_TARGETS_PRIVATE)
CaeUnsRecomb2DPoly_DEPS += $(CaeUnsRecomb2DPoly_DEPS_PRIVATE)

PLUGIN_MAINT_TARGETS += $(CaeUnsRecomb2DPoly_MAINT_TARGETS)
PLUGIN_OBJ += $(CaeUnsRecomb2DPoly_OBJ)

# add to plugin maint targets to the global .PHONY target
.PHONY: \
	$(CaeUnsRecomb2DPoly_MAINT_TARGETS) \
	$(NULL)


########################################################################
# Set the final build macros
CaeUnsRecomb2DPoly_CXXFLAGS += $(CXXFLAGS) $(PLUGINS_STDDEFS) $(CaeUnsRecomb2DPoly_INCL) \
    -DPWGM_HIDE_STRUCTURED_API


ifeq ($(machine),macosx)
CaeUnsRecomb2DPoly_LDFLAGS += -install_name "$(REL_BIN_TO_PW_LIB_DIR)/$(CaeUnsRecomb2DPoly_FULLNAME)"
else
CaeUnsRecomb2DPoly_LDFLAGS +=
endif


########################################################################
# list of plugin's build targets
#
CaeUnsRecomb2DPoly: $(CaeUnsRecomb2DPoly_FULLLIB)

$(CaeUnsRecomb2DPoly_FULLLIB): $(CaeUnsRecomb2DPoly_OBJ) $(CaeUnsRecomb2DPoly_DEPS)
	@echo "***"
	@echo "*** $@"
	@echo "***"
	@mkdir -p $(PLUGINS_DIST_DIR)
	$(SHLIB_LD) $(ARCH_FLAGS) $(CaeUnsRecomb2DPoly_LDFLAGS) -o $(CaeUnsRecomb2DPoly_FULLLIB) $(CaeUnsRecomb2DPoly_OBJ) $(CaeUnsRecomb2DPoly_LIBS) $(SYS_LIBS)

CaeUnsRecomb2DPoly_info:
	@echo ""
	@echo "--------------------------------------------------------------"
ifeq ($(machine),macosx)
	otool -L -arch all $(CaeUnsRecomb2DPoly_FULLLIB)
	@echo ""
endif
	file $(CaeUnsRecomb2DPoly_FULLLIB)
	@echo "--------------------------------------------------------------"
	@echo ""


########################################################################
# list of plugin's intermediate targets
#
$(CaeUnsRecomb2DPoly_OBJ_LOC):
	mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)

#.......................................................................
# build .d files for the plugin and each of the shared runtime sources
# the .d files will be placed in the plugins OBJ folder CaeUnsRecomb2DPoly_OBJ_LOC
$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.d: $(CaeUnsRecomb2DPoly_CXX_LOC)/%.cxx
	@echo "Updating dependencies $(CaeUnsRecomb2DPoly_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	@./depend.sh $(CaeUnsRecomb2DPoly_OBJ_LOC) $(CaeUnsRecomb2DPoly_CXXFLAGS) $< > $@

$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.d: $(PLUGINS_RT_PWPLOC)/%.cxx
	@echo "Updating dependencies $(CaeUnsRecomb2DPoly_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	@./depend.sh $(CaeUnsRecomb2DPoly_OBJ_LOC) $(CaeUnsRecomb2DPoly_CXXFLAGS) $< > $@

$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.d: $(PLUGINS_RT_PWGMLOC)/%.cxx
	@echo "Updating dependencies $(CaeUnsRecomb2DPoly_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	@./depend.sh $(CaeUnsRecomb2DPoly_OBJ_LOC) $(CaeUnsRecomb2DPoly_CXXFLAGS) $< > $@

$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.d: $(PLUGINS_RT_CAEPLOC)/%.cxx
	@echo "Updating dependencies $(CaeUnsRecomb2DPoly_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	@./depend.sh $(CaeUnsRecomb2DPoly_OBJ_LOC) $(CaeUnsRecomb2DPoly_CXXFLAGS) $< > $@

#.......................................................................
# build .o files for the plugin and each of the shared runtime sources.
# the .o files will be placed in the plugins OBJ folder CaeUnsRecomb2DPoly_OBJ_LOC
$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o: $(CaeUnsRecomb2DPoly_CXX_LOC)/%.cxx
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	$(CXX) $(CaeUnsRecomb2DPoly_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<

$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o: $(PLUGINS_RT_PWPLOC)/%.cxx
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	$(CXX) $(CaeUnsRecomb2DPoly_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<

$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o: $(PLUGINS_RT_PWGMLOC)/%.cxx
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	$(CXX) $(CaeUnsRecomb2DPoly_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<

$(CaeUnsRecomb2DPoly_OBJ_LOC)/%.o: $(PLUGINS_RT_CAEPLOC)/%.cxx
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsRecomb2DPoly_OBJ_LOC)
	$(CXX) $(CaeUnsRecomb2DPoly_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<


########################################################################
# list of plugin's clean targets
#
CaeUnsRecomb2DPoly_cleandep:
	-$(RMR) $(CaeUnsRecomb2DPoly_OBJ_LOC)/*.d

CaeUnsRecomb2DPoly_clean:
	-$(RMR) $(CaeUnsRecomb2DPoly_OBJ_LOC)/*.{d,o}

CaeUnsRecomb2DPoly_distclean: CaeUnsRecomb2DPoly_clean
	-$(RMF) $(CaeUnsRecomb2DPoly_FULLLIB) > /dev/null 2>&1

########################################################################
# list of plugin's clean targets
#
CaeUnsRecomb2DPoly_install: install_validate CaeUnsRecomb2DPoly_installnow
	@echo "CaeUnsRecomb2DPoly Installed to '$(PLUGIN_INSTALL_FULLPATH)'"

CaeUnsRecomb2DPoly_installnow:
	-@$(CP) $(CaeUnsRecomb2DPoly_FULLLIB) "$(PLUGIN_INSTALL_FULLPATH)/libCaeUnsRecomb2DPoly.$(SHLIB_SUFFIX)"

CaeUnsRecomb2DPoly_uninstall:
	@echo "CaeUnsRecomb2DPoly Uninstalled from '$(PLUGIN_INSTALL_FULLPATH)'"
	-@$(RMF) "$(PLUGIN_INSTALL_FULLPATH)/libCaeUnsRecomb2DPoly.$(SHLIB_SUFFIX)"