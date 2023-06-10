#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX 100

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bridge_empty = PTHREAD_COND_INITIALIZER;

int count_CityA = 0;
int count_CityB = 0;

typedef struct {
    int id;
    char samochod[MAX];
    struct Queue *next;
} Queue;

// Function to insert a new node at the beginning of the linked list
void insert(Queue** head, const char* samochod) {
    // Create a new node
    Queue* newNode = (Queue*)malloc(sizeof(Queue));
    strncpy(newNode->samochod, samochod, MAX - 1);
    newNode->samochod[MAX - 1] = '\0';

    // Set the next pointer of the new node to the current head
    newNode->next = *head;
    // Update the head to point to the new node
    *head = newNode;
}

// Function to remove the last node from the linked list and return its data
int pop(Queue** head) {
    // If the list is empty
    if (*head == NULL) {
        printf("Error: The list is empty.\n");
        return -1;
    }

    // If there is only one node in the list
    if ((*head)->next == NULL) {
        int id = (*head)->id;
        free(*head);
        *head = NULL;
        return id;
    }
    // Traverse the list until the second last node
    Queue* current = *head;
    while (current->next->next != NULL) {
        current = current->next;
    }
    // Remove the last node
    Queue* lastNode = current->next;
    int id = lastNode->id;
    free(lastNode);
    current->next = NULL;
    return id;
}
// Function to print the linked list
void printQueue(Queue* head) {
    Queue* current = head;

    printf("Linked List:\n");
    while (current != NULL) {
        printf("ID: %d, Samochod: %s\n", current->id, current->samochod);
        current = current->next;
    }
    printf("\n");
}

void* miasto_A(void* arg) {
    int car_id = *(int*)arg;
    while (1) {
        usleep(rand() % 1000000 + 100000); // Czas między kolejnymi wywołaniami

        pthread_mutex_lock(&mutex);
        side_a_count++;

        while (on_bridge > 0) {
            pthread_cond_wait(&bridge_empty, &mutex);
        }

        side_a_count--;

        pthread_cond_broadcast(&bridge_empty);
        pthread_mutex_unlock(&mutex);

        printf("A-%d %d %d --> [>> %d >>] <-- %d B\n", car_id, side_a_count, on_bridge, car_id, side_b_count);
    }

    pthread_exit(NULL);
}

void* miasto_B(void* arg) {
    int car_id = *(int*)arg;
    while (1) {
        usleep(rand() % 1000000 + 100000); // Czas między kolejnymi wywołaniami

        pthread_mutex_lock(&mutex);
        side_b_count++;

        while (on_bridge > 0) {
            pthread_cond_wait(&bridge_empty, &mutex);
        }

        side_b_count--;

        pthread_cond_broadcast(&bridge_empty);
        pthread_mutex_unlock(&mutex);

        printf("A-%d %d %d --> [>> %d >>] <-- %d B\n", side_a_count, on_bridge, car_id, side_b_count, car_id);
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {

    int num_cars = 5; // Domyślna liczba samochodów

    if (argc > 1) {
        num_cars = atoi(argv[1]);
        if (num_cars > MAX_CARS) {
            printf("Zbyt wiele samochodów (maksymalna liczba: %d)\n", MAX_CARS);
            return 1;
        }
    }
    Queue* queueA = NULL;
    Queue* queueB = NULL;

    for (int i = 0; i < num_cars; i++) {
        char car[20];
        sprintf(samochod, "Samochód%d", i);
        insert(&queueA, car);
    }

    srand(time(NULL));

    for (int i = 0; i < num_cars; i++) {
        pthread_create(&cars_A[i], NULL, miasto_A, &car_ids[i]);
        pthread_create(&cars_B[i], NULL, miasto_B, &car_ids[i]);
    }

    for (int i = 0; i < num_cars; i++) {
        pthread_join(cars_A[i], NULL);
        pthread_join(cars_B[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&bridge_empty);

    return 0;
}
