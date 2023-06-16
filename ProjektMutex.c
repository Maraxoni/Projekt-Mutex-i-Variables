#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>

#define MAX 100

int info = false;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexQueueA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexQueueB = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;
sem_t bridgeSemaphore;

typedef struct Queue {
    int id;
    char city;
    struct Queue* next;
} Queue;

Queue* queue = NULL;
Queue* queuetemp = NULL;

void insertQueue(Queue** head, Queue* newNode, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex); // Acquire mutex before modifying the queue
    if (newNode == NULL) {
        printf("Trying to add NULL\n");
        pthread_mutex_unlock(mutex);
        return;
    } else if (*head == NULL) {
        *head = newNode;
        newNode->next = NULL;
    } else {
        Queue* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
        newNode->next = NULL;
    }

    pthread_mutex_unlock(mutex); // Release the mutex after modifying the queue
}
void insertQueues(Queue** head, Queue* newNode, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex); // Acquire mutex before modifying the queue
    if (newNode == NULL) {
        printf("Trying to add NULL\n");
        pthread_mutex_unlock(mutex);
        return;
    } else if (*head == NULL) {
        *head = newNode;
        newNode->next = NULL;
    } else {
        newNode->next = *head;
        *head = newNode;
    }

    pthread_mutex_unlock(mutex); // Release the mutex after modifying the queue
}
void popQueue(Queue** head, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex); // Acquire mutex before modifying the queue

    if (*head == NULL) {
        printf("Error: The queue is empty.\n");
        pthread_mutex_unlock(mutex); // Release the mutex if the queue is empty
        return;
    }

    Queue* temp = *head;
    *head = (*head)->next;
    free(temp);

    pthread_mutex_unlock(mutex); // Release the mutex after modifying the queue
}

int topQueue(Queue* head, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex); // Acquire mutex before accessing the queue

    if (head == NULL) {
        printf("Error: The queue is empty.\n");
        pthread_mutex_unlock(mutex); // Release the mutex if the queue is empty
        return 0;
    }

    int id = head->id;

    pthread_mutex_unlock(mutex); // Release the mutex after accessing the queue

    return id;
}

void printQueue(Queue* head) {
    printf("Queue:\n");
    if (head == NULL) {
        printf("Empty.\n\n");
        return;
    } else {
        Queue* current = head;
        int i = 1;
        while (current != NULL) {
            printf("%d: %d,%c\n", i, current->id, current->city);
            current = current->next;
            i++;
        }
    }
    printf("\n");
}

void printQueues(Queue* head) {
    printf("Queues:\n");
    if (head == NULL) {
        printf("Empty.\n\n");
        return;
    } else {
        Queue* queueA = NULL;
        Queue* queueB = NULL;
        Queue* current = head;
        while (current != NULL) {
            Queue* temp = current; // Store the next pointer before modifying the current node
            if (current->city == 'A') {
                Queue* newNode = malloc(sizeof(Queue));
                newNode->id = current->id;
                newNode->city = current->city;
                newNode->next = queueA;
                queueA = newNode;
            } else if (current->city == 'B') {
                Queue* newNode = malloc(sizeof(Queue));
                newNode->id = current->id;
                newNode->city = current->city;
                newNode->next = queueB;
                queueB = newNode;
            }

            current = temp->next; // Move to the next node
        }

        printf("Queue A:\n");
        printQueue(queueA);

        printf("Queue B:\n");
        printQueue(queueB);
    }
    printf("\n");
}

int sizeQueue(Queue* head, char check) {
    pthread_mutex_lock(&mutex); // Acquire mutex before accessing the queue

    int size = 0;
    Queue* current = head;
    while (current != NULL) {
        if (current->city == check) {
            size++;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&mutex); // Release the mutex after accessing the queue

    return size;
}

void cityWait() {
    for (int i = 0; i < 999999999 * (rand() % 3); i++) {
        for (int j = 0; j < 9999999; j++) {
            int b = i + 1;
        }
    }
}

void* city(void* arg) {
    Queue* data = (Queue*)arg;
    int idThread = data->id;
    char cityThread = data->city;
    while (1) {
        cityWait();

        sem_wait(&semaphore); // Acquire semaphore

        Queue* temp = malloc(sizeof(Queue));
        queuetemp = temp;
        temp->id = idThread;
        temp->city = cityThread;

        if (topQueue(queue, &mutex) == idThread) {
            popQueue(&queue, &mutex);
            printQueues(queue);
            if (cityThread == 'A') {
                printf("CityA-%d QueueA-%d --> [>> %d >>] <-- QueueB-%d CityB-%d\n", 0, sizeQueue(queue, 'A'), idThread, sizeQueue(queue, 'B'), 0);
                cityThread = 'B';
                temp->city = 'B';
            } else {
                printf("CityA-%d QueueA-%d --> [<< %d <<] <-- QueueB-%d CityB-%d\n", 0, sizeQueue(queue, 'A'), idThread, sizeQueue(queue, 'B'), 0);
                cityThread = 'A';
                temp->city = 'A';
            }

            insertQueue(&queue, temp, &mutex);
        }

        sem_post(&semaphore); // Release semaphore
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    int cars = 5;
    srand(time(NULL));
    if (argc < 2 || argc > 4) {
        printf("Invalid arguments. Try -N [Number of cars] or -info\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-N") == 0) {
            i++;
            if (argv[i] == NULL || atoi(argv[i]) <= 0) {
                printf("Invalid number of cars. Insert a value between 1 and %d\n", MAX);
                return 1;
            }
            cars = atoi(argv[i]);
        } else if (strcmp(argv[i], "-info") == 0) {
            info = 1;
        } else {
            printf("Invalid argument: %s\n", argv[i]);
            return 1;
        }
    }

    fprintf(stdout, "info: %d\n", info);
    fprintf(stdout, "cars: %d\n", cars);
    printf("Starting Queue:\n");
    printf("-----\n");

    sem_init(&semaphore, 0, 1);
    sem_init(&bridgeSemaphore, 0, 1);

    pthread_t* tid = malloc(cars * sizeof(pthread_t));

    for (int i = 0; i < cars; i++) {
        Queue* data = malloc(sizeof(Queue));
        data->id = i + 1;
        data->city = 'A';
        insertQueue(&queue, data, &mutex);
        pthread_create(&tid[i], NULL, city, (void*)data);
    }

    for (int i = 0; i < cars; i++) {
        pthread_join(tid[i], NULL);
    }

    free(tid);

    return 0;
}
