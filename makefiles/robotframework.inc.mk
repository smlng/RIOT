RFBASE ?= $(RIOTBASE)/dist/robotframework
RFPYPATH ?= $(APPDIR)/tests:$(RFBASE)/lib:$(RFBASE)/res:$(RIOTBASE)/dist/tests:$(RIOTBASE)/dist/pythonlibs/
RFOUTPATH ?= $(RIOTBUILDOUT)/robot/$(BOARD)/$(APPLICATION)

ROBOT_FILES ?= $(wildcard tests/*.robot)

robot-test:
ifneq (,$(ROBOT_FILES))
	robot --noncritical warn-if-failed -P "$(RFPYPATH)" -d $(RFOUTPATH) $(ROBOT_FILES)
else
	@echo No test scripts found, no worries.
endif
