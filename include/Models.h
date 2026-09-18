#ifndef MODELS_H
#define MODELS_H

#include <string>

// Stores the information for one campus resource.
struct Resource {
    std::string id;
    std::string name;
    std::string type;
    bool available = true;
    int reservationCount = 0;
};

// Stores one active or cancelled reservation.
struct Reservation {
    int id = 0;
    std::string studentId;
    std::string studentName;
    std::string resourceId;
    std::string date;
};

// Stores a student request while the resource is unavailable.
struct WaitingRequest {
    std::string studentId;
    std::string studentName;
    std::string date;
};

#endif