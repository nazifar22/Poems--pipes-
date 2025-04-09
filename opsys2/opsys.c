// Nazifa Nazrul Rodoshi - w8czne

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define MAX_LINE_LENGTH 1024
#define DATA_FILE "poems.txt"
#define NUM_BOYS 4
#define MSG_SIZE 2048
#define MSG_TYPE 1

typedef struct
{
    long msg_type;
    char poem[MSG_SIZE];
} message;

void childProcess(int pipe_fd[2], int msg_id);
void sendPoemsToBoy(int pipe_fd[2], char *poem1, char *poem2);
void receivePoemFromBoy(int msg_id);
void sendPoemToMama(int msg_id, char *poem);
void clearMessageQueue(int msg_id);
void readPoemByID(int id);
int getPoemCount();
void addPoem();
void listPoems();
void deletePoem(int id);
void modifyPoem(int id);
void performWateringRitual(int pipe_fd[2], int msg_id);

int getPoemCount()
{
    FILE *file = fopen(DATA_FILE, "r");
    if (!file)
    {
        perror("Error opening file");
        return 0;
    }

    int count = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"id\": "))
        {
            count++;
        }
    }

    fclose(file);
    return count;
}

void addPoem()
{
    char title[256], content[MAX_LINE_LENGTH], buffer[MAX_LINE_LENGTH];
    printf("Enter poem title: ");
    scanf(" %[^\n]", title);
    getchar();
    printf("Enter poem content (use ';' as a newline): ");
    scanf(" %[^\n]", content);
    getchar();

    for (char *p = content; *p; ++p)
    {
        if (*p == ';')
            *p = '\n';
    }

    FILE *file = fopen(DATA_FILE, "a");
    if (!file)
    {
        perror("Error opening file");
        return;
    }

    int id = getPoemCount() + 1;
    fprintf(file, "{ \"id\": %d, \"title\": \"%s\", \"content\": \"%s\" }\n", id, title, content);

    fclose(file);
    printf("Poem added successfully.\n");
}

void listPoems()
{
    FILE *file = fopen(DATA_FILE, "r");
    if (!file)
    {
        perror("Error opening file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }

    fclose(file);
}

void deletePoem(int id)
{
    FILE *file = fopen(DATA_FILE, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    if (!file || !tempFile)
    {
        perror("Error opening file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int found = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"id\": ") && atoi(strstr(line, "\"id\": ") + 6) == id)
        {
            found = 1;
            continue;
        }
        fprintf(tempFile, "%s", line);
    }

    fclose(file);
    fclose(tempFile);

    remove(DATA_FILE);
    rename("temp.txt", DATA_FILE);

    if (found)
    {
        printf("Poem with ID %d deleted successfully.\n", id);
    }
    else
    {
        printf("Poem with ID %d not found.\n", id);
    }
}

void modifyPoem(int id)
{
    FILE *file = fopen(DATA_FILE, "r");
    if (!file)
    {
        perror("Error opening file");
        return;
    }

    FILE *tempFile = fopen("temp.txt", "w");
    if (!tempFile)
    {
        perror("Error creating temporary file");
        fclose(file);
        return;
    }

    char line[MAX_LINE_LENGTH];
    int found = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"id\": ") && atoi(strstr(line, "\"id\": ") + 6) == id)
        {
            found = 1;
            char newTitle[256], newContent[MAX_LINE_LENGTH], modifiedLine[MAX_LINE_LENGTH];
            printf("Enter new poem title: ");
            scanf(" %[^\n]", newTitle);
            getchar();
            printf("Enter new poem content (use ';' as a newline): ");
            scanf(" %[^\n]", newContent);
            getchar();

            for (char *p = newContent; *p; ++p)
            {
                if (*p == ';')
                    *p = '\n';
            }

            snprintf(modifiedLine, sizeof(modifiedLine), "{ \"id\": %d, \"title\": \"%s\", \"content\": \"%s\" }\n", id, newTitle, newContent);
            fputs(modifiedLine, tempFile);
            continue;
        }

        fputs(line, tempFile);
    }

    fclose(file);
    fclose(tempFile);

    if (found)
    {
        remove(DATA_FILE);
        rename("temp.txt", DATA_FILE);
        printf("Poem with ID %d modified successfully.\n", id);
    }
    else
    {
        remove("temp.txt");
        printf("Poem with ID %d not found.\n", id);
    }
}

void performWateringRitual(int pipe_fd[2], int msg_id)
{

    printf("Starting...\n");
    int numPoems = getPoemCount();
    srand(time(NULL));

    if (numPoems < 2)
    {
        printf("Not enough poems available.\n");
        return;
    }
    else
    {
        printf("Found %d poems\n", numPoems);
    }

    int ids[numPoems];
    int idx = 0;
    FILE *file = fopen(DATA_FILE, "r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"id\": "))
        {
            ids[idx++] = atoi(strstr(line, "\"id\": ") + 6);
        }
    }
    fclose(file);

    int randIndex1 = rand() % numPoems;
    int randIndex2 = rand() % numPoems;
    while (randIndex2 == randIndex1)
    {
        randIndex2 = rand() % numPoems;
    }
    int poemId1 = ids[randIndex1];
    int poemId2 = ids[randIndex2];

    printf("Selected poems %d and %d.\n", poemId1, poemId2);

    file = fopen(DATA_FILE, "r");
    char poem1[MSG_SIZE] = {0}, poem2[MSG_SIZE] = {0};
    char *currentPoem = NULL;
    int currentPoemId = 0;
    int foundPoemCount = 0;

    while (fgets(line, sizeof(line), file) && foundPoemCount < 2)
    {
        if (strstr(line, "\"id\": "))
        {
            currentPoemId = atoi(strstr(line, "\"id\": ") + 6);
            continue;
        }

        if (currentPoemId == poemId1 || currentPoemId == poemId2)
        {
            currentPoem = (currentPoemId == poemId1) ? poem1 : poem2;
            if (strstr(line, "}"))
            {
                line[strlen(line) - 3] = '\0';
            }
            if (strlen(currentPoem) + strlen(line) < MSG_SIZE)
            {
                strcat(currentPoem, line);
            }

            if (strstr(line, "}"))
            {
                printf("Poem %d: %s\n", currentPoemId, currentPoem);
                foundPoemCount++;
            }
        }
    }
    fclose(file);

    pid_t pid = fork();

    if (pid == 0)
    {
        close(pipe_fd[1]);
        signal(SIGINT, handleSignal);
        childProcess(pipe_fd, msg_id);
        exit(0);
        printf("Child process finished.\n");
    }
    else if (pid > 0)
    {

        int bytes_written;

        bytes_written = write(pipe_fd[1], poem1, strlen(poem1) + 1);
        if (bytes_written == strlen(poem1) + 1)
        {
            printf("Sent the first poem.\n");
        }
        else
        {
            printf("Failed to send first poem.\n");
        }

        bytes_written = write(pipe_fd[1], poem2, strlen(poem2) + 1);
        if (bytes_written == strlen(poem2) + 1)
        {
            printf("Sent the second poem.\n");
        }
        else
        {
            printf("Failed to send second poem.\n");
        }

        close(pipe_fd[1]);

        wait(NULL);

        printf("Waiting to receive the message...\n");

        message msg;

        msgrcv(msg_id, &msg, sizeof(msg.poem), 0, 0);
        printf("Received the message.\n");

        printf("Bunny Boy recites the poem :\n%s\nMay I water!\n", msg.poem);
        printf("Bunny Boy waters the girls!\n");
    }
    else
    {
        perror("Failed to fork");
    }
}

void childProcess(int pipe_fd[2], int msg_id)
{
    close(pipe_fd[1]);
    char buffer[MSG_SIZE * 2] = {0};
    int bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1);

    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';

        char *currentPoem = buffer;
        int poemId = 0;

        while (currentPoem < buffer + bytes_read)
        {
            if (poemId == 0)
            {
                printf("Received poem1:\n%s\n", currentPoem);
            }
            else if (poemId == 1)
            {
                printf("Received poem2:\n%s\n", currentPoem);
            }
            currentPoem += strlen(currentPoem) + 1;
            poemId++;
        }

        currentPoem = buffer;
        srand(time(NULL));
        int chosenIndex = rand() % poemId;
        while (chosenIndex > 0)
        {
            currentPoem += strlen(currentPoem) + 1;
            chosenIndex--;
        }

        printf("Chosen poem:\n%s\n", currentPoem);

        message msg = {1, ""};
        char *chosenPoem = currentPoem;
        printf("Copying poem to message...\n");
        strncpy(msg.poem, chosenPoem, MSG_SIZE - 1);
        msg.poem[MSG_SIZE - 1] = '\0';

        clearMessageQueue(msg_id);
        printf("Message queue cleared.\n");

        printf("Sending message...\n");

        if (msgsnd(msg_id, &msg, strlen(msg.poem) + 1, IPC_NOWAIT) == -1)
        {
            perror("Failed to send message");
        }
        else
        {
            printf("Message sent successfully.\n");
        }
    }
    else
    {
        printf("Failed to read any poems.\n");
    }

    close(pipe_fd[0]);
}

void readPoemByID(int id)
{
    FILE *file = fopen(DATA_FILE, "r");
    if (!file)
    {
        perror("Error opening file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int found = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"id\": ") && atoi(strstr(line, "\"id\": ") + 6) == id)
        {
            found = 1;
            printf("Poem found:\n%s", line);
            break;
        }
    }

    if (!found)
    {
        printf("Poem with ID %d not found.\n", id);
    }

    fclose(file);
}

void clearMessageQueue(int msg_id)
{
    message buf;
    while (msgrcv(msg_id, &buf, sizeof(buf.poem), 0, IPC_NOWAIT) != -1)
    {
    }
    if (errno != ENOMSG)
    {
        perror("Error clearing message queue");
    }
}

void handleSignal(int signal)
{
    printf("Received signal %d\n", signal);
    cleanup();
    exit(0);
}

void cleanup()
{
    if (msg_queue_id != -1)
    {
        if (msgctl(msg_queue_id, IPC_RMID, NULL) == -1)
        {
            perror("Failed to remove message queue");
        }
        else
        {
            printf("Message queue removed successfully\n");
        }
    }
}

int main()
{
    int choice, id;
    do
    {
        printf("\nPoem Database\n");
        printf("1. Add Poem\n");
        printf("2. List Poems\n");
        printf("3. Delete Poem\n");
        printf("4. Modify Poem\n");
        printf("5. Quit\n");
        printf("6. Perform Watering Ritual\n");
        printf("7. Read Poem by ID\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            addPoem();
            break;
        case 2:
            listPoems();
            break;
        case 3:
            printf("Enter Poem ID to delete: ");
            scanf("%d", &id);
            deletePoem(id);
            break;
        case 4:
            printf("Enter Poem ID to modify: ");
            scanf("%d", &id);
            modifyPoem(id);
            break;
        case 5:
            printf("Quitting...\n");
            break;
        case 6:
        {
            int pipe_fd[2];
            if (pipe(pipe_fd) == -1)
            {
                perror("Failed to create pipe");
                break;
            }

            key_t key = ftok("msgq", 65);
            int msg_id = msgget(key, 0666 | IPC_CREAT);
            signal(SIGINT, handleSignal);
            if (msg_id == -1)
            {
                perror("Failed to create message queue");
                break;
            }

            performWateringRitual(pipe_fd, msg_id);
            break;
        }
        case 7:
            printf("Enter Poem ID to read: ");
            scanf("%d", &id);
            readPoemByID(id);
            break;
        default:
            printf("Invalid option, please try again.\n");
        }
    } while (choice != 5);

    cleanup();

    return 0;
}
