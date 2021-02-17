#
# Copyright (c) 2021 Sung Ho Park and CSOS
# 
# SPDX-License-Identifier: Apache-2.0
#

###############################################################################

config configd clean cleand:
	@echo ""
	@echo ""
	

	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo ""
	@echo ""
	make -f makefile.mk $@ CONFIG_DIR=../app CONFIG_NAME=esp8266at_tester_nucleof207zg_rexternal_build
	@echo ""
	@echo ""


	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo ""

%:
	@echo "Nothing to do"


###############################################################################

