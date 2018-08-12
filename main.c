#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	int leftCounter;
	int rightCounter;
	int leftMaxSize;
	int rightMaxSize;
	int reference_counter;
	char *left;
	char *right;
} tapeInfo;

typedef struct _configuration {
	int stateID;
	long moves;
	int index;
	tapeInfo *tape;
	struct _configuration *next;
} configuration;

int loadTape(tapeInfo **tapeP) {
	char parsing_ch;
	tapeInfo *tape = *tapeP;

	tape->leftCounter = 0;
	tape->leftMaxSize = 0;
	tape->left = NULL;
	tape->rightCounter = 0;
	tape->rightMaxSize = 64;
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
			tape->rightMaxSize = tape->rightMaxSize + 2;
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
	bool deterministic = false;
	bool to_exit = false;

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
	configuration *queueCursor = queue; // Indica sempre la testa
	configuration *queueTemp = NULL;

	queue->stateID = 0;
	queue->index = 0;
	queue->moves = 0;
	queue->tape = malloc(sizeof(tapeInfo));
	queue->tape->reference_counter = 0;
	queue->tape->leftCounter = basicTape->leftCounter;
	queue->tape->rightCounter = basicTape->rightCounter;
	queue->tape->leftMaxSize = basicTape->leftMaxSize;
	queue->tape->rightMaxSize = basicTape->rightMaxSize;

	queue->tape->left = NULL;
	queue->tape->right = malloc(sizeof(char) * queue->tape->rightMaxSize);
	for (i = 0; i < queue->tape->rightMaxSize; i++) 
		queue->tape->right[i] = basicTape->right[i];

	queue->next = NULL;
	
	while (queueLength > 0) {
		
		//printf("---\n");
		//for (int j = queueCursor->tape->leftMaxSize; j > 0; j--)
		//	printf(" %c", queueCursor->tape->left[j]);
		//printf("||");
		//for (int j = 0; j < queueCursor->tape->rightMaxSize; j++) 
		//	printf(" %c", queueCursor->tape->right[j]);
		//printf("\n");
		//printf("%d\t (moves: %ld, queue size: %d)\n", queueCursor->stateID, queueCursor->moves, queueLength);
		

		to_exit = false;
		for (i = 0; i < acceptingCounter; i++) {
			if (queueCursor->stateID == acceptingState[i]) {
				mt_status = 1;

				while (queueCursor != NULL) {
					queueTemp = queueCursor;
					queueCursor = queueCursor->next;
					if (queueTemp->tape->left != NULL)
						free(queueTemp->tape->left);
					free(queueTemp->tape->right);
					free(queueTemp->tape);
					free(queueTemp);
				}

				queueLength = 0;
				to_exit = true;
				break;
			}
		}
		if (to_exit) break;
		//printf("a\n");

		if (queueCursor->moves >= maxSteps) {
			mt_status = 2;

			queueTemp = queueCursor;
			queueCursor = queueCursor->next;
			free(queueTemp->tape->left);
			free(queueTemp->tape->right);
			free(queueTemp->tape);
			free(queueTemp);
			queueLength--;
			continue;
		}
		//printf("b\n");

		transition *transitionCursor = NULL;
		transition *transitionTemp = NULL;
		char currentChar = 0;

		if (queueCursor->index >= 0) {
			//printf("position: %c (%d) | index: %d\n", queueCursor->tape->right[queueCursor->index], queueCursor->tape->right[queueCursor->index], queueCursor->index);
			//printf("rightMaxSize: %d | counter: %d\n", queueCursor->tape->rightMaxSize, queueCursor->tape->rightCounter);
			if (queueCursor->index >= queueCursor->tape->rightMaxSize) {
				queueCursor->tape->rightMaxSize = queueCursor->index + 2;
				queueCursor->tape->right = realloc(queueCursor->tape->right, sizeof(char) * queueCursor->tape->rightMaxSize);
				
				for (int i = queueCursor->tape->rightCounter; i < queueCursor->tape->rightMaxSize; i++)
					queueCursor->tape->right[i] = '_';
			}
			currentChar = queueCursor->tape->right[queueCursor->index];
			transitionCursor = chars[queueCursor->tape->right[queueCursor->index] - 48][queueCursor->stateID % STATES_HASHMAP];
		} else {
			if (queueCursor->tape->left == NULL) {
				queueCursor->tape->leftMaxSize = 64;
				queueCursor->tape->left = malloc(sizeof(char) * queueCursor->tape->leftMaxSize);
				for (int i = 0; i < queueCursor->tape->leftMaxSize; i++)
					queueCursor->tape->left[i] = '_';
			}

			if (abs(queueCursor->index) >= queueCursor->tape->leftMaxSize) {
				queueCursor->tape->leftMaxSize = abs(queueCursor->index) + 2;
				queueCursor->tape->left = realloc(queueCursor->tape->left, sizeof(char) * queueCursor->tape->leftMaxSize);
				
				for (int i = queueCursor->tape->leftCounter; i < queueCursor->tape->leftMaxSize; i++)
					queueCursor->tape->left[i] = '_';
			}
			currentChar = queueCursor->tape->left[abs(queueCursor->tape->leftCounter + queueCursor->index) - 1];
			transitionCursor = chars[queueCursor->tape->left[abs(queueCursor->tape->leftCounter + queueCursor->index) - 1] - 48][queueCursor->stateID % STATES_HASHMAP];
		}
		transitionCounter = 0;
		transitionTemp = transitionCursor;
		//printf("c\n");

		while (transitionTemp != NULL) {
			//printf("TS: %d, QS: %d, TC: %c, CC: %c\n", transitionTemp->startState, queueCursor->stateID, transitionTemp->inChar, currentChar);
			if (transitionTemp->startState == queueCursor->stateID && transitionTemp->inChar == currentChar) {
				transitionCounter++;
			}
			transitionTemp = transitionTemp->next;
		}

		//printf("d %d\n",transitionCounter);
		deterministic = false;
		localQueueCounter = 0;
		while (transitionCursor != NULL) {
			if (transitionCursor->startState != queueCursor->stateID) {
				transitionCursor = transitionCursor->next;
				continue;
			}
			//printf("e\n");
			if (queueCursor->index >= 0) {
				if (transitionCursor->inChar != queueCursor->tape->right[queueCursor->index]) {
					transitionCursor = transitionCursor->next;
					continue;
				}
			} else {
				if (transitionCursor->inChar != queueCursor->tape->left[abs(queueCursor->tape->leftCounter + queueCursor->index) - 1]) {
					transitionCursor = transitionCursor->next;
					continue;
				}
			}
			//printf("f\n");
			if (transitionCounter == 1) {
				//printf("è deter, pozzo?\n");
				// Se è uno stato pozzo, siamo già in U per quel ramo.
				if (transitionCursor->inChar == transitionCursor->outChar &&
					transitionCursor->move == MOVE_STAY &&
					queueCursor->stateID == transitionCursor->endState) {
					//printf("è un pozzo\n");
					mt_status = 2;

					queueTemp = queueCursor;
					queueCursor = queueCursor->next;
					free(queueTemp->tape->left);
					free(queueTemp->tape->right);
					free(queueTemp->tape);
					free(queueTemp);
					queueLength--;
					continue;
				}
			}
			//printf("g\n");
			queue->next = malloc(sizeof(configuration));
			queue = queue->next;

			queue->stateID = transitionCursor->endState;
			queue->index = queueCursor->index;
			queue->moves = queueCursor->moves + 1;

			if (transitionCounter > 1) {
				localQueueCounter = 0;
				if (queueCursor->tape->reference_counter > 0)
					queueCursor->tape->reference_counter--;
				
				queue->tape = malloc(sizeof(tapeInfo));
				queue->tape->leftCounter = queueCursor->tape->leftCounter;
				queue->tape->rightCounter = queueCursor->tape->rightCounter;
				queue->tape->leftMaxSize = queueCursor->tape->leftMaxSize;
				queue->tape->rightMaxSize = queueCursor->tape->rightMaxSize;

				if (queueCursor->tape->left == NULL) {
					queue->tape->left = NULL;
				} else {
					queue->tape->left = malloc(sizeof(char) * queue->tape->leftMaxSize);
					for (i = 0; i < queue->tape->leftMaxSize; i++) 
						queue->tape->left[i] = queueCursor->tape->left[i];
				}

				queue->tape->right = malloc(sizeof(char) * queue->tape->rightMaxSize);
				for (i = 0; i < queue->tape->rightMaxSize; i++) 
					queue->tape->right[i] = queueCursor->tape->right[i];
				queue->tape->reference_counter = 0;
			} else if (transitionCounter == 1) {
				deterministic = true;
				queueCursor->tape->reference_counter++;
				queue->tape = queueCursor->tape;
			}

			if (queue->index >= 0) {
				if (transitionCursor->inChar == queue->tape->right[queue->index])
					queue->tape->right[queue->index] = transitionCursor->outChar;
			} else {
				if (transitionCursor->inChar == queue->tape->left[abs(queue->tape->leftCounter + queue->index) - 1])
					queue->tape->left[abs(queue->tape->leftCounter + queue->index) - 1] = transitionCursor->outChar;
			}

			if (transitionCursor->move == MOVE_RIGHT) {
				queue->index = queue->index + 1;
			} else if (transitionCursor->move == MOVE_LEFT) {
				queue->index = queue->index - 1;
			} else if (transitionCursor->move == MOVE_STAY) {
				queue->index = queue->index;
			}

			queue->next = NULL;
			localQueueCounter++;
			queueLength++;

			transitionCursor = transitionCursor->next;
		}

		queueTemp = queueCursor;
		queueCursor = queueCursor->next;
		//if (queueTemp->tape->reference_counter > 0)
		//	queueTemp->tape->reference_counter--;
		
		//printf("REF COUNTER: %d, mt_status: %d, queue_len: %d, localQueueCounter: %d\n", queueTemp->tape->reference_counter, mt_status, queueLength, localQueueCounter);
		if (queueTemp->tape->reference_counter == 0 || localQueueCounter == 0) {
			//printf("Free nastro!!\n");
			free(queueTemp->tape->left);
			free(queueTemp->tape->right);
			free(queueTemp->tape);
		}
		free(queueTemp);
		queueLength--;
	}

	while (queueCursor != NULL) {
		queueTemp = queueCursor;
		queueCursor = queueTemp->next;
		if (queueTemp->tape->left != NULL)
			free(queueTemp->tape->left);
		free(queueTemp->tape->right);
		free(queueTemp->tape);
		free(queueTemp);
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

	/*int lineToSkip = 2;
	while (lineToSkip > 0) {
		while ((parsing_ch = getc(stdin)) != EOF) {
			if (parsing_ch == '\n') {
				break;
			}
		}
		lineToSkip -= 1;
	}*/
	//simulate(chars, maxSteps, &acceptingState, acceptingCounter);
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