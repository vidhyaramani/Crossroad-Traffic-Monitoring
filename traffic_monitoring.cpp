#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <boost/intrusive/list.hpp>

// Represent states for the Crossroad traffic Monitoring
enum class State {   //data types represents using enum
    Init,
    Active,
    Error,
    Stopped
};

// Base class for vehicles
class Vehicle {
public:
    std::string id;

    explicit Vehicle(const std::string& id) : id(id) {}  // Constructor for the Vehicle class
    virtual ~Vehicle() = default;  //Destructor for the Vehicle class

    virtual std::string getType() const = 0; // Pure virtual method
};

// Classes for specific vehicle types
class Bicycle : public Vehicle {
public:
    explicit Bicycle(const std::string& id) : Vehicle(id) {}
    std::string getType() const override { return "Bicycle"; }
};

class Car : public Vehicle {
public:
    explicit Car(const std::string& id) : Vehicle(id) {}
    std::string getType() const override { return "Car"; }
};

class Scooter : public Vehicle {
public:
    explicit Scooter(const std::string& id) : Vehicle(id) {}
    std::string getType() const override { return "Scooter"; }
};

// VehicleNode with the list
struct VehicleNode : public boost::intrusive::list_base_hook<> {
    std::string id;
    std::string type;
    int count;

    VehicleNode(const std::string& id, const std::string& type)
        : id(id), type(type), count(1) {}
};

// Class for Crossroad Traffic Monitoring 
class CrossroadTrafficMonitoring {
public:
    explicit CrossroadTrafficMonitoring(int period)
        : state(State::Init), period(period), errorCount(0) {}

    void Start() {
        if (state == State::Init) {
            state = State::Active;
        }
    }

    void Stop() {
        if (state == State::Active) {
            state = State::Stopped;
        }
    }

    void Reset() {
        state = State::Active;
        errorCount = 0;
        vehicleList.clear_and_dispose([](VehicleNode* node) { delete node; });
        vehicleMap.clear();
    }

    void OnSignal() {
        if (state == State::Active || state == State::Init) {
            state = State::Error;
        } else if (state == State::Error) {
            errorCount++;
            std::cerr << "Error: Issue in Signal.\n";
        }
    }

    void OnSignal(const Vehicle& vehicle) {
    // Check if the traffic system is in Active state
    if (state != State::Active) {
        std::cout << "Traffic System is not Active\n";
        return;
    }

    // Generate the unique key for the vehicle
    const std::string key = vehicle.id + "-" + vehicle.getType();

    // Check if the vehicle already exists in the map
    auto it = vehicleMap.find(key);
    if (it != vehicleMap.end()) {
        // Vehicle already exists, increment its count
        it->second->count++;
        std::cout << "Vehicle " << vehicle.id << " (" << vehicle.getType() 
                  << ") count incremented to " << it->second->count << ".\n";
    } else {
        // Vehicle does not exist, create a new node and add it in the system
        std::cout << "Vehicle " << vehicle.id << " (" << vehicle.getType() 
                  << ") is added new for the system..\n";

        // Create a new VehicleNode
        auto node = std::make_unique<VehicleNode>(vehicle.id, vehicle.getType());
        
        // Add the node to the Vehicle list
        vehicleList.push_back(*node);
        
        // Update the map with the new node
        vehicleMap[key] = node.get();

        node.release();
    }
}
    
    int GetErrorCount() const {
        return errorCount;
    }

    void GetStatistics() const {
        std::cout << "Traffic Statistics:\n";
        for (const auto& node : vehicleList) {
            std::cout << "- " << node.id << " (" << node.type << "): " << node.count << " pass\n";
        }
    }

    ~CrossroadTrafficMonitoring() {    // Destructor
        vehicleList.clear_and_dispose([](VehicleNode* node) { delete node; });
    }

private:
    State state;
    int period;
    int errorCount;
    boost::intrusive::list<VehicleNode> vehicleList;
    std::unordered_map<std::string, VehicleNode*> vehicleMap;
};

int main() {
    CrossroadTrafficMonitoring traffic_monitoring(10);
    traffic_monitoring.Start();

    Bicycle a("ABC-011");
    Car b("ABC-012");
    Scooter c("ABC-014");

    traffic_monitoring.OnSignal(a);
    traffic_monitoring.OnSignal(b);
    traffic_monitoring.OnSignal(c);

    traffic_monitoring.OnSignal(); // Error signal

    traffic_monitoring.GetStatistics();
    std::cout << "Error Count: " << traffic_monitoring.GetErrorCount() << "\n";

    traffic_monitoring.Reset();
    traffic_monitoring.OnSignal(b);
    traffic_monitoring.OnSignal(c);

    traffic_monitoring.GetStatistics();
    if (traffic_monitoring.GetErrorCount() > 0) {
        std::cout << "Error Count: " << traffic_monitoring.GetErrorCount() << "\n";
    } else {
        std::cout << "No errors\n";
    }

    return 0;
}