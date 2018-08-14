#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RIGHT_MIN_SIZE 64
#define LEFT_MIN_SIZE 64
#define ACCEPTING_STATES_MIN_SIZE 10
#define STATES_HASHMAP 16
#define INPUT_BUFFER 18

typedef enum {MOVE_RIGHT, MOVE_STAY, MOVE_LEFT} moveType;
typedef enum {false, true} bool;

typedef struct _transition {
	unsigned int startState;
	unsigned int endState;
	char inChar;
	char outChar;
	moveType move;
	struct _transition *next;
} transition;

typedef struct _tape {
	int tapeID;
	int leftCounter;
	int rightCounter;
	int leftMaxSize;
	int rightMaxSize;
	char *left;
	char *right;
	int reference_counter;
} tapeInfo;

typedef struct _configuration {
	int stateID;
	long moves;
	int index;
	bool inherited;
	tapeInfo *tape;
	struct _configuration *next;
} configuration;

int loadTape(tapeInfo **tapeP) {
	char parsing_ch;
	tapeInfo *tape = *tapeP;

	tape->reference_counter= 0;
	tape->leftCounter = 0;
	tape->leftMaxSize = 0;
	tape->left = NULL;
	tape->rightCounter = 0;
	tape->rightMaxSize = RIGHT_MIN_SIZE;
	tape->right = malloc(sizeof(char) * tape->rightMaxSize);

	for (int i = 0; i < tape->rightMaxSize; i++)
		tape->right[i] = '_';

	while ((parsing_ch = getc(stdin)) != EOF) {
		if (parsing_ch == '\r') {
			continue;
		} else if (parsing_ch == '\n') {
			break;
		}
		if (tape->rightCounter == tape->rightMaxSize) {
			tape->rightMaxSize = tape->rightMaxSize * 2;
			tape->right = realloc(tape->right, sizeof(char) * tape->rightMaxSize);
			for (int i = tape->rightCounter; i < tape->rightMaxSize; i++)
				tape->right[i] = '_';
		}
		tape->right[tape->rightCounter] = parsing_ch;
		tape->rightCounter = tape->rightCounter + 1;
	}
	if (parsing_ch == EOF)
		return 1;
	return 0;
}

int simulate(transition ***_chars, long maxSteps, unsigned int **acceptingStateP, int acceptingCounter) {
	int localQueueCounter = 0;
	int queueLength = 1;
	int eof = 0;
	int i;
	int mt_status = 0;
	int transitionCounter = 0;
	bool to_exit = false;
	bool to_clean = false;

	int tapeCounter = 0;

	tapeInfo *basicTape = malloc(sizeof(tapeInfo));
	transition ***chars = _chars;
	unsigned int *acceptingState = *acceptingStateP;

	eof = loadTape(&basicTape);
	if (eof && basicTape->rightCounter == 0) {
		free(basicTape->right);
		if (basicTape->left != NULL)
			free(basicTape->left);
		free(basicTape);
		return eof;
	}
	
	//for (int i = 0; i < basicTape->rightMaxSize; i++)
	//	printf("tape right [%d]:\t%c\n", i, basicTape->right[i]);

	configuration *queue = malloc(sizeof(configuration)); // Indica sempre la coda
	configuration *queueSuperHead = queue;
	configuration *queueHead = queue; // Indica sempre la testa
	configuration *queueTemp = NULL;

	queue->stateID = 0;
	queue->index = 0;
	queue->moves = 0;
	queue->inherited = false;
	queue->tape = malloc(sizeof(tapeInfo));
	queue->tape->tapeID = tapeCounter;
	tapeCounter++;
	queue->tape->reference_counter = 0;
	queue->tape->leftCounter = basicTape->leftCounter;
	queue->tape->rightCounter = basicTape->rightCounter;
	queue->tape->leftMaxSize = basicTape->leftMaxSize;
	queue->tape->rightMaxSize = basicTape->rightMaxSize;
	//printf("CREO NASTRO %d (a)\n", queue->tape->tapeID);

	queue->tape->left = NULL;
	queue->tape->right = malloc(sizeof(char) * queue->tape->rightMaxSize);
	for (i = 0; i < queue->tape->rightMaxSize; i++) 
		queue->tape->right[i] = basicTape->right[i];

	queue->next = NULL;
	
	while (queueLength > 0) {
		
		//printf("---\n");
		//for (int j = queueHead->tape->leftMaxSize; j > 0; j--)
		//	printf(" %c", queueHead->tape->left[j]);
		//printf("||");
		//for (int j = 0; j < queueHead->tape->rightMaxSize; j++) 
		//	printf(" %c", queueHead->tape->right[j]);
		//printf("\n");
		//printf("%d\t (moves: %ld, queue size: %d)\n", queueHead->stateID, queueHead->moves, queueLength);
		
		to_exit = false;
		to_clean = false;
		for (i = 0; i < acceptingCounter; i++) {
			if (queueHead->stateID == acceptingState[i]) {
				to_exit = true;
				break;
			}
		}
		if (to_exit) {
			mt_status = 1;

			while (queueHead != NULL) {
				queueTemp = queueHead;
				queueHead = queueHead->next;

				//printf("DIST NASTRO %d (b)\n", queueTemp->tape->tapeID);
				if (queueTemp->tape->left != NULL)
					free(queueTemp->tape->left);
				free(queueTemp->tape->right);
				free(queueTemp->tape);
				free(queueTemp);
			}

			queueLength = 0;
			break;
		}

		if (queueHead->moves >= maxSteps) {
			mt_status = 2;

			queueTemp = queueHead;
			queueHead = queueHead->next;

			//printf("DIST NASTRO %d (c)\n", queueTemp->tape->tapeID);
			if (queueTemp->tape->left != NULL)
				free(queueTemp->tape->left);
			free(queueTemp->tape->right);
			free(queueTemp->tape);
			free(queueTemp);

			queueLength--;
			continue;
		}

		transition *transitionCursor = NULL;
		transition *transitionTemp = NULL;
		char currentChar = 0;

		if (queueHead->index >= 0) {
			if (queueHead->index >= queueHead->tape->rightMaxSize) {
				queueHead->tape->rightMaxSize = queueHead->tape->rightMaxSize * 2;
				queueHead->tape->right = realloc(queueHead->tape->right, sizeof(char) * queueHead->tape->rightMaxSize);
				
				for (int i = queueHead->tape->rightCounter; i < queueHead->tape->rightMaxSize; i++)
					queueHead->tape->right[i] = '_';
			}

			currentChar = queueHead->tape->right[queueHead->index];
			if (chars[currentChar - 48] == NULL) {
				transitionCursor = NULL;
			} else {
				transitionCursor = chars[currentChar - 48][queueHead->stateID % STATES_HASHMAP];
			}
		} else {
			if (queueHead->tape->left == NULL) {
				queueHead->tape->leftMaxSize = LEFT_MIN_SIZE;
				queueHead->tape->left = malloc(sizeof(char) * queueHead->tape->leftMaxSize);
				for (int i = 0; i < queueHead->tape->leftMaxSize; i++)
					queueHead->tape->left[i] = '_';
			}

			if (abs(queueHead->index) >= queueHead->tape->leftMaxSize) {
				queueHead->tape->leftMaxSize = queueHead->tape->leftMaxSize * 2;
				queueHead->tape->left = realloc(queueHead->tape->left, sizeof(char) * queueHead->tape->leftMaxSize);
				
				for (int i = queueHead->tape->leftCounter; i < queueHead->tape->leftMaxSize; i++)
					queueHead->tape->left[i] = '_';
			}

			currentChar = queueHead->tape->left[abs(queueHead->index) - 1];
			if (chars[currentChar - 48] == NULL) {
				transitionCursor = NULL;
			} else {
				transitionCursor = chars[currentChar - 48][queueHead->stateID % STATES_HASHMAP];
			}
		}
		transitionCounter = 0;
		transitionTemp = transitionCursor;

		while (transitionTemp != NULL) {
			if (transitionTemp->startState == queueHead->stateID && transitionTemp->inChar == currentChar) {
				transitionCounter++;
			}
			transitionTemp = transitionTemp->next;
		}
		//printf("ELAB NASTRO (%d) (%d)\n", queueHead->tape->tapeID, transitionCounter);
		localQueueCounter = 0;
		while (transitionCounter > 0 && transitionCursor != NULL) {
			if ((transitionCursor->startState != queueHead->stateID) || (transitionCursor->inChar != currentChar)) {
				transitionCursor = transitionCursor->next;
				continue;
			}

			if (transitionCounter == 1) {
				// Se è uno stato pozzo, siamo già in U per quel ramo.
				if (transitionCursor->inChar == transitionCursor->outChar &&
					transitionCursor->move == MOVE_STAY &&
					queueHead->stateID == transitionCursor->endState) {
					mt_status = 2;

					queueTemp = queueHead;
					queueHead = queueHead->next;

					//printf("DIST NASTRO %d (d)\n", queueTemp->tape->tapeID);
					if (queueTemp->tape->left != NULL)
						free(queueTemp->tape->left);
					free(queueTemp->tape->right);
					free(queueTemp->tape);
					free(queueTemp);

					queueLength--;
					to_exit = true;
					break;
				}
			}

			queue->next = malloc(sizeof(configuration));
			queue = queue->next;

			queue->inherited = false;
			queue->stateID = transitionCursor->endState;
			queue->index = queueHead->index;
			queue->moves = queueHead->moves + 1;

			if (transitionCursor->inChar == transitionCursor->outChar && transitionCounter == 1) {
				queueHead->tape->reference_counter = 1;
				//printf("LINK NASTRO %d (e)\n", queueHead->tape->tapeID);
				queue->tape = queueHead->tape;
				queue->inherited = true;
			} else {
				if (transitionCounter > 1) {
					queueHead->tape->reference_counter = 0;

					queue->tape = malloc(sizeof(tapeInfo));
					queue->tape->tapeID = tapeCounter;
					tapeCounter++;
					queue->tape->reference_counter = 0;
					queue->tape->leftCounter = queueHead->tape->leftCounter;
					queue->tape->rightCounter = queueHead->tape->rightCounter;
					queue->tape->leftMaxSize = queueHead->tape->leftMaxSize;
					queue->tape->rightMaxSize = queueHead->tape->rightMaxSize;

					//printf("CREO NASTRO %d (f)\n", queue->tape->tapeID);
					if (queueHead->tape->left == NULL) {
						queue->tape->left = NULL;
					} else {
						queue->tape->left = malloc(sizeof(char) * queue->tape->leftMaxSize);
						for (i = 0; i < queue->tape->leftMaxSize; i++) 
							queue->tape->left[i] = queueHead->tape->left[i];
					}

					queue->tape->right = malloc(sizeof(char) * queue->tape->rightMaxSize);
					for (i = 0; i < queue->tape->rightMaxSize; i++) 
						queue->tape->right[i] = queueHead->tape->right[i];

				} else if (transitionCounter == 1) {
					queueHead->tape->reference_counter = 1;
					//printf("LINK NASTRO %d (g)\n", queueHead->tape->tapeID);
					queue->tape = queueHead->tape;
					queue->inherited = true;
				}

				if (queue->index >= 0) {
					queue->tape->right[queue->index] = transitionCursor->outChar;
				} else {
					queue->tape->left[abs(queue->index) - 1] = transitionCursor->outChar;
				}
			}

			if (transitionCursor->move == MOVE_RIGHT) {
				queue->index = queue->index + 1;
			} else if (transitionCursor->move == MOVE_LEFT) {
				queue->index = queue->index - 1;
			} else if (transitionCursor->move == MOVE_STAY) {
				queue->index = queue->index;
			}

			queue->next = NULL;
			queueLength++;

			transitionCursor = transitionCursor->next;
		}
		if (to_exit)
			continue;

		queueTemp = queueHead;
		queueHead = queueHead->next;
		if (transitionCounter == 0 || transitionCounter > 1) {
			//printf("DIST NASTRO %d (h)\n", queueTemp->tape->tapeID);
			if (queueTemp->tape->left != NULL)
				free(queueTemp->tape->left);
			free(queueTemp->tape->right);
			free(queueTemp->tape);
		}
		free(queueTemp);

		// fine while transitioncursor

		//printf("o\n");

		queueLength--;
		//printf("---\n");
	}

	if (mt_status == 2) {
		printf("U\n");
	} else {
		printf("%d\n", mt_status);
	}

	
	free(basicTape->right);
	if (basicTape->left != NULL)
		free(basicTape->left);
	free(basicTape);

	return eof;
}

int main(int argc, char const *argv[]) {
	int  parsing_step,
		 parsing_index,
		 i,
		 k;
	char parsing_line[INPUT_BUFFER],
		 parsing_ch,
		 parsing_move;

	unsigned int maxSteps,
				 parsing_state;

	transition **chars[76];
	for (i = 0; i < 76; i++)
		chars[i] = NULL;

	int acceptingSize = ACCEPTING_STATES_MIN_SIZE;
	int acceptingCounter = 0;
	unsigned int *acceptingState = malloc(sizeof(int) * acceptingSize);


	parsing_step = 0;
	parsing_index = 0;
	for (i = 0; i < INPUT_BUFFER; i++) 
		parsing_line[i] = (char) 0;

	while ((parsing_ch = getc(stdin)) != EOF) {
		if (parsing_ch != '\n') {
			if (parsing_ch == '\r') continue;
			parsing_line[parsing_index] = parsing_ch;
			parsing_index++;
		} else {
			if ((parsing_step == 0) && (strcmp(parsing_line, "tr") == 0)) {
				parsing_step = 1;
			} else if (parsing_step == 1) {
				transition *node = malloc(sizeof(transition));
 				node->next = NULL;
				if (sscanf(parsing_line, "%d%*c%c%*c%c%*c%c%*c%d", &(node->startState),
																   &(node->inChar),
																   &(node->outChar),
																   &parsing_move,
																   &(node->endState)) == 5) {
					if (parsing_move == 'R')
						node->move = MOVE_RIGHT;
					else if (parsing_move == 'S')
						node->move = MOVE_STAY;
					else if (parsing_move == 'L')
						node->move = MOVE_LEFT;
					int stateHash = node->startState % STATES_HASHMAP;
					if (chars[node->inChar-48] == NULL) {
						transition **table = malloc(STATES_HASHMAP * sizeof(transition*));
						for (i = 0; i < STATES_HASHMAP; i++)
							table[i] = NULL;

						table[stateHash] = node;
						chars[node->inChar-48] = table;
					} else {
						if (chars[node->inChar-48][stateHash] == NULL) {
							chars[node->inChar-48][stateHash] = node;
						} else {
							node->next = chars[node->inChar-48][stateHash];
							chars[node->inChar-48][stateHash] = node;
						}
					}
				} else if (strcmp(parsing_line, "acc") == 0) {
					free(node);
					parsing_step = 2;
				}
			} else if (parsing_step == 2) {
				if (sscanf(parsing_line, "%u", &parsing_state) == 1) {
					if (acceptingCounter >= acceptingSize) {
						acceptingSize = acceptingSize * 2;
						acceptingState = realloc(acceptingState, acceptingSize);
					}
				
					acceptingState[acceptingCounter] = parsing_state;
					acceptingCounter++;
				} else if (strcmp(parsing_line, "max") == 0) {
					parsing_step = 3;
				}
			} else if (parsing_step == 3) {
				if (sscanf(parsing_line, "%u", &maxSteps) == 1) {

				} else if (strcmp(parsing_line, "run") == 0) {
					parsing_step = 4;
					break;
				}
			}

			parsing_index = 0;
			for (i = 0; i < INPUT_BUFFER; i++) 
				parsing_line[i] = (char) 0;
		}
	}

	
	/*printf(">> Riepilogo\n");
	for(i = 0; i < 76; i++) {
		if (chars[i] == NULL) continue;

		printf("\tChar: %c\n", i+48);
		for (k = 0; k < 16; k++) {
			if (chars[i][k] == NULL) continue;

			transition *transitionCursor = chars[i][k];
			while (transitionCursor != NULL) {
				printf("\t\t %d -> %d [%c|%c|%d]\n",
						transitionCursor->startState,
						transitionCursor->endState,
						transitionCursor->inChar,
						transitionCursor->outChar,
						transitionCursor->move);
				transitionCursor = transitionCursor->next;
			}
		}
	}
	printf("\tStati d'accettazione:\n");
	for (i = 0; i < acceptingCounter; i++) {
		printf("\t\t- %d\n", acceptingState[i]);
	}*/

	/*
	int lineToSkip = 0;
	while (lineToSkip > 0) {
		while ((parsing_ch = getc(stdin)) != EOF) {
			if (parsing_ch == '\n') {
				break;
			}
		}
		lineToSkip -= 1;
	}
	simulate(chars, maxSteps, &acceptingState, acceptingCounter);
	*/
	while(simulate(chars, maxSteps, &acceptingState, acceptingCounter) != 1);

	free(acceptingState);
	transition *transitionTemp, *transitionCursor;
	for (i = 0; i < 76; i++) {
		if (chars[i] == NULL) continue;
		for (k = 0; k < 16; k++) {
			if (chars[i][k] == NULL) continue;
			transitionCursor = chars[i][k];
			while (transitionCursor != NULL) {
				transitionTemp = transitionCursor;
				transitionCursor = transitionCursor->next;
				free(transitionTemp);
			}
		}
		free(chars[i]);
	}

	return 0;
}