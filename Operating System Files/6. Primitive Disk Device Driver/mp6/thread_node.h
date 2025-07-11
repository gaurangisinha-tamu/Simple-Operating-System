
#include "thread.H"

class threadnode{
    Thread* thread;
    threadnode* nextNode;


public:
    static threadnode* head_list;
    static threadnode* tail_list;
    threadnode(){
        thread = nullptr;
        nextNode = nullptr;
    }

    threadnode(Thread *thread1){
        thread = thread1;
        nextNode = nullptr;
    }

    void enqueue(Thread *thread1){
        threadnode* newNode = new threadnode(thread1);
        if (!head_list) {
            head_list = newNode;
            tail_list = head_list;
        } else {
            tail_list->nextNode = newNode;
            tail_list = tail_list->nextNode;
        }
    }

    void delthread(Thread *thread1){
        // Early exit if either head or tail of the list is null, indicating an empty list.
if (head_list == nullptr || tail_list == nullptr) {
    return;
}

threadnode *current = head_list;
threadnode *prev = nullptr;

// Iterate over the linked list to find and remove the node containing the specified thread.
while (current != nullptr) {
    // Check if the current node contains the thread we're looking for.
    if (current->thread == thread1) {
        // Update the link to bypass the node to be deleted.
        if (prev != nullptr) {
            prev->nextNode = current->nextNode;
        } else {
            // Handle the case where the node to delete is the head of the list.
            head_list = current->nextNode;
        }

        // Delete the node and exit.
        delete current;
        return;
    }
    
    // Move to the next node.
    prev = current;
    current = current->nextNode;
}


    }

    static bool Isemp(){
        if(head_list != nullptr) return false;
        return true;
    }

    static Thread* popFront(){
        if (Isemp()) return nullptr;

        threadnode* frontNode = head_list;
        Thread* frontThread = frontNode->thread;
        head_list = frontNode->nextNode;
        if (head_list == nullptr) {
            tail_list = nullptr;
        }
        delete frontNode;
        return frontThread;
    }

};

