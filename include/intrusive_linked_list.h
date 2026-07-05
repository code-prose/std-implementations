template <typename T>
class Intrusive {
    public:
        T* next;
        T* prev;
};


// example class
class Order : Intrusive<Order> {
    int id{};
    double price{};
};

template <typename T>
T* push_front(T* item, T*& head) {
    if (head == nullptr) return item;

    item->prev = nullptr;
    item->next = head;
    head->prev = item;
    return item;
}

template <typename T>
void remove(T* item, T*& head) {
    T* prev = item->prev;
    T* next = item->next;

    if (item->prev) item->prev->next = item->next;
    else head = item-> next;
    if (item->next) item->next->prev = item->prev;
    item->prev = item->next = nullptr;
}
