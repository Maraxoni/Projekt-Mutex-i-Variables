#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
//zdefiniowanie maksymalnej wartosci watkow
#define MAX 100
//deklaracja zmiennych
int info = false;
volatile int waitingA = 0;
volatile int waitingB = 0;
//inicjalizacja mutexow i condition variables
pthread_mutex_t mutexCity = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cityCondition = PTHREAD_COND_INITIALIZER;
//deklaracja struktury Queue
typedef struct Queue {
    int id;
    char city;
    struct Queue* next;
} Queue;

Queue* queue = NULL; //stworzenie globalnej kolejki queue w oparciu o strukture
//funkcja dodajaca 
void insertQueue(Queue** head, Queue* newNode) {
    if (newNode == NULL) {
        printf("Trying to add NULL\n");
        return;
    }

    newNode->next = NULL; // ustawienie wskaznika elementu nastepnego na NULL
    //sprawdzenie czy pierwszy element kolejki istnieje i w przeciwnym razie stworzenie nowego
    if (*head == NULL) {
        *head = newNode;
    } else {
        Queue* current = *head;
        //petla przechodzoca na koniec kolejki
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    
}
//funkcja usuwajaca pierwszy element i ustawiajacy element nastepny jako pierwszy
void popQueue(Queue** head) {
    if (*head == NULL) {
        printf("Error: The queue is empty.\n");
        return;
    }
    Queue* temp = *head;
    *head = (*head)->next;
    free(temp);
}
//funkcja zwracajaca id pierwszego elementu w kolejce
int topQueue(Queue* head) {
    if (head == NULL) {
        printf("Error: The queue is empty.\n");
        return 0;
    }
    int id = head->id;
    return id;
}
//funkcja wypisujaca zawartosc kolejki watkow
void printQueue(Queue* head) {
    printf("Queue:\n");
    //sprawdzanie czy kolejka jest pusta
    if (head == NULL) {
        printf("Empty.\n\n");
        return;
    } else {
        Queue* current = head;
        int i = 1;
        //iteracja po kolejce i wypisywanie id elementow oraz miast w ktorych jest watek
        while (current != NULL) {
            printf("%d: %d,%c\n", i, current->id, current->city);
            //ustawianie wskaznika obecnego elementu na wskaznik nastepnego
            current = current->next;
            i++;
        }
    }
    printf("\n");
}
//funkcja tworzaca dwie kolejki w oparciu o queue i miasta w ktorych sa watki i wypisujaca ich zawartosc
void printQueues(Queue* head) {
    if (head == NULL) {
        printf("Both queues empty.\n\n");
        return;
    } else {
        //tworzenie kolejek
        Queue* queueA = NULL;
        Queue* queueB = NULL;
        Queue* current = head;
        while (current != NULL) {
            Queue* temp = current; // tworzenie tymczasowego wskaznika
            if (current->city == 'A') {
                Queue* newNode = malloc(sizeof(Queue));
                newNode->id = current->id;
                newNode->city = current->city;
                insertQueue(&queueA, newNode);
            } else if (current->city == 'B') {
                Queue* newNode = malloc(sizeof(Queue));
                newNode->id = current->id;
                newNode->city = current->city;
                insertQueue(&queueB, newNode);
            }

            current = temp->next; // przechodzenie do nastepnego wezla
        }
        //wypisywanie kolejek
        printf("Queue A:\n");
        printQueue(queueA);
        printf("Queue B:\n");
        printQueue(queueB);
    }
    printf("\n");
}
//funkcja zwracajaca wielkosc kolejki w zaleznosci od miasta podanego jako argument
int sizeQueue(Queue* head, char check) {
    int size = 0;
    Queue* current = head;
    while (current != NULL) {
        if (current->city == check) {
            size++;
        }
        current = current->next;
    }
    return size;
}
//wykonywanie obliczen przez losowy czas
void cityWait() {
    for (int i = 0; i < 9999999999 * (rand() % 5); i++) {
        for (int j = 0; j < 9999999; j++) {
            int b = i + 1;
        }
    }
}
//funkcja watku zmieniajaca swoj stan z A na B i z powrotem
void* city(void* arg) {
    //przypisanie watkowi id oraz miasta podanego w argumencie
    Queue* data = (Queue*)arg;
    int idThread = data->id;
    char cityThread = data->city;
    while (1) {
        //wykonywanie obliczen zajmujacych czas
        cityWait();
        //zablokowanie mutexu 
        pthread_mutex_lock(&mutexCity);
        Queue* temp = malloc(sizeof(Queue));
        temp->id = idThread;
        temp->city = cityThread;
        //dodanie watku do kolejki
        insertQueue(&queue, temp);
        //odblokowanie mutexu
        pthread_mutex_unlock(&mutexCity);

        //zablokowanie mutexu
        pthread_mutex_lock(&mutexCity);
        //oczekiwanie az nastapi kolej wykonania watku
        while (topQueue(queue) != idThread) {
            pthread_cond_wait(&cityCondition, &mutexCity);
        }
        //usuniecie watku z listy
        popQueue(&queue);
        //wykonanie zamiany miasta watku i wypisanie zawartosci kolejek
        if (cityThread == 'A') {
            waitingA--; //zmniejszenie liczby samochodow znajdujacych sie w miescie A
            printf("CityA-%d QueueA-%d --> [>> %d >>] <-- QueueB-%d CityB-%d\n", waitingA-sizeQueue(queue, 'A'), sizeQueue(queue, 'A'), idThread, sizeQueue(queue, 'B'), waitingB-sizeQueue(queue, 'B'));
            cityThread = 'B';
            temp->city = 'B';
            waitingB++; //zwiekszenie liczby samochodow znajdujacych sie w miescie B
        } else {
            waitingB--; //zmniejszenie liczby samochodow znajdujacych sie w miescie B
            printf("CityA-%d QueueA-%d --> [<< %d <<] <-- QueueB-%d CityB-%d\n", waitingA-sizeQueue(queue, 'A'), sizeQueue(queue, 'A'), idThread, sizeQueue(queue, 'B'), waitingB-sizeQueue(queue, 'B'));
            cityThread = 'A';
            temp->city = 'A';
            waitingA++; //zwiekszenie liczby samochodow znajdujacych sie w miescie A
        }
        if(info == 1){
            printf("---\n");
            printQueues(queue);
        }
        //wyslanie sygnalu o zakonczeniu wykonania funkcji przez watek
        pthread_cond_broadcast(&cityCondition);
        //odblokowanie mutexu
        pthread_mutex_unlock(&mutexCity);
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    //domyslna watosc watkow
    int cars = 15;
    srand(time(NULL));
    //sprawdzenie ilosci argumentow
    if (argc < 1 || argc > 4) {
        printf("Invalid arguments. Try -N [Number of cars] or -info\n");
        return 1;
    }
    //sprawdzenie poprawnosci argumentow
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-N") == 0) {
            i++; //ustawnienie na sprawdzanie nastepnego elementu czy jest intem wiekszym od 0
            if (argv[i] == NULL || atoi(argv[i]) <= 0) {
                printf("Invalid number of cars. Insert a value between 1 and %d\n", MAX);
                return 1;
            }
            cars = atoi(argv[i]);
        } else if (strcmp(argv[i], "-info") == 0) {
            info = 1; //ustawienie wartosci info na 1
        } else {
            printf("Invalid argument: %s\n", argv[i]); //komunikat o zlym argumencie
            return 1;
        }
    }
    //ustawienie ilosci samochodow w miescie A na ilosc samochodow
    waitingA = cars;
    //wypisanie nowych zmiennych
    fprintf(stdout, "info: %d\n", info);
    fprintf(stdout, "cars: %d\n", cars);
    printf("Starting Queue:\n");
    printf("-----\n");
    //deklaracja tid watku
    pthread_t* tid = malloc(cars * sizeof(pthread_t));
    //stworzenie watkow i przydzielenie im unikalnego id oraz miasta startowego A
    for (int i = 0; i < cars; i++) {
        Queue* data = malloc(sizeof(Queue));
        data->id = i + 1;
        data->city = 'A';
        pthread_create(&tid[i], NULL, city, (void*)data);
    }
    //czekanie na wykonanie watkow
    for (int i = 0; i < cars; i++) {
        pthread_join(tid[i], NULL);
    }
    //zwolnienie pamieci z tid
    free(tid);
    return 0;
}
