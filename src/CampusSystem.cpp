#include "CampusSystem.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;

vector<string> CampusSystem::split(const string& line, char delimiter) {
    // Break one line from a data file into separate fields.
    vector<string> fields;
    string field;
    stringstream stream(line);
    while (getline(stream, field, delimiter)) fields.push_back(field);
    return fields;
}

Resource* CampusSystem::findResource(const string& id) {
    // Resource IDs are searched linearly through the vector.
    for (Resource& resource : resources) {
        if (resource.id == id) return &resource;
    }
    return nullptr;
}

bool CampusSystem::reservationIdExists(int id) const {
    return reservations.find(id) != nullptr;
}

bool CampusSystem::readInt(const string& prompt, int& value) {
    cout << prompt;
    if (cin >> value) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return true;
    }
    // Reset the input stream so the menu can continue after bad input.
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid number.\n";
    return false;
}

string CampusSystem::readText(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

void CampusSystem::addDefaultResources() {
    resources = {
        {"R205", "Quiet Study Room", "Study Room", true, 0},
        {"L101", "Dell Laptop 101", "Laptop", true, 0},
        {"C014", "Scientific Calculator", "Calculator", true, 0},
        {"E310", "Electronics Lab Kit", "Lab Equipment", true, 0},
        {"T200", "Math Tutoring Appointment", "Tutoring", true, 0}
    };
}

void CampusSystem::loadResources() {
    // Use the input file when it exists; otherwise use sample resources.
    ifstream file("data/resources.txt");
    if (!file) {
        addDefaultResources();
        return;
    }
    string line;
    while (getline(file, line)) {
        vector<string> fields = split(line, '|');
        if (fields.size() >= 4) {
            resources.push_back({fields[0], fields[1], fields[2], fields[3] == "available", 0});
        }
    }
    if (resources.empty()) addDefaultResources();
}

void CampusSystem::loadReservations() {
    ifstream file("data/reservations.txt");
    if (!file) return;
    string line;
    while (getline(file, line)) {
        // Invalid or comment lines are ignored by the field-count check.
        vector<string> fields = split(line, '|');
        if (fields.size() < 5) continue;
        try {
            Reservation reservation{stoi(fields[0]), fields[1], fields[2], fields[3], fields[4]};
            Resource* resource = findResource(reservation.resourceId);
            if (!resource || reservationIdExists(reservation.id)) continue;
            reservations.add(reservation);
            resource->available = false;
            ++resource->reservationCount;
            nextReservationId = max(nextReservationId, reservation.id + 1);
        } catch (const invalid_argument&) {
            continue;
        }
    }
}

bool CampusSystem::compareResources(const Resource& left, const Resource& right) {
    return left.name < right.name;
}

bool CampusSystem::compareResourcePopularity(const Resource& left, const Resource& right) {
    return left.reservationCount > right.reservationCount;
}
// Merge sort gives predictable O(n log n) ordering for resource reports.
void CampusSystem::mergeSort(vector<Resource>& values, int first, int last) {
    if (first >= last) return;
    int middle = first + (last - first) / 2;
    mergeSort(values, first, middle);
    mergeSort(values, middle + 1, last);
    vector<Resource> merged;
    int left = first;
    int right = middle + 1;
    while (left <= middle && right <= last) {
        if (compareResources(values[left], values[right])) merged.push_back(values[left++]);
        else merged.push_back(values[right++]);
    }
    while (left <= middle) merged.push_back(values[left++]);
    while (right <= last) merged.push_back(values[right++]);
    copy(merged.begin(), merged.end(), values.begin() + first);
}
void CampusSystem::mergeSortByPopularity(vector<Resource>& values, int first, int last) {
    if (first >= last) return;

    int middle = first + (last - first) / 2;

    mergeSortByPopularity(values, first, middle);
    mergeSortByPopularity(values, middle + 1, last);

    vector<Resource> merged;

    int left = first;
    int right = middle + 1;

    while (left <= middle && right <= last) {
        if (compareResourcePopularity(values[left], values[right])) {
            merged.push_back(values[left++]);
        }
        else {
            merged.push_back(values[right++]);
        }
    }

    while (left <= middle) {
        merged.push_back(values[left++]);
    }

    while (right <= last) {
        merged.push_back(values[right++]);
    }

    copy(merged.begin(), merged.end(), values.begin() + first);
}
void CampusSystem::mergeSortReservationsByDate(vector<Reservation>& values, int first, int last) {
    if (first >= last) return;

    int middle = first + (last - first) / 2;

    mergeSortReservationsByDate(values, first, middle);
    mergeSortReservationsByDate(values, middle + 1, last);

    vector<Reservation> merged;

    int left = first;
    int right = middle + 1;

    while (left <= middle && right <= last) {
        if (values[left].date < values[right].date) {
            merged.push_back(values[left++]);
        }
        else {
            merged.push_back(values[right++]);
        }
    }

    while (left <= middle) {
        merged.push_back(values[left++]);
    }

    while (right <= last) {
        merged.push_back(values[right++]);
    }

    copy(merged.begin(), merged.end(), values.begin() + first);
}
void CampusSystem::displayResource(const Resource& resource) const {
    cout << left << setw(8) << resource.id << setw(26) << resource.name
         << setw(18) << resource.type << (resource.available ? "Available" : "Reserved") << '\n';
}

void CampusSystem::createReservation() {
    string resourceId = readText("Resource ID: ");
    Resource* resource = findResource(resourceId);
    if (!resource) {
        cout << "Invalid resource ID.\n";
        return;
    }
    WaitingRequest request{readText("Student ID: "), readText("Student Name: "), readText("Reservation Date: ")};
    // An unavailable resource sends the request to its waiting queue.
    if (!resource->available) {
        waitingLists[resourceId].push(request);
        cout << "Resource unavailable. Added to the waiting list.\n";
        return;
    }
    Reservation reservation{nextReservationId++, request.studentId, request.studentName, resourceId, request.date};
    reservations.add(reservation);
    resource->available = false;
    ++resource->reservationCount;
    cout << "Reservation Created Successfully. ID: " << reservation.id << '\n';
}

void CampusSystem::cancelReservation() {
    int id;
    if (!readInt("Reservation ID: ", id)) return;
    Reservation cancelled;
    if (!reservations.remove(id, cancelled)) {
        cout << "Reservation not found.\n";
        return;
    }
    Resource* resource = findResource(cancelled.resourceId);
    if (resource) {
        resource->available = true;
        if (resource->reservationCount > 0) --resource->reservationCount;
    }
    // Save the cancellation so option 5 can undo it later.
    cancellationHistory.push(cancelled);
    cout << "Reservation Cancelled and added to cancellation history.\n";
    assignNextWaiting(cancelled.resourceId);
}

void CampusSystem::assignNextWaiting(const string& resourceId) {
    Resource* resource = findResource(resourceId);
    auto list = waitingLists.find(resourceId);
    if (!resource || list == waitingLists.end() || list->second.empty() || !resource->available) return;
    // The front of the queue is the student who has waited the longest.
    WaitingRequest request = list->second.front();
    list->second.pop();
    Reservation reservation{nextReservationId++, request.studentId, request.studentName, resourceId, request.date};
    reservations.add(reservation);
    resource->available = false;
    ++resource->reservationCount;
    cout << "Waiting-list request assigned reservation ID " << reservation.id << ".\n";
}

void CampusSystem::undoCancellation() {
    if (cancellationHistory.empty()) {
        cout << "Cancellation history is empty.\n";
        return;
    }
    // Only the most recent cancellation can be restored.
    Reservation restored = cancellationHistory.top();
    Resource* resource = findResource(restored.resourceId);
    if (!resource || !resource->available) {
        cout << "Cannot restore: the resource is already reserved.\n";
        return;
    }
    cancellationHistory.pop();
    reservations.add(restored);
    resource->available = false;
    ++resource->reservationCount;
    cout << "Reservation Restored Successfully.\n";
}

void CampusSystem::searchReservations() {
    string query = readText("Enter reservation ID or student ID: ");
    bool found = false;
    // Search by either the reservation ID or the student's ID.
    for (const Reservation& reservation : reservations.toVector()) {
        if (to_string(reservation.id) == query || reservation.studentId == query) {
            cout << "ID: " << reservation.id << " | Student: " << reservation.studentName
                 << " (" << reservation.studentId << ") | Resource: " << reservation.resourceId
                 << " | Date: " << reservation.date << '\n';
            found = true;
        }
    }
    if (!found) cout << "No matching reservations found.\n";
}
void CampusSystem::searchResource() {
    string id = readText("Enter resource ID: ");

    Resource* resource = findResource(id);

    if (resource != nullptr) {
        cout << "\nResource Found:\n";
        cout << left << setw(8) << "ID"
             << setw(26) << "Name"
             << setw(18) << "Type"
             << "Status\n";

        displayResource(*resource);
    }
    else {
        cout << "Resource not found.\n";
    }
}
void CampusSystem::viewWaitingLists() const {
    bool found = false;
    for (const auto& entry : waitingLists) {
        if (entry.second.empty()) continue;
        found = true;
        queue<WaitingRequest> requests = entry.second;
        cout << entry.first << ": " << requests.size() << " waiting request(s)\n";
        while (!requests.empty()) {
            cout << "  " << requests.front().studentId << " - " << requests.front().studentName << '\n';
            requests.pop();
        }
    }
    if (!found) cout << "No students are currently waiting.\n";
}
void CampusSystem::reportActiveReservations() const {
    vector<Reservation> activeReservations = reservations.toVector();

    cout << "\n=== Active Reservations ===\n";

    if (activeReservations.empty()) {
        cout << "No active reservations.\n";
        return;
    }

    cout << "Total active reservations: "
         << activeReservations.size() << '\n';

    for (const Reservation& reservation : activeReservations) {
        cout << "ID: " << reservation.id
             << " | Student: " << reservation.studentName
             << " (" << reservation.studentId << ")"
             << " | Resource: " << reservation.resourceId
             << " | Date: " << reservation.date << '\n';
    }
    void CampusSystem::reportResourceUtilization() const {
    cout << "\n=== Resource Utilization ===\n";

    if (resources.empty()) {
        cout << "No resources available.\n";
        return;
    }

    for (const Resource& resource : resources) {
        cout << resource.id
             << " | " << resource.name
             << " | Reservations: " << resource.reservationCount
             << " | Status: "
             << (resource.available ? "Available" : "Reserved")
             << '\n';
    }
}
void CampusSystem::generateReport() const {
    // Sort a copy so the original resource order is not changed by a report.
    vector<Resource> sorted = resources;
    if (!sorted.empty()) mergeSort(sorted, 0, static_cast<int>(sorted.size()) - 1);
    cout << "\n=== Campus Resource Report ===\n";
    cout << "Active reservations: " << reservations.size() << '\n';
    cout << "\nResource availability and usage:\n";
    cout << left << setw(8) << "ID" << setw(26) << "Name" << setw(18) << "Type" << "Status\n";
    for (const Resource& resource : sorted) displayResource(resource);
    cout << "\nWaiting lists:\n";
    for (const auto& entry : waitingLists) {
        if (!entry.second.empty()) cout << entry.first << ": " << entry.second.size() << " waiting\n";
    }
}

CampusSystem::CampusSystem() {
    // Load files before showing the menu to the user.
    loadResources();
    loadReservations();
}

void CampusSystem::run() {
    cout << "===== Campus Resource Reservation System =====\n";
    while (true) {
        cout << "\n1. View Resources\n2. Create Reservation\n3. Cancel Reservation\n"
                "4. View Waiting Lists\n5. Undo Cancellation\n6. Search\n"
                "7. Sort Resources\n8. Generate Report\n9. Exit\n";
        int choice;
        if (!readInt("Enter Choice: ", choice)) continue;
        // Each case performs one action from the main menu.
        switch (choice) {
            case 1:
                cout << left << setw(8) << "ID" << setw(26) << "Name" << setw(18) << "Type" << "Status\n";
                for (const Resource& resource : resources) displayResource(resource);
                break;
            case 2: createReservation(); break;
            case 3: cancelReservation(); break;
            case 4: viewWaitingLists(); break;
            case 5: undoCancellation(); break;
            
            case 6: {
                cout << "\n===== Search Menu =====\n";
                cout << "1. Search Resource by ID\n";
                cout << "2. Search Reservation or Student ID\n";
                cout << "3. Back\n";
            
                int searchChoice;
            
                if (!readInt("Enter Choice: ", searchChoice))
                    break;
            
                if (searchChoice == 1) {
                    searchResource();
                }
                else if (searchChoice == 2) {
                    searchReservations();
                }
                else if (searchChoice == 3) {
                    break;
                }
                else {
                    cout << "Invalid search option.\n";
                }
            
                break;
            }
            
            case 7: {
                cout << "\n===== Sort Menu =====\n";
                cout << "1. Sort Resources by Name\n";
                cout << "2. Sort Resources by Popularity\n";
                cout << "3. Sort Reservations by Date\n";
                cout << "4. Back\n";
            
                int sortChoice;
            
                if (!readInt("Enter Choice: ", sortChoice))
                    break;
            
                if (sortChoice == 1) {
                    if (!resources.empty()) {
                        mergeSort(resources, 0, static_cast<int>(resources.size()) - 1);
                    }
                    cout << "Resources sorted by name.\n";
                }
                else if (sortChoice == 2) {
                    if (!resources.empty()) {
                        mergeSortByPopularity(resources, 0, static_cast<int>(resources.size()) - 1);
                    }
                    cout << "Resources sorted by popularity.\n";
                }
                else if (sortChoice == 3) {
                    vector<Reservation> sortedReservations = reservations.toVector();
                
                    if (!sortedReservations.empty()) {
                        mergeSortReservationsByDate(
                            sortedReservations,
                            0,
                            static_cast<int>(sortedReservations.size()) - 1
                        );
                    }
                
                    cout << "\nReservations sorted by date:\n";
                
                    for (const Reservation& reservation : sortedReservations) {
                        cout << "ID: " << reservation.id
                             << " | Student: " << reservation.studentName
                             << " | Resource: " << reservation.resourceId
                             << " | Date: " << reservation.date << '\n';
                    }
                }
                else if (sortChoice == 4) {
                    break;
                }
            case 8: generateReport(); break;
            case 9: cout << "Goodbye.\n"; return;
            default: cout << "Please select a menu option from 1 to 9.\n";
        }
    }
}
