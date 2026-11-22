#ifndef M3_SERVICE_REST_API_SERVER_H
#define M3_SERVICE_REST_API_SERVER_H

#include "ShortestPathFinder.h"
#include "NearbyServicesFinder.h"
#include "TransportEstimator.h"
#include "../repository/ILocationRepository.h"
#include "../persistence/SqliteHospitalRepository.h"
#include "../persistence/SqliteAmbulanceRepository.h"
#include "../persistence/SqliteEmergencyRepository.h"
#include "../persistence/SqlitePatientLocationRepository.h"
#include "EmergencyDispatcher.h"
#include "AuthService.h"
#include "../persistence/Database.h"
#include "../persistence/SqliteUserRepository.h"
#include "../persistence/SqliteSessionRepository.h"
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <optional>

// Forward declaration of httplib::Server
namespace httplib {
    class Server;
    struct Request;
    struct Response;
}

class RestApiServer {
private:
    int port;
    std::string host;
    std::shared_ptr<ShortestPathFinder> pathFinder;
    std::shared_ptr<NearbyServicesFinder> nearbyFinder;
    std::shared_ptr<TransportEstimator> transportEstimator;
    std::shared_ptr<ILocationRepository> locationRepo;
    std::shared_ptr<RoadGraph> graph;

    std::shared_ptr<SqliteHospitalRepository> hospitalRepo;
    std::shared_ptr<SqliteAmbulanceRepository> ambulanceRepo;
    std::shared_ptr<SqliteEmergencyRepository> emergencyRepo;
    std::shared_ptr<SqlitePatientLocationRepository> patientRepo;
    std::shared_ptr<EmergencyDispatcher> dispatcher;

    std::shared_ptr<AuthService> authService;
    std::shared_ptr<SqliteUserRepository> userRepo;
    std::shared_ptr<Database> db;

    std::unique_ptr<httplib::Server> server;
    std::thread serverThread;
    std::atomic<bool> running;

    void registerRoutes();

public:
    RestApiServer(int port,
                  std::shared_ptr<ShortestPathFinder> pf,
                  std::shared_ptr<NearbyServicesFinder> nf,
                  std::shared_ptr<TransportEstimator> te,
                  std::shared_ptr<ILocationRepository> lr,
                  std::shared_ptr<RoadGraph> rg,
                  std::shared_ptr<SqliteHospitalRepository> hr,
                  std::shared_ptr<SqliteAmbulanceRepository> ar,
                  std::shared_ptr<SqliteEmergencyRepository> er,
                  std::shared_ptr<SqlitePatientLocationRepository> pr,
                  std::shared_ptr<EmergencyDispatcher> disp = nullptr,
                  std::shared_ptr<AuthService> auth = nullptr,
                  std::shared_ptr<SqliteUserRepository> ur = nullptr,
                  const std::string& host = "0.0.0.0",
                  std::shared_ptr<Database> database = nullptr);
    ~RestApiServer();

    // Authentication & authorization helper
    std::optional<AuthUser> authenticate(
        const httplib::Request& req,
        httplib::Response& res,
        const std::vector<std::string>& allowedRoles = {});

    static std::string extractBearerToken(const httplib::Request& req);

    void resetDemoData();
    void clearRateLimits();

    void start(bool blocking = false);
    void stop();
    bool isRunning() const { return running.load(); }
    int getPort() const { return port; }
};

#endif // M3_SERVICE_REST_API_SERVER_H
