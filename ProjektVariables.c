#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAX 100
int info = false;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int count_CityA = 5;
int count_CityB = 6;

typedef struct Queue {
    char nameCar[MAX];
    struct Queue* next;
} Queue;

void insertQueue(Queue** head, const char* car) {
    Queue* newNode = malloc(sizeof(Queue));
    strncpy(newNode->nameCar, car, MAX - 1);
    newNode->nameCar[MAX - 1] = '\0';

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
        printf("Error: The list is empty.\n");
        return;
    }

    Queue* temp = *head;
    *head = (*head)->next;
    free(temp);
}

char* topQueue(Queue* head) {
    if (head == NULL) {
        printf("Error: The list is empty.\n");
        return NULL;
    }

    return head->nameCar;
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
            printf("%d: %s\n", i, current->nameCar);
            current = current->next;
            i++;
        }
    }
    printf("\n");
    return;
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

    while (1) {
        usleep(rand() % 1000000 + 100000); // od 0.1 do 1.1 sekundy

        while (*qA == NULL) {
            pthread_cond_wait(&cond, NULL);
        }

        char temp[MAX];
        sprintf(temp, "%s", topQueue(*qA));
        popQueue(qA);
        if(info==true){
            printf("---\n");
            printQueue(*qA);
            printQueue(*qB);
        } 
        printf("MiastoA-%d KolejkaA-%d --> [>> %s >>] <-- KolejkaB-%d MiastoB-%d\n", count_CityA, sizeQueue(*qA), temp, sizeQueue(*qB), count_CityB);

        insertQueue(qB, temp);

        pthread_cond_broadcast(&cond);
    }

    pthread_exit(NULL);
}

void* miasto_B(void* arg) {
    Queue** queues = *(Queue***)arg;
    Queue** qA = &queues[0];
    Queue** qB = &queues[1];

    while (1) {
        usleep(rand() % 1000000 + 100000); // od 0.1 do 1.1 sekundy

        while (*qB == NULL) {
            pthread_cond_wait(&cond, NULL);
        }

        char temp[MAX];
        sprintf(temp, "%s", topQueue(*qB));
        popQueue(qB);
        if(info==true){
            printf("---\n");
            printQueue(*qA);
            printQueue(*qB);
        }
        printf("MiastoA-%d KolejkaA-%d --> [<< %s <<] <-- KolejkaB-%d MiastoB-%d\n", count_CityA, sizeQueue(*qA), temp, sizeQueue(*qB), count_CityB);

        insertQueue(qA, temp);

        pthread_cond_broadcast(&cond);
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    int cars = 5;

    if (argc < 1 || argc > 4) {
        printf("Invaid arguments. Try -N [Number of cars] or -info\n");
        return 1; 
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-N") == 0) {
            i++;
            if (argv[i]==NULL || atoi(argv[i]) <= 0 ) {
                printf("Invalid number of cars. Insert value between 1 and %d\n", MAX);
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
    
    fprintf(stdout,"info: %d\n", info);
    fprintf(stdout,"cars: %d\n", cars);

    Queue* queueA = NULL;
    Queue* queueB = NULL;
    Queue** queues = malloc(2 * sizeof(Queue*));

    for (int i = 0; i < cars; i++) {
        char car[MAX];
        sprintf(car, "Car%d", i + 1);
        insertQueue(&queueA, car);
    }
    printf("Starting Queue:\n");
    printQueue(queueA);
    printQueue(queueB);
    printf("-----\n");

    queues[0] = queueA;
    queues[1] = queueB;

    pthread_t* tid1 = malloc(cars * sizeof(pthread_t));  // Alokacja pamieci
    pthread_t* tid2 = malloc(cars * sizeof(pthread_t));  // Alokacja pamieci

    for (int i = 0; i < cars; i++) {
        pthread_create(&tid1[i], NULL, miasto_A, &queues);
        pthread_create(&tid2[i], NULL, miasto_B, &queues);
    }

    for (int i = 0; i < cars; i++) {
        pthread_join(tid1[i], NULL);
        pthread_join(tid2[i], NULL);
    }

    free(tid1);
    free(tid2);
    free(queues);

    return 0;
}
