# Root Makefile for JEEVAN MARG - PBL Graphic Era University

all:
	$(MAKE) -C m1_dispatch
	$(MAKE) -C m2_healthcare
	$(MAKE) -C m3_navigation

server:
	$(MAKE) -C m3_navigation server

test:
	@echo "========================================="
	@echo " RUNNING ALL MODULE AUTOMATED TEST SUITES"
	@echo "========================================="
	$(MAKE) -C m1_dispatch test
	$(MAKE) -C m2_healthcare test
	$(MAKE) -C m3_navigation test

run:
	$(MAKE) -C m3_navigation run

serve:
	$(MAKE) -C m3_navigation serve

clean:
	$(MAKE) -C m1_dispatch clean
	$(MAKE) -C m2_healthcare clean
	$(MAKE) -C m3_navigation clean

.PHONY: all server test run serve clean
