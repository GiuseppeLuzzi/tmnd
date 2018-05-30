#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {MOVE_RIGHT, MOVE_STAY, MOVE_LEFT} moveType;
typedef enum {false, true} bool;

typedef struct _input {
	char value;
	/*char originalValue;*/
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
	transition *transitions;
} state;


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
		/*printf("<<%c>>\n", parsing_ch);*/
		if (parsing_ch != '\n') {
			/*printf(". a\n");*/
			if (*headCell == NULL) {
				/*printf(". b\n");*/
				*headCell = malloc(sizeof(input));
				cellCursor = *headCell;
				cellCursor->prev = NULL;
 			} else {
				cellCursor->next = malloc(sizeof(input));
				cellCursor->next->prev = cellCursor;
				cellCursor = cellCursor->next;
			}
			cellCursor->value = parsing_ch;
			/*cellCursor->originalValue = parsing_ch;*/
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

/* return values: 0 - not accepted | 1 - accepted | 2 - undefined */
int simulate(state ***states, input **cell, int currentState, int steps, int maxSteps, input **headCell) {
	state **statesCursor = *states;
	input *cellCursor;
	input *_headCell = *headCell;
	cellCursor = *cell;
	/*transition *transitionCursor = statesCursor[currentState]->transitions;*/
	transition *transitionCursor = statesCursor[currentState]->transitions;

	char originalValue;

	/*printf("%c | %c | %d | %d \n", statesCursor[currentState]->transitions->inChar, cellCursor->value, currentState, steps);*/
	/*printf("[>%d] -----------------------------------------------------------\n", steps);*/
	if (cellCursor == NULL) {
		return 0;
		/*printf("Sono uscito dal nastro!!!\n");*/
	}

	/* Se lo stato è finale, l'esecuzione termina */
	if (statesCursor[currentState]->final) {
		/*printf("[%d]> %d\n", steps, currentState);*/
		return 1;
	}


	int result = 0;
	if (steps > maxSteps) {
		return 2;
	}
	while (transitionCursor != NULL) {
		if (cellCursor->value == transitionCursor->inChar) {
			/*printf("Da %d provo %d (%c=%c->%c;%c)\n", currentState, transitionCursor->endState, cellCursor->value, transitionCursor->inChar, transitionCursor->outChar, m2c(transitionCursor->move));*/
			
			/*cellCursor->value = transitionCursor->outChar;*/
			originalValue = cellCursor->value;
			cellCursor->value = transitionCursor->outChar;
			/*printInput(*headCell);*/
			if (transitionCursor->move == MOVE_RIGHT) {
				if (cellCursor->next == NULL) {
					/*printf("Sono uscito dal nastro (davanti)!!\n");*/
					input *newCell = malloc(sizeof(input));
					newCell->value = '_';
					/*newCell->originalValue = '_';*/
					newCell->prev = cellCursor;
					newCell->next = NULL;
					cellCursor->next = newCell;
				}

				result = simulate(&statesCursor, &(cellCursor->next), transitionCursor->endState, steps+1, maxSteps, &_headCell);
			} else if (transitionCursor->move == MOVE_LEFT) {
				if (cellCursor->prev == NULL) {
					/*printf("Sono uscito dal nastro (indietro)!!\n");*/
					input *newCell = malloc(sizeof(input));
					newCell->value = '_';
					/*newCell->originalValue = '_';*/
					newCell->prev = NULL;
					newCell->next = cellCursor;
					cellCursor->prev = newCell;
				}

				result = simulate(&statesCursor, &(cellCursor->prev), transitionCursor->endState, steps+1, maxSteps, &_headCell);
			} else if (transitionCursor->move == MOVE_STAY) {
				result = simulate(&statesCursor, &(cellCursor), transitionCursor->endState, steps+1, maxSteps, &_headCell);
			}

			if (result == 1) {
				/*printf("[%d]> %d\n", steps, currentState);*/
				return result;
			} else if (result == 2) {
				/*printf("[%d]> U\n", steps);*/
				return result;
			}

			cellCursor->value = originalValue;
		}
		transitionCursor = transitionCursor->next;
	}

	/*printf("[<%d] -----------------------------------------------------------\n", steps);*/
	return result;
}

int main(int argc, char const *argv[]) {
	int statesSize;
	int lastStatesSize;
	int maxSteps;

	int parsing_step;
	int parsing_index;
	int parsing_state;
	char parsing_ch;
	char parsing_move;
	char parsing_line[11];
	
	int i;
	
	statesSize = 10;
	lastStatesSize= 10;
	input* cell = NULL;
	/*state** states = (state**) malloc(statesSize * sizeof(state*));*/
	state** states = malloc(statesSize * sizeof(state*));

	for (i = 0; i < statesSize; i++) 
		states[i] = NULL;

	parsing_step = 0;
	parsing_index = 0;
	for (i = 0; i < 11; i++) 
		parsing_line[i] = (char) 0;

	while ((parsing_ch = getc(stdin)) != EOF) {
		if (parsing_ch != '\n') {
			parsing_line[parsing_index] = parsing_ch;
			parsing_index++;
		} else {
			/*printf(">>%s\n", parsing_line);*/
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

					/*printf(">>>> %d %c %c %d %d\n", node->startState, node->inChar, node->outChar, node->move, node->endState);*/
					
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
						state_node->transitions = NULL;
						states[node->startState] = state_node;
					}
					if ((states[node->startState])->transitions != NULL) {
						node->next = (states[node->startState])->transitions;
						(states[node->startState])->transitions = node;
					} else {
						(states[node->startState])->transitions = node;
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
						state_node->transitions = NULL;
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
			for (i = 0; i < 11; i++) 
				parsing_line[i] = (char) 0;
		}
	}

	/*printf(">>> Riepilogo\n");
	printf("- Max steps: %d\n", maxSteps);
	printf("- States: %d\n", statesSize);

	for (i = 0; i < statesSize; i++) {
		if (states[i] != NULL) {
			printf("- Stato #%d (%d)\n", i, states[i]->final);
			if (states[i]->transitions != NULL) {
				transition *transitionCursor = states[i]->transitions;
				while (transitionCursor != NULL) {
					printf("\t %d -> %d [%c|%c|%d]\n",
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
	printf("bbb\n");*/

	/*while (lineToSkip > 0) {
		while ((parsing_ch = getc(stdin)) != EOF) {
			if (parsing_ch == '\n') {
				break;
			}
		}
		lineToSkip -= 1;
	}*/
	while (loadInput(&cell)) {
		/*printf("1)\n");
		printf("2)\n");
		printInput(cell);*/

		int result = 0;
		/*printf("aaa\n");*/
		/* int simulate(state ***nastro, int currentState, int steps) { */
		result = simulate(&states, &cell, 0, 0, maxSteps, &cell);
		freeNastro(&cell);
		/*free(cell);*/
		cell = NULL;
		if (result == 2) {
			printf("U\n");
		} else {
			printf("%d\n", result);
		}
	}

	for (i = 0; i < statesSize; i++) {
		if (states[i] != NULL) {
			transition *tempTransition = NULL;
			transition *transitionCursor = states[i]->transitions;
			while (transitionCursor != NULL) {
				tempTransition = transitionCursor;
				transitionCursor = transitionCursor->next;
				free(tempTransition);
			}
			states[i]->transitions = NULL;
		}
	}
	free(states);

	return 0;
}