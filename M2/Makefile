CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

ifeq ($(OS),Windows_NT)
  ifneq ($(findstring /,$(SHELL)),)
    IS_UNIX = 1
  else ifneq ($(MSYSTEM),)
    IS_UNIX = 1
  else
    IS_UNIX = 0
  endif
else
  IS_UNIX = 1
endif

ifeq ($(IS_UNIX),1)
  CLEAN_CMD = rm -f *.exe *.o
  RUN_CMD = ./$(1)
else
  CLEAN_CMD = -del /f /q *.exe *.o 2>nul
  RUN_CMD = $(subst /,\,$(1))
endif

SRC = main.cpp \
      model/BedAvailability.cpp \
      model/Doctor.cpp \
      model/Department.cpp \
      model/Hospital.cpp \
      repository/HospitalRepository.cpp \
      service/HealthcareService.cpp

TEST_SRC = test/test_m2.cpp \
           model/BedAvailability.cpp \
           model/Doctor.cpp \
           model/Department.cpp \
           model/Hospital.cpp \
           repository/HospitalRepository.cpp \
           service/HealthcareService.cpp

all: m2_healthcare.exe

m2_healthcare.exe: $(SRC)
	$(CXX) $(CXXFLAGS) -I. $(SRC) -o m2_healthcare.exe

test: m2_test.exe
	$(call RUN_CMD,m2_test.exe)

m2_test.exe: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -I. $(TEST_SRC) -o m2_test.exe

clean:
	$(CLEAN_CMD)

.PHONY: all test clean
