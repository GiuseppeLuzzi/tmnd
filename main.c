#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BENCH
	#include <time.h>
#endif

#define DEBUG
#define INPUT_BUFFER 18
#define CHUNK_SIZE 15

typedef enum {MOVE_RIGHT, MOVE_STAY, MOVE_LEFT} moveType;
typedef enum {false, true} bool;

typedef struct _transition {
	int startState;
	int endState;
	char inChar;
	char outChar;
	moveType move;
	struct _transition *next;
} transition;

typedef struct _state {
	bool final;
	transition *keys[256];
	int tr_counter[256];
} state;

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

char m2c(moveType move) {
	if (move == MOVE_RIGHT) return 'R';
	if (move == MOVE_LEFT)  return 'L';
	if (move == MOVE_STAY)  return 'S';
	return '-';
}

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


/* return values: 0 - not accepted | 1 - accepted | 2 - undefined */
int simulate(state ***states, long maxSteps) {
	int localQueueCounter = 0;
	int queueLength = 1;
	int eof = 0;
	int i;
	int mt_status = 0;
	bool deterministic = false;

	tapeInfo *basicTape = malloc(sizeof(tapeInfo));
	state **statesCursor = *states;

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
		/*
		printf("---\n");
		for (int j = queueCursor->tape->leftMaxSize; j > 0; j--)
			printf(" %c", queueCursor->tape->left[j]);
		printf("||");
		for (int j = 0; j < queueCursor->tape->rightMaxSize; j++) 
			printf(" %c", queueCursor->tape->right[j]);
		printf("\n");
		printf("%d\t (final: %d, moves: %d, queue size: %d)\n", queueCursor->stateID, statesCursor[queueCursor->stateID]->final, queueCursor->moves, queueLength);
		*/

		if (statesCursor[queueCursor->stateID]->final == true) {
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

			break;
		} 

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

		transition *transitionCursor = NULL;
		if (queueCursor->index >= 0) {
			//printf("position: %c (%d) | index: %d\n", queueCursor->tape->right[queueCursor->index], queueCursor->tape->right[queueCursor->index], queueCursor->index);
			//printf("rightMaxSize: %d | counter: %d\n", queueCursor->tape->rightMaxSize, queueCursor->tape->rightCounter);
			if (queueCursor->index >= queueCursor->tape->rightMaxSize) {
				queueCursor->tape->rightMaxSize = queueCursor->index * 2;
				queueCursor->tape->right = realloc(queueCursor->tape->right, sizeof(char) * queueCursor->tape->rightMaxSize);
				
				for (int i = queueCursor->tape->rightCounter; i < queueCursor->tape->rightMaxSize; i++)
					queueCursor->tape->right[i] = '_';
			}

			transitionCursor = (*states)[queueCursor->stateID]->keys[queueCursor->tape->right[queueCursor->index]];
		} else {
			if (queueCursor->tape->left == NULL) {
				queueCursor->tape->leftMaxSize = 64;
				queueCursor->tape->left = malloc(sizeof(char) * queueCursor->tape->leftMaxSize);
				for (int i = 0; i < queueCursor->tape->leftMaxSize; i++)
					queueCursor->tape->left[i] = '_';
			}

			if (abs(queueCursor->index) >= queueCursor->tape->leftMaxSize) {
				queueCursor->tape->leftMaxSize = queueCursor->tape->leftMaxSize * 2;
				queueCursor->tape->left = realloc(queueCursor->tape->left, sizeof(char) * queueCursor->tape->leftMaxSize);
				
				for (int i = queueCursor->tape->leftCounter; i < queueCursor->tape->leftMaxSize; i++)
					queueCursor->tape->left[i] = '_';
			}

			transitionCursor = (*states)[queueCursor->stateID]->keys[queueCursor->tape->left[abs(queueCursor->tape->leftCounter + queueCursor->index) - 1]];
		}

		deterministic = false;
		localQueueCounter = 0;
		while (transitionCursor != NULL) {

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

			if ((*states)[queueCursor->stateID]->tr_counter[transitionCursor->inChar] == 1) {
				// Se è uno stato pozzo, siamo già in U per quel ramo.
				if (transitionCursor->inChar == transitionCursor->outChar &&
					transitionCursor->move == MOVE_STAY &&
					queueCursor->stateID == transitionCursor->endState) {
					
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

			queue->next = malloc(sizeof(configuration));
			queue = queue->next;

			queue->stateID = transitionCursor->endState;
			queue->index = queueCursor->index;
			queue->moves = queueCursor->moves + 1;

			if ((*states)[queueCursor->stateID]->tr_counter[transitionCursor->inChar] > 1) {
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
			} else if ((*states)[queueCursor->stateID]->tr_counter[transitionCursor->inChar] == 1) {
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
	#ifdef BENCH
		clock_t begin = clock();
	#endif
	
	int statesSize;
	int lastStatesSize;
	long maxSteps;

	int parsing_step;
	int parsing_index;
	int parsing_state;
	char parsing_ch;
	char parsing_move;
	char parsing_line[INPUT_BUFFER];
	
	int i;
	
	statesSize = 256;
	lastStatesSize= 256;

	state **states = malloc(statesSize * sizeof(state*));

	for (i = 0; i < statesSize; i++) 
		states[i] = NULL;

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
				transition* node = (transition*) malloc(sizeof(transition));
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
					
					/* Ingrandisco l'array quanto necessario */
					while ((node->startState > (statesSize - 1)) || (node->endState > (statesSize - 1))) {
						statesSize *= 2;
					}
					if (lastStatesSize != statesSize) {
						states = (state**) realloc(states, statesSize * sizeof(state*));
						for (i = lastStatesSize; i < statesSize; i++)
							states[i] = NULL;

						lastStatesSize = statesSize;
					}

					/* Aggiungo lo stato all'array linked-list*/
					if (states[node->startState] == NULL) {
						state* state_node = (state*) malloc(sizeof(state));
						state_node->final = false;
						for (i = 0; i < 256; i++)
							state_node->keys[i] = NULL;
						for (i = 0; i < 256; i++)
							state_node->tr_counter[i] = 0;
						states[node->startState] = state_node;
					}

					if (states[node->endState] == NULL) {
						state* state_node = (state*) malloc(sizeof(state));
						state_node->final = false;
						for (i = 0; i < 256; i++)
							state_node->keys[i] = NULL;
						for (i = 0; i < 256; i++)
							state_node->tr_counter[i] = 0;
						states[node->endState] = state_node;
					}

					if ((states[node->startState])->keys[node->inChar] != NULL) {
						node->next = (states[node->startState])->keys[node->inChar];
						(states[node->startState])->keys[node->inChar] = node;
					} else {
						(states[node->startState])->keys[node->inChar] = node;
					}

					/* Conto per ogni carattere quante transizioni ci sono */
					states[node->startState]->tr_counter[node->inChar]++;

				} else if (strcmp(parsing_line, "acc") == 0) {
					free(node);
					parsing_step = 2;
				}
			} else if (parsing_step == 2) {
				if (sscanf(parsing_line, "%d", &parsing_state) == 1) {
					if (states[parsing_state] == NULL) {
						state* state_node = (state*) malloc(sizeof(state));
						state_node->final = true;
						for (i = 0; i < 256; i++)
							state_node->keys[i] = NULL;
						for (i = 0; i < 256; i++)
							state_node->tr_counter[i] = 0;

						states[parsing_state] = state_node;
					} else {
						states[parsing_state]->final = true;
					}
					/*printf(">>> #%d diventa finale\n", parsing_state);*/
				} else if (strcmp(parsing_line, "max") == 0) {
					parsing_step = 3;
				}
			} else if (parsing_step == 3) {
				if (sscanf(parsing_line, "%ld", &maxSteps) == 1) {
					/*printf(">>> Max steps: %d\n", maxSteps);*/
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

	/*
	printf(">>> Riepilogo\n");
	printf("- Max steps: %d\n", maxSteps);
	printf("- States: %d\n", statesSize);

	for (i = 0; i < statesSize; i++) {
		if (states[i] != NULL) {
			printf("- Stato #%d (%d)\n", i, states[i]->final);
			int k;
			for (k = 0; k < 256; k++) {
				printf("\t Key: %d\n", k);
				if (states[i]->keys[k] != NULL) {
					transition *transitionCursor = states[i]->keys[k];
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
			for (k = 0; k < 256; k++) {
				if (states[i]->tr_counter[k] > 0)
					printf("\t Char: %c -> %d transizioni\n", k, states[i]->tr_counter[k]);
			}
		}
	}
	printf(">>> Esecuzione\n");
	*/
	
	
	/*int lineToSkip = 2;
	while (lineToSkip > 0) {
		while ((parsing_ch = getc(stdin)) != EOF) {
			if (parsing_ch == '\n') {
				break;
			}
		}
		lineToSkip -= 1;
	}*/
	//simulate(&states, maxSteps);
	while(simulate(&states, maxSteps) != 1);
	
	/*int lineToSkip = 7;
	while (lineToSkip > 0) {
		while ((parsing_ch = getc(stdin)) != EOF) {
			if (parsing_ch == '\n') {
				break;
			}
		}
		lineToSkip -= 1;
	}*/
	
	transition *transitionCursorTemp = NULL;
	transition *transitionCursor = NULL;
	for (i = 0; i < statesSize; i++) {
		if (states[i] != NULL) {
			for (int k = 0; k < 256; k++) {
				if (states[i]->keys[k] != NULL) {
					transitionCursor = states[i]->keys[k];
					while (transitionCursor != NULL) {
						transitionCursorTemp = transitionCursor;
						transitionCursor = transitionCursor->next;
						free(transitionCursorTemp);
					}
				}
			}
			free(states[i]);
		}
	}
	free(states);
	#ifdef BENCH
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("%fs (%f)\n", time_spent, (double)(end - begin));
	#endif
	return 0;
}