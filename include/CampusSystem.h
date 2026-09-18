#ifndef CAMPUS_SYSTEM_H
#define CAMPUS_SYSTEM_H

#include "Models.h"
#include "ReservationList.h"
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

// Controls the menu and connects all of the reservation data structures
class CampusSystem {
private:
    // A vector is used because resources are easy to scan and sort
    std::vector<Resource> resources;
    // Active reservations are required to use a linked list
    ReservationList reservations;
    // Each resource ID has its own first-in, first-out waiting queue.
    std::unordered_map<std::string, std::queue<WaitingRequest>> waitingLists;
    // The latest cancelled reservation is restored first.
    std::stack<Reservation> cancellationHistory;
    int nextReservationId = 301;

    static std::vector<std::string> split(const std::string& line, char delimiter);
    Resource* findResource(const std::string& id);
    bool reservationIdExists(int id) const;
    static bool readInt(const std::string& prompt, int& value);
    static std::string readText(const std::string& prompt);
    void addDefaultResources();
    void loadResources();
    void loadReservations();
    static bool compareResources(const Resource& left, const Resource& right);
    static void mergeSort(std::vector<Resource>& values, int first, int last);
    void displayResource(const Resource& resource) const;
    void createReservation();
    void cancelReservation();
    void assignNextWaiting(const std::string& resourceId);
    void undoCancellation();
    void searchReservations();
    void viewWaitingLists() const;
    void generateReport() const;

public:
    CampusSystem();
    void run();
};

#endif