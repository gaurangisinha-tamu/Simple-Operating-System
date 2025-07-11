// Header inclusion
#include "thread.H"

// Definition of the Thread Container class
class Thread;
class threadnode {
private:
    Thread* payloadThread; 
    threadnode* nextNode; 

public:
    // Static members to keep track of the list's head and tail
    static threadnode* head_list;
    static threadnode* tail_list;

    // Default constructor
    threadnode() : payloadThread(nullptr), nextNode(nullptr) {}

    // Constructor with thread parameter
        threadnode(Thread *thread1){
        payloadThread = thread1;
        nextNode = nullptr;
    }

    // Method to add a thread to the container
    static void enqueue(Thread *thread1) {
        threadnode* newNode = new threadnode(thread1);
        if (!head_list) {
            head_list = newNode;
            tail_list = head_list;
        } else {
            tail_list->nextNode = newNode;
            tail_list = tail_list->nextNode;
        }
    }

    // Method to remove a thread from the container
    static void removeThread(Thread *thread1) {
        if (!head_list) return;

        threadnode *current = head_list, *previous = nullptr;
        while (current) {
            if (current->payloadThread == thread1) {
                if (current == head_list) {
                    head_list = current->nextNode;
                    if (current == tail_list) {
                        tail_list = nullptr;
                    }
                } else {
                    previous->nextNode = current->nextNode;
                    if (current == tail_list) {
                        tail_list = previous;
                    }
                }
                delete current;
                return;
            }
            previous = current;
            current = current->nextNode;
        }
    }

    // Check if the container is empty
    static bool isEmpty(){
    	if(head_list != nullptr) return false;
        return true;
    }

    // Get the front thread and remove it from the container
    static Thread* popFrontThread() {
        if (isEmpty()) return nullptr;

        threadnode* frontNode = head_list;
        Thread* frontThread = frontNode->payloadThread;
        head_list = frontNode->nextNode;
        if (head_list == nullptr) {
            tail_list = nullptr;
        }
        delete frontNode;
        return frontThread;
    }
};


