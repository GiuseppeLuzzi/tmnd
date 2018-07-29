#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BENCH
#ifdef BENCH
#include <time.h>
#endif

#define DEBUG
#define INPUT_BUFFER 18
#define CHUNK_SIZE 15

typedef enum {MOVE_RIGHT, MOVE_STAY, MOVE_LEFT} moveType;
typedef enum {false, true} bool;

typedef struct _input {
	char value;
	struct _input *prev;
	struct _input *next;
} input;

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
	transition *keys[8];
} state;

typedef struct _configuration {
	int stateID;
	long moves;
	struct _configuration *next;
} configuration;

typedef struct _tapechunk {
	int size;
	char cells[CHUNK_SIZE];
	struct _tapechunk *prev;
	struct _tapechunk *next;
} tapechunk;


char m2c(moveType move) {
	if (move == MOVE_RIGHT) return 'R';
	if (move == MOVE_LEFT) return 'L';
	if (move == MOVE_STAY) return 'S';
	return '-';
}

bool loadInput(input **headCell) {
	char parsing_ch;
	input *cellCursor = NULL;
	while ((parsing_ch = getc(stdin)) != EOF) {
		if (parsing_ch != '\n') {
			if (*headCell == NULL) {
				*headCell = malloc(sizeof(input));
				cellCursor = *headCell;
				cellCursor->prev = NULL;
 			} else {
				cellCursor->next = malloc(sizeof(input));
				cellCursor->next->prev = cellCursor;
				cellCursor = cellCursor->next;
			}
			cellCursor->value = parsing_ch;
			cellCursor->next = NULL;
		} else {
			return true;
		}
	}
	return false;
}

void printInput(input *cell) {
	input *cellCursor = NULL;
	cellCursor = cell;
	while (cellCursor != NULL) {
		printf(" %c |", cellCursor->value);
		cellCursor = cellCursor->next;
	}
	printf("\n");
}

void freeNastro(input **headCell) {
	input *cellCursor = NULL;
	cellCursor = *headCell;

	while (cellCursor->next != NULL) {
		cellCursor = cellCursor->next;
	}
	while (cellCursor->prev != NULL) {
		cellCursor = cellCursor->prev;
		free(cellCursor->next);
	}
}

void loadTapeChunk(tapechunk **tape) {
	int counter = 0;
	char parsing_ch;
	tapechunk *tapeCursor = NULL;
	tapeCursor = *tape;

	while ((parsing_ch = getc(stdin)) != EOF) {
		if (parsing_ch == '\r') {
			continue;
		} else if ((parsing_ch == '\n') || (counter > CHUNK_SIZE)) {
			if (parsing_ch == '\n') {
				for (int i = counter; i < CHUNK_SIZE; i++) {
					tapeCursor->cells[i] = '_';
				}
				tapeCursor->size = CHUNK_SIZE;
			}
			break;
		}
		counter++;

		printf("- %c\n", parsing_ch);
		if (tapeCursor == NULL) {
			tapeCursor = malloc(sizeof(tapechunk));
			tapeCursor->size = 0;
			tapeCursor->next = NULL;
			tapeCursor->prev = NULL;
		}
		tapeCursor->cells[counter] = parsing_ch;
		tapeCursor->size++;
	}
}


/* return values: 0 - not accepted | 1 - accepted | 2 - undefined */
int simulate(state ***states, tapechunk **tape) {
	int queueLength = 1;
	int i;

	state **statesCursor = *states;
	tapechunk *tapeHead = *tape;

	configuration *queue = malloc(sizeof(configuration));
	configuration *queueCursor = queue;
	configuration *queueTemp = queueCursor;
	queue->stateID = 0;
	queue->moves = 0;
	queue->next = NULL;

	loadTapeChunk(&tapeHead);

	for (int i = 0; i < tapeHead->size; i++)
		printf("tape [%d]: %c\n", i, tapeHead->cells[i]);

	while (queueLength > 0) {
		printf("%d\t(final: %d, moves: %ld, queue size: %d)\n", queueCursor->stateID, statesCursor[queueCursor->stateID]->final, queue->moves, queueLength);
		
		if (statesCursor[queueCursor->stateID]->final == true) {
			//printf("finale\n");
			return 1;
		}
		
		for (i = 0; i < 8; i++) {
			if (statesCursor[queueCursor->stateID]->keys[i] != NULL) {
				queue->next = malloc(sizeof(configuration));
				queue = queue->next;
				queue->stateID = statesCursor[queueCursor->stateID]->keys[i]->endState;
				queue->moves = queueCursor->moves + 1;
				queue->next = NULL;
				queueLength++;
			}
		}

		queueTemp = queueCursor;
		queueCursor = queueCursor->next;
		free(queueTemp);
		queueLength--;
	}
	return 0;
}


int main(int argc, char const *argv[]) {
	#ifdef BENCH
		clock_t begin = clock();
	#endif
	
	int statesSize;
	int lastStatesSize;
	int maxSteps;

	int parsing_step;
	int parsing_index;
	int parsing_state;
	char parsing_ch;
	char parsing_move;
	char parsing_line[INPUT_BUFFER];
	
	int i;
	
	statesSize = 10;
	lastStatesSize= 10;
	input* cell = NULL;

	state** states = malloc(statesSize * sizeof(state*));
	tapechunk* tape = NULL;

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
						for (i = 0; i < 8; i++)
							state_node->keys[i] = NULL;
						states[node->startState] = state_node;
					}

					if (states[node->endState] == NULL) {
						state* state_node = (state*) malloc(sizeof(state));
						state_node->final = false;
						for (i = 0; i < 8; i++)
							state_node->keys[i] = NULL;
						states[node->endState] = state_node;
					}

					int hash = node->inChar >> 5;
					if ((states[node->startState])->keys[hash] != NULL) {
						node->next = (states[node->startState])->keys[hash];
						(states[node->startState])->keys[hash] = node;
					} else {
						(states[node->startState])->keys[hash] = node;
					}


				} else if (strcmp(parsing_line, "acc") == 0) {
					free(node);
					parsing_step = 2;
				}
			} else if (parsing_step == 2) {
				if (sscanf(parsing_line, "%d", &parsing_state) == 1) {
					if (states[parsing_state] == NULL) {
						state* state_node = (state*) malloc(sizeof(state));
						state_node->final = true;
						for (i = 0; i < 8; i++)
							state_node->keys[i] = NULL;
						states[parsing_state] = state_node;
					} else {
						states[parsing_state]->final = true;
					}
					/*printf(">>> #%d diventa finale\n", parsing_state);*/
				} else if (strcmp(parsing_line, "max") == 0) {
					parsing_step = 3;
				}
			} else if (parsing_step == 3) {
				if (sscanf(parsing_line, "%d", &maxSteps) == 1) {
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

	/*printf(">>> Riepilogo\n");
	printf("- Max steps: %d\n", maxSteps);
	printf("- States: %d\n", statesSize);

	for (i = 0; i < statesSize; i++) {
		if (states[i] != NULL) {
			printf("- Stato #%d (%d)\n", i, states[i]->final);
			int k;
			for (k = 0; k < 8; k++) {
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
		}
	}
	printf("bbb\n");*/

	/*int lineToSkip = 1;
	while (lineToSkip > 0) {
		while ((parsing_ch = getc(stdin)) != EOF) {
			if (parsing_ch == '\n') {
				break;
			}
		}
		lineToSkip -= 1;
	}*/

	simulate(&states, &tape);

	/*int lineToSkip = 7;
	while (lineToSkip > 0) {
		while ((parsing_ch = getc(stdin)) != EOF) {
			if (parsing_ch == '\n') {
				break;
			}
		}
		lineToSkip -= 1;
	}*/
	
	#ifdef BENCH
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("%fs (%f)\n", time_spent, (double)(end - begin));
	#endif
	return 0;
}