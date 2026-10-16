CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I. -Iinclude
LDLIBS = -lws2_32 -lcrypt32

BUILD_DIR = build
TARGET = $(BUILD_DIR)/m3_nav.exe
TEST_TARGET = $(BUILD_DIR)/test_runner.exe

# Core shared sources across application and test runners
CORE_SRCS = model/Location.cpp \
            model/TransportMode.cpp \
            model/TransportOption.cpp \
            model/AmbulanceOption.cpp \
            model/CarOption.cpp \
            repository/LocationRepository.cpp \
            repository/TransportOptionRepository.cpp \
            datasource/LocationCsvLoader.cpp \
            datasource/RoadNetworkCsvLoader.cpp \
            datasource/TransportOptionCsvLoader.cpp \
            service/RoadGraphBuilder.cpp \
            service/ShortestPathFinder.cpp \
            service/RouteRequestValidator.cpp \
            service/TransportEstimator.cpp \
            service/NearbyServicesFinder.cpp \
            service/RestApiServer.cpp \
            structure/LocationNode.cpp \
            structure/RoadEdge.cpp \
            structure/RoadGraph.cpp \
            dto/RouteSegment.cpp \
            dto/RouteResponse.cpp \
            dto/RouteRequest.cpp \
            util/GeoUtil.cpp

MAIN_SRC = main.cpp
TEST_SRC = test/test_main.cpp

.PHONY: all run test clean

all: $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(MAIN_SRC) $(CORE_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) $(CORE_SRCS) -o $(TARGET) $(LDLIBS)

run: all
	./$(TARGET)

$(TEST_TARGET): $(TEST_SRC) $(CORE_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(CORE_SRCS) -o $(TEST_TARGET) $(LDLIBS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	@rm -rf $(BUILD_DIR)
