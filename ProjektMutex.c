#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX 100

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int count_CityA = 3;
int count_CityB = 7;

typedef struct Queue {
    char samochod[MAX];
    struct Queue* next;
} Queue;

void insertQueue(Queue** head, const char* samochod) {
    Queue* newNode = malloc(sizeof(Queue));
    strncpy(newNode->samochod, samochod, MAX - 1);
    newNode->samochod[MAX - 1] = '\0';

    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
    } else {
        Queue* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void popQueue(Queue** head) {
    if (*head == NULL) {
        printf("Error: The list is emptyP.\n");
        return;
    }

    Queue* temp = *head;
    *head = (*head)->next;
    free(temp);
}

char* topQueue(Queue* head) {
    if (head == NULL) {
        printf("Error: The list is emptyT.\n");
        return NULL;
    }

    return head->samochod;
}

void printQueue(Queue* head) {
    printf("Linked List:\n");
    if (head == NULL) {
        printf("Empty.\n");
    } else {
        Queue* current = head;
        while (current != NULL) {
            printf("Samochod: %s\n", current->samochod);
            current = current->next;
        }
    }
    printf("\n");
}

int sizeQueue(Queue* head) {
    int size = 0;
    Queue* current = head;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}

void* miasto_A(void* arg) {
    Queue** queues = *(Queue***)arg;
    Queue** qA = &queues[0];
    Queue** qB = &queues[1];
    
    if (*qA != NULL) {
        while (1) {
            if(*qA==NULL)
            {
                break;
            }
            usleep(rand() % 1000000 + 100000);
            pthread_mutex_lock(&mutex);
            printf("MiastoA-%d KolejkaA-%d --> [>> %s >>] <-- KolejkaB-%d MiastoB-%d\n", count_CityA, sizeQueue(*qA)-1, topQueue(*qA), sizeQueue(*qB), count_CityB);
            insertQueue(qB, topQueue(*qA));
            popQueue(qA);

            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(NULL);
}

void* miasto_B(void* arg) {
    Queue** queues = *(Queue***)arg;
    Queue** qA = &queues[0];
    Queue** qB = &queues[1];
    
    if (*qB != NULL) {
        while (1) {
            if(*qB==NULL)
            {
                break;
            }
            usleep(rand() % 1000000 + 100000);
            pthread_mutex_lock(&mutex);
            printf("MiastoA-%d KolejkaA-%d --> [<< %s <<] <-- KolejkaB-%d MiastoB-%d\n", count_CityA, sizeQueue(*qA), topQueue(*qB), sizeQueue(*qB)-1, count_CityB);
            
            insertQueue(qA, topQueue(*qB));
            popQueue(qB);
            
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    int num_cars = 5;

    if (argc > 1) {
        num_cars = atoi(argv[1]);
        if (num_cars > MAX) {
            printf("Too many cars (maximum value: %d)\n", MAX);
            return 1;
        }
    }

    Queue* queueA = NULL;
    Queue* queueB = NULL;
    Queue** queues = malloc(2 * sizeof(Queue*));

    for (int i = 0; i < num_cars; i++) {
        char car[20];
        sprintf(car, "Samochod%d", i + 1);
        insertQueue(&queueA, car);
    }
    popQueue(&queueA);
    insertQueue(&queueB, "SamochodA");
    printQueue(queueA);
    printQueue(queueB);
    queues[0] = queueA;
    queues[1] = queueB;

    pthread_t* tid1 = malloc(2 * sizeof(pthread_t));  // Poprawiony rozmiar alokacji
    pthread_t* tid2 = malloc(2 * sizeof(pthread_t));  // Poprawiony rozmiar alokacji


    for (int i = 0; i < num_cars; i++) {
        pthread_create(&tid1[i], NULL, miasto_A, &queues);
        pthread_create(&tid2[i], NULL, miasto_B, &queues);
    }

    for (int i = 0; i < num_cars; i++) {
        pthread_join(tid1[i], NULL);
        pthread_join(tid2[i], NULL);
    }

    

    pthread_mutex_destroy(&mutex);

    return 0;
}
