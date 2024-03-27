// #include <iostream>

template<class T> class List;
template<class T> class Node;

//==============================================================

template<class T>
class Node {
private :
    T data;
    Node<T>* next;
public :
    //-----------------------------------------------------
    Node(const T& d) : data(d) {
        next = nullptr;
    }
    //-----------------------------------------------------
    void print() {
        std::cout << data.toStr() << std::endl;
    }
    //-----------------------------------------------------
    T getData() {
        return data;
    }
    //-----------------------------------------------------
friend class List<T>;
};

//==============================================================

template<class T>
class List {
private :
    bool    copied;
    Node<T> **head;
    Node<T> **tail;
public:
    //-----------------------------------------------------
    List() : copied(false) {
        head = new (Node<T>*);
        tail = new (Node<T>*);
        (*head) = (*tail) = nullptr;
    }
    //-----------------------------------------------------
    List(const List& source) {
        head = source.head;
        tail = source.tail;
        copied = true;
    }
    //-----------------------------------------------------
    ~List() {
        empty();
        if (copied) return;
        delete head;
        delete tail;
    }
    //-----------------------------------------------------
    void copy(const List<T>& source) const {
        empty();
        Node<T>* aux = (*source.head);
        while (aux) {
            append(aux->data);
            aux = aux->next;
        }
    }
    //-----------------------------------------------------
    inline std::string toStr() const {
        Node<T>* aux = (*head);
        std::ostringstream stream("");
        stream << "{";
        while (aux) {
            stream << aux->data << ",";
            aux = aux->next;
        }
        stream <<  "}";
        return stream.str();
    }
    //-----------------------------------------------------
    void push(T d) {
        Node<T>* aux = new Node<T>(d);
        if (!(*head)) {
            (*head) = (*tail) = aux;
        } else {
            aux->next = (*head);
            (*head) = aux;
        }
    }
    //-----------------------------------------------------
    void append(const T& d) const {
        Node<T>* aux = new Node<T>(d);
        if (!*head) {
            (*head) = (*tail) = aux;
        } else {
            (*tail)->next = aux;
            *tail = aux;
        }
    }
    //-----------------------------------------------------
    T pull() const {
        if (*head) {
            Node<T>* aux = (*head);
            (*head) = (*head)->next;
            if (*head) (*tail)=nullptr;
            return aux->data;
        }
        return 0;
    }
    //-----------------------------------------------------
    void empty() const {
        while (*head) {
            Node<T>* aux = (*head);
            (*head) = (*head)->next;
            delete aux;
        }
    }
    //-----------------------------------------------------
    Node<T>* elementAt(int i) const {
        int counter=0;
        Node<T>* aux = (*head);
        while (aux) {
            if (counter == i) {
                return aux;
            }
            aux = aux->next;
            counter++;
        }
        return aux;
    }
    //-----------------------------------------------------
    T get(int i) const {
        int counter=0;
        Node<T>* aux = (*head);
        while (aux) {
            if (counter == i) {
                return aux->data;
            }
            aux = aux->next;
            counter++;
        }
        return aux->data;
    }
    //-----------------------------------------------------
    void set(int i, T d) {
        Node<T>* aux = elementAt(i);
        if (aux) {
            aux->data = d;
        }
    }
    //-----------------------------------------------------
    int lenght() const {
        int counter=0;
        Node<T>* aux = (*head);
        while (aux) {
            aux = aux->next;
            counter++;
        }
        return counter;
    }
    //-----------------------------------------------------
    T operator[](int i) const {
        return get(i);
    }
    //-----------------------------------------------------
    bool operator==(const List<T>& other) {
        int counter = 0;
        Node<T>* aux1 = (*head);
        Node<T>* aux2 = (*other.head);
        while (aux1 && aux2) {
            if (aux1->data != aux2->data) return false;
            aux1 = aux1->next;
            aux2 = aux2->next;
        }
        if (aux1 || aux2) return false;
        return true;
    }
    //-----------------------------------------------------
    bool exists(const T& other) {
        Node<T>* aux = (*head);
        while (aux) {
            if (aux->data == other) return true;
            aux = aux->next;
        }
        return false;
    }

};
