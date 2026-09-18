#ifndef RESERVATION_LIST_H
#define RESERVATION_LIST_H

#include "Models.h"
#include <vector>

// This linked list stores the active reservations in the system.
class ReservationList {
private:
    struct Node {
        Reservation value;
        Node* next;
        Node(const Reservation& reservation, Node* nextNode = nullptr);
    };

    Node* head = nullptr;

public:
    // The default constructor starts with an empty list.
    ReservationList() = default;
    // The destructor removes all dynamically allocated nodes.
    ~ReservationList();

    ReservationList(const ReservationList&) = delete;
    ReservationList& operator=(const ReservationList&) = delete;

    // Adds a reservation to the front of the list.
    void add(const Reservation& reservation);
    // Removes a reservation and copies it into removed.
    bool remove(int id, Reservation& removed);
    // Searches for a reservation by its ID.
    Reservation* find(int id);
    const Reservation* find(int id) const;
    std::vector<Reservation> toVector() const;
    int size() const;
    void clear();
};

#endif