CXX = g++
CC  = gcc
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -I../m1_dispatch
LDFLAGS = -lws2_32 -lwsock32 -lbcrypt -static-libgcc -static-libstdc++

BUILD_DIR = build
TARGET = $(BUILD_DIR)/m3_nav.exe
TEST_TARGET = $(BUILD_DIR)/m3_test.exe
TEST_DB_TARGET = $(BUILD_DIR)/m3_db_test.exe
TEST_DISPATCH_TARGET = $(BUILD_DIR)/m3_dispatch_test.exe
TEST_AUTH_TARGET = $(BUILD_DIR)/m3_auth_test.exe
TEST_PORTALS_TARGET = $(BUILD_DIR)/m3_portals_test.exe
TEST_RELIABILITY_TARGET = $(BUILD_DIR)/m3_reliability_test.exe
TEST_SECURITY_TARGET = $(BUILD_DIR)/m3_security_test.exe

SQLITE_OBJ   = $(BUILD_DIR)/sqlite3.o
SQLITE_FLAGS = -DSQLITE_THREADSAFE=1 -DSQLITE_ENABLE_FTS5

PERSISTENCE_SRC = \
    persistence/Database.cpp \
    persistence/SqliteLocationRepository.cpp \
    persistence/SqliteRoadRepository.cpp \
    persistence/SqliteHospitalRepository.cpp \
    persistence/SqliteAmbulanceRepository.cpp \
    persistence/SqliteEmergencyRepository.cpp \
    persistence/SqlitePatientLocationRepository.cpp \
    persistence/SqliteUserRepository.cpp \
    persistence/SqliteSessionRepository.cpp \
    persistence/DatabaseBootstrap.cpp

DISPATCH_SRC = \
    service/EmergencyDispatcher.cpp \
    ../m1_dispatch/service/TriageRuleEngine.cpp

AUTH_SRC = \
    util/CryptoUtil.cpp \
    service/AuthService.cpp

SRC = main.cpp \
      model/Location.cpp \
      structure/LocationNode.cpp \
      structure/RoadEdge.cpp \
      structure/RoadGraph.cpp \
      repository/LocationRepository.cpp \
      datasource/LocationCsvLoader.cpp \
      datasource/RoadNetworkCsvLoader.cpp \
      dto/RouteResponse.cpp \
      service/RoadGraphBuilder.cpp \
      service/ShortestPathFinder.cpp \
      service/NearbyServicesFinder.cpp \
      service/TransportEstimator.cpp \
      service/RestApiServer.cpp \
      util/GeoUtil.cpp \
      $(PERSISTENCE_SRC) \
      $(DISPATCH_SRC) \
      $(AUTH_SRC)

TEST_SRC = test/test_main.cpp \
           model/Location.cpp \
           structure/LocationNode.cpp \
           structure/RoadEdge.cpp \
           structure/RoadGraph.cpp \
           repository/LocationRepository.cpp \
           datasource/LocationCsvLoader.cpp \
           datasource/RoadNetworkCsvLoader.cpp \
           dto/RouteResponse.cpp \
           service/RoadGraphBuilder.cpp \
           service/ShortestPathFinder.cpp \
           service/NearbyServicesFinder.cpp \
           service/TransportEstimator.cpp \
           util/GeoUtil.cpp

TEST_DB_SRC = test/test_db.cpp \
              model/Location.cpp \
              structure/LocationNode.cpp \
              structure/RoadEdge.cpp \
              structure/RoadGraph.cpp \
              repository/LocationRepository.cpp \
              datasource/LocationCsvLoader.cpp \
              datasource/RoadNetworkCsvLoader.cpp \
              dto/RouteResponse.cpp \
              service/RoadGraphBuilder.cpp \
              service/ShortestPathFinder.cpp \
              util/GeoUtil.cpp \
              util/CryptoUtil.cpp \
              $(PERSISTENCE_SRC)

TEST_DISPATCH_SRC = test/test_dispatch.cpp \
                    model/Location.cpp \
                    structure/LocationNode.cpp \
                    structure/RoadEdge.cpp \
                    structure/RoadGraph.cpp \
                    repository/LocationRepository.cpp \
                    datasource/LocationCsvLoader.cpp \
                    datasource/RoadNetworkCsvLoader.cpp \
                    dto/RouteResponse.cpp \
                    service/RoadGraphBuilder.cpp \
                    service/ShortestPathFinder.cpp \
                    service/NearbyServicesFinder.cpp \
                    service/TransportEstimator.cpp \
                    util/GeoUtil.cpp \
                    $(PERSISTENCE_SRC) \
                    $(DISPATCH_SRC) \
                    $(AUTH_SRC)

TEST_AUTH_SRC = test/test_auth.cpp \
                model/Location.cpp \
                structure/LocationNode.cpp \
                structure/RoadEdge.cpp \
                structure/RoadGraph.cpp \
                repository/LocationRepository.cpp \
                datasource/LocationCsvLoader.cpp \
                datasource/RoadNetworkCsvLoader.cpp \
                dto/RouteResponse.cpp \
                service/RoadGraphBuilder.cpp \
                service/ShortestPathFinder.cpp \
                service/NearbyServicesFinder.cpp \
                service/TransportEstimator.cpp \
                service/RestApiServer.cpp \
                util/GeoUtil.cpp \
                $(PERSISTENCE_SRC) \
                $(DISPATCH_SRC) \
                $(AUTH_SRC)

TEST_PORTALS_SRC = test/test_portals.cpp \
                   model/Location.cpp \
                   structure/LocationNode.cpp \
                   structure/RoadEdge.cpp \
                   structure/RoadGraph.cpp \
                   repository/LocationRepository.cpp \
                   datasource/LocationCsvLoader.cpp \
                   datasource/RoadNetworkCsvLoader.cpp \
                   dto/RouteResponse.cpp \
                   service/RoadGraphBuilder.cpp \
                   service/ShortestPathFinder.cpp \
                   service/NearbyServicesFinder.cpp \
                   service/TransportEstimator.cpp \
                   util/GeoUtil.cpp \
                   $(PERSISTENCE_SRC) \
                   $(DISPATCH_SRC) \
                   $(AUTH_SRC)

TEST_RELIABILITY_SRC = test/test_reliability.cpp \
                       model/Location.cpp \
                       structure/LocationNode.cpp \
                       structure/RoadEdge.cpp \
                       structure/RoadGraph.cpp \
                       repository/LocationRepository.cpp \
                       datasource/LocationCsvLoader.cpp \
                       datasource/RoadNetworkCsvLoader.cpp \
                       dto/RouteResponse.cpp \
                       service/RoadGraphBuilder.cpp \
                       service/ShortestPathFinder.cpp \
                       service/NearbyServicesFinder.cpp \
                       service/TransportEstimator.cpp \
                       util/GeoUtil.cpp \
                       $(PERSISTENCE_SRC) \
                       $(DISPATCH_SRC) \
                       $(AUTH_SRC)

TEST_SECURITY_SRC = test/test_security.cpp \
                    service/RestApiServer.cpp \
                    model/Location.cpp \
                    structure/LocationNode.cpp \
                    structure/RoadEdge.cpp \
                    structure/RoadGraph.cpp \
                    repository/LocationRepository.cpp \
                    datasource/LocationCsvLoader.cpp \
                    datasource/RoadNetworkCsvLoader.cpp \
                    dto/RouteResponse.cpp \
                    service/RoadGraphBuilder.cpp \
                    service/ShortestPathFinder.cpp \
                    service/NearbyServicesFinder.cpp \
                    service/TransportEstimator.cpp \
                    util/GeoUtil.cpp \
                    $(PERSISTENCE_SRC) \
                    $(DISPATCH_SRC) \
                    $(AUTH_SRC)

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
  MKDIR_BUILD = mkdir -p $(BUILD_DIR)
  CLEAN_CMD = rm -rf $(BUILD_DIR)
  RUN_CMD = ./$(1)
else
  MKDIR_BUILD = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
  CLEAN_CMD = -if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
  RUN_CMD = $(subst /,\,$(1))
endif

all: $(TARGET)

server: $(TARGET)

$(BUILD_DIR):
	$(MKDIR_BUILD)

# Compile SQLite amalgamation as C (only when missing or changed)
$(SQLITE_OBJ): third_party/sqlite/sqlite3.c | $(BUILD_DIR)
	$(CC) -O2 $(SQLITE_FLAGS) -c third_party/sqlite/sqlite3.c -o $(SQLITE_OBJ)

$(TARGET): $(SRC) $(SQLITE_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(SRC) $(SQLITE_OBJ) -o $(TARGET) $(LDFLAGS)

$(TEST_TARGET): $(TEST_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(TEST_SRC) -o $(TEST_TARGET) $(LDFLAGS)

$(TEST_DB_TARGET): $(TEST_DB_SRC) $(SQLITE_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(TEST_DB_SRC) $(SQLITE_OBJ) -o $(TEST_DB_TARGET) $(LDFLAGS)

$(TEST_DISPATCH_TARGET): $(TEST_DISPATCH_SRC) $(SQLITE_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(TEST_DISPATCH_SRC) $(SQLITE_OBJ) -o $(TEST_DISPATCH_TARGET) $(LDFLAGS)

$(TEST_AUTH_TARGET): $(TEST_AUTH_SRC) $(SQLITE_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(TEST_AUTH_SRC) $(SQLITE_OBJ) -o $(TEST_AUTH_TARGET) $(LDFLAGS)

$(TEST_PORTALS_TARGET): $(TEST_PORTALS_SRC) $(SQLITE_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(TEST_PORTALS_SRC) $(SQLITE_OBJ) -o $(TEST_PORTALS_TARGET) $(LDFLAGS)

$(TEST_RELIABILITY_TARGET): $(TEST_RELIABILITY_SRC) $(SQLITE_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(TEST_RELIABILITY_SRC) $(SQLITE_OBJ) -o $(TEST_RELIABILITY_TARGET) $(LDFLAGS)

$(TEST_SECURITY_TARGET): $(TEST_SECURITY_SRC) $(SQLITE_OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $(TEST_SECURITY_SRC) $(SQLITE_OBJ) -o $(TEST_SECURITY_TARGET) $(LDFLAGS)

test: $(TEST_TARGET) $(TEST_DB_TARGET) $(TEST_DISPATCH_TARGET) $(TEST_AUTH_TARGET) $(TEST_PORTALS_TARGET) $(TEST_RELIABILITY_TARGET) $(TEST_SECURITY_TARGET)
	$(call RUN_CMD,$(TEST_TARGET))
	$(call RUN_CMD,$(TEST_DB_TARGET))
	$(call RUN_CMD,$(TEST_DISPATCH_TARGET))
	$(call RUN_CMD,$(TEST_AUTH_TARGET))
	$(call RUN_CMD,$(TEST_PORTALS_TARGET))
	$(call RUN_CMD,$(TEST_RELIABILITY_TARGET))
	$(call RUN_CMD,$(TEST_SECURITY_TARGET))

test_db: $(TEST_DB_TARGET)
	$(call RUN_CMD,$(TEST_DB_TARGET))

test_dispatch: $(TEST_DISPATCH_TARGET)
	$(call RUN_CMD,$(TEST_DISPATCH_TARGET))

test_auth: $(TEST_AUTH_TARGET)
	$(call RUN_CMD,$(TEST_AUTH_TARGET))

test_portals: $(TEST_PORTALS_TARGET)
	$(call RUN_CMD,$(TEST_PORTALS_TARGET))

test_reliability: $(TEST_RELIABILITY_TARGET)
	$(call RUN_CMD,$(TEST_RELIABILITY_TARGET))

test_security: $(TEST_SECURITY_TARGET)
	$(call RUN_CMD,$(TEST_SECURITY_TARGET))

run: $(TARGET)
	$(call RUN_CMD,$(TARGET))

serve: $(TARGET)
	$(call RUN_CMD,$(TARGET)) --serve

clean:
	$(CLEAN_CMD)

.PHONY: all server test test_db test_dispatch test_auth test_portals test_reliability run serve clean
