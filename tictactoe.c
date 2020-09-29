#include "tictactoe.h"

int checkWin(char board[ROWS][COLUMNS], char mark) {
    if (board[0][0] == board[0][1] && board[0][1] == board[0][2])  // row
        return (mark == board[0][0]) ? WIN : LOSE;
    else if (board[1][0] == board[1][1] && board[1][1] == board[1][2])  // row
        return (mark == board[1][0]) ? WIN : LOSE;
    else if (board[2][0] == board[2][1] && board[2][1] == board[2][2])  // row
        return (mark == board[2][0]) ? WIN : LOSE;
    else if (board[0][0] == board[1][0] && board[1][0] == board[2][0])  // column
        return (mark == board[0][0]) ? WIN : LOSE;
    else if (board[0][1] == board[1][1] && board[1][1] == board[2][1])  // column
        return (mark == board[0][1]) ? WIN : LOSE;
    else if (board[0][2] == board[1][2] && board[1][2] == board[2][2])  // column
        return (mark == board[0][2]) ? WIN : LOSE;
    else if (board[0][0] == board[1][1] && board[1][1] == board[2][2])  // diagonal
        return (mark == board[0][0]) ? WIN : LOSE;
    else if (board[2][0] == board[1][1] && board[1][1] == board[0][2])  // diagonal
        return (mark == board[2][0]) ? WIN : LOSE;
    else if (board[0][0] != '1' && board[0][1] != '2' && board[0][2] != '3' &&
             board[1][0] != '4' && board[1][1] != '5' && board[1][2] != '6' &&
             board[2][0] != '7' && board[2][1] != '8' && board[2][2] != '9')
        return DRAW;
    else return GAME_ON;
}

void printBoard(char board[ROWS][COLUMNS], char mark) {
    printf("\n\n\n\tCurrent TicTacToe Game\n\n");
    printf("Your mark is (%c)\n\n\n", mark);
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[0][0], board[0][1], board[0][2]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[1][0], board[1][1], board[1][2]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[2][0], board[2][1], board[2][2]);
    printf("     |     |     \n\n");
}

int parseGeneralError(uint8_t statusModifier) {
    if (statusModifier == OUT_OF_RESOURCES) {
        printf("Out of resources.\n");
        return 1;
    }
    else if (statusModifier == MALFORMED_REQUEST)
        printf("Malformed request.\n");
    else if (statusModifier == SERVER_SHUTDOWN)
        printf("Server shutdown.\n");
    else if (statusModifier == TIME_OUT)
        printf("Time out.\n");
    else if (statusModifier == TRY_AGAIN) {
        printf("Try again.\n");
        return 1;
    }
    else printf("Unknown error.\n");

    return 0;
}

void respondToInvalidRequest(
        int sd,
        int sendSequenceNum,
        uint8_t gameId) {

    uint8_t sb[BUFFER_SIZE] = {
            VERSION, 0, GAME_ERROR, MALFORMED_REQUEST, MOVE, gameId,
            (uint8_t) sendSequenceNum};

    sendBuffer(sd, sb);
}

int sendBuffer(int connected_sd, uint8_t buffer[BUFFER_SIZE]) {
    int writeResult = (int) write(connected_sd, buffer, BUFFER_SIZE);
    if (writeResult < 0) {
        perror("Failed to send data");
        return 0;
    }
    printf("SEND choice: %d status: %d statusModifier: %d "
           "gameType: %d gameId: %d sequenceNum: %d\n",
           buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6]);
    return 1;
}

int sendMoveWithChoice(
        int sd,
        uint8_t choice,
        uint8_t gameId,
        uint8_t sequenceNum,
        char board[ROWS][COLUMNS],
        char mark) {

    //update board and check win
    int row = (choice-1) / ROWS;
    int column = (choice-1) % COLUMNS;

    board[row][column] = mark;
    printBoard(board, mark);

    int result = checkWin(board, mark);

    //send msg
    int status = (result == GAME_ON) ? GAME_ON : GAME_COMPLETE;

    uint8_t sb[BUFFER_SIZE] = {
            VERSION, choice, (uint8_t) status, (uint8_t) result, MOVE,
            gameId, sequenceNum};

    if (sendBuffer(sd, sb) == 0) return GAME_ERROR;
    return GAME_ON;
}

// check if choice dosn't overlap
int isMoveValid(char board[ROWS][COLUMNS], int row, int column, int choice) {
    if (choice > 9 || choice < 1) return 0;
    if (board[row][column] == (choice+'0')) return 1;
    else return 0;
}

// initialize the board
void initBoard(char board[ROWS][COLUMNS]) {
    int i, j, count = 1;
    for (i=0; i<3; i++) {
        for (j = 0; j < 3; j++) {
            board[i][j] = (char) (count + '0');
            count++;
        }
    }
}

// check a given character is constructed by digits or not
int isDigitValid(const char *s) {
    while (*s) {
        if (*s >= '0' && *s <= '9') s++;
        else return 0;
    }
    return 1;
}


// validate a given string is valid ip address or not
int isIpValid(const char *ip) {
    char ip_str[29];
    strcpy(ip_str, ip);

    int num, dots = 0;
    char *ptr = strtok(ip_str, DELIM);

    if (ip == NULL || ptr == NULL) return 0;

    while (ptr) {
        if (!isDigitValid(ptr)) return 0;

        num = (uint8_t) strtol(ptr, NULL, 10);

        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL) dots++;
        }
        else return 0;
    }
    return (dots == 3) ? 1 : 0;
}

// return 1 if port number is valid, else return 0
int isPortNumValid(const char *portNum) {
    int len = (int) strlen(portNum);
    if (len <= 0) return 0;
    if (portNum[0] > '9' || portNum[0] < '1') return 0;
    for (int i = 1; i < strlen(portNum); i++)
        if (portNum[i] > '9' || portNum[i] < '0') return 0;
    return 1;
}

void u16_to_u8(uint16_t port_s, uint8_t port_array[2]) {
    port_array[0] = port_s >> 0;
    port_array[1] = port_s >> 8;
}

uint16_t u8_to_u16(const uint8_t port_array[2]) {
    uint16_t port_s = port_array[1] << 8;
    port_s = port_s ^ (uint16_t) port_array[0];
    return port_s;
}
