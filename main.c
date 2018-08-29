#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 128
#define RIGHT_MIN_SIZE 128
// 128
#define LEFT_MIN_SIZE 128
// 512
#define STATES_HASHMAP 512
#define INPUT_BUFFER 128

#define MOVE_RIGHT 0
#define MOVE_STAY 1
#define MOVE_LEFT 2

typedef struct _transition {
	unsigned int startState;
	unsigned int endState;
	char inChar;
	char outChar;
	int move;
	struct _transition *next;
} transition;

typedef struct _tape {
	//int tapeID;
	int leftCounter;
	int rightCounter;
	int leftMaxSize;
	int rightMaxSize;
	char *left;
	char *right;
	int reference_counter;
} tapeInfo;

typedef struct _status {
	unsigned int size;
	transition** transitions;
} statusInfo;

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

	tape->reference_counter= 0;
	tape->leftCounter = 0;
	tape->leftMaxSize = 0;
	tape->left = NULL;
	tape->rightCounter = 0;
	tape->rightMaxSize = RIGHT_MIN_SIZE;
	tape->right = malloc(sizeof(char) * tape->rightMaxSize);

	for (int i = 0; i < tape->rightMaxSize; i++)
		tape->right[i] = '_';

	while ((parsing_ch = getchar()) != EOF) {
		if (parsing_ch == '\r') {
			continue;
		} else if (parsing_ch == '\n') {
			break;
		}
		if (tape->rightCounter == tape->rightMaxSize) {
			tape->rightMaxSize = tape->rightMaxSize + RIGHT_MIN_SIZE;
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

int simulate(statusInfo *chars[], long maxSteps, int *acceptingState) {
	int queueLength = 1;
	int eof = 0;
	int i;
	int mt_status = 0;
	int transitionCounter = 0;

	int to_exit = 0;

	tapeInfo *basicTape = malloc(sizeof(tapeInfo));

	eof = loadTape(&basicTape);
	if (eof && basicTape->rightCounter == 0) {
		free(basicTape->right);
		if (basicTape->left != NULL)
			free(basicTape->left);
		free(basicTape);
		return eof;
	}
	basicTape->rightCounter = basicTape->rightMaxSize;
	
	configuration *queue = malloc(sizeof(configuration)); // Indica sempre la coda
	configuration *queueSuperHead = queue;
	configuration *queueHead = queue; // Indica sempre la testa
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
		to_exit = 0;

		if (queueHead->moves >= maxSteps) {
			mt_status = 2;

			queueTemp = queueHead;
			queueHead = queueHead->next;

			if (queueTemp->tape->left != NULL)
				free(queueTemp->tape->left);
			free(queueTemp->tape->right);
			free(queueTemp->tape);
			free(queueTemp);

			queueLength--;
			continue;
		}

		transition *transitionCursor = NULL;
		char currentChar = 0;

		if (queueHead->index >= 0) {
			if (queueHead->index >= queueHead->tape->rightMaxSize - 1) {
				queueHead->tape->rightMaxSize = queueHead->tape->rightMaxSize + RIGHT_MIN_SIZE;
				queueHead->tape->right = realloc(queueHead->tape->right, sizeof(char) * queueHead->tape->rightMaxSize);
				
				for (int i = queueHead->tape->rightCounter; i < queueHead->tape->rightMaxSize; i++)
					queueHead->tape->right[i] = '_';
				queueHead->tape->rightCounter = queueHead->tape->rightMaxSize;
			}

			currentChar = queueHead->tape->right[queueHead->index];

			if (chars[currentChar - 48] != NULL && chars[currentChar - 48]->transitions != NULL && queueHead->stateID < chars[currentChar - 48]->size && chars[currentChar - 48]->transitions[queueHead->stateID] != NULL) {
				transitionCursor = chars[currentChar - 48]->transitions[queueHead->stateID];
			} else {
				transitionCursor = NULL;
			}
		} else {
			if (queueHead->tape->left == NULL) {
				queueHead->tape->leftMaxSize = LEFT_MIN_SIZE;
				queueHead->tape->left = malloc(sizeof(char) * queueHead->tape->leftMaxSize);
				for (int i = 0; i < queueHead->tape->leftMaxSize; i++)
					queueHead->tape->left[i] = '_';
				queueHead->tape->leftCounter = queueHead->tape->leftMaxSize;
			}

			if (-queueHead->index >= queueHead->tape->leftMaxSize - 1) {
				queueHead->tape->leftMaxSize = queueHead->tape->leftMaxSize + LEFT_MIN_SIZE;
				queueHead->tape->left = realloc(queueHead->tape->left, sizeof(char) * queueHead->tape->leftMaxSize);
				
				for (int i = queueHead->tape->leftCounter; i < queueHead->tape->leftMaxSize; i++)
					queueHead->tape->left[i] = '_';
				queueHead->tape->leftCounter = queueHead->tape->leftMaxSize;
			}

			currentChar = queueHead->tape->left[-queueHead->index - 1];

			if (chars[currentChar - 48] != NULL && chars[currentChar - 48]->transitions != NULL && queueHead->stateID < chars[currentChar - 48]->size && chars[currentChar - 48]->transitions[queueHead->stateID] != NULL) {
				transitionCursor = chars[currentChar - 48]->transitions[queueHead->stateID];
			} else {
				transitionCursor = NULL;
			}
		}

		transitionCounter = 0;

		if (chars[currentChar - 48] == NULL) {
			transitionCounter = 0;
		} else if (chars[currentChar - 48]->transitions == NULL) {
			transitionCounter = 0;
		} else if (queueHead->stateID > chars[currentChar - 48]->size) {
			transitionCounter = 0;
		} else if (chars[currentChar - 48]->transitions[queueHead->stateID] == NULL) {
			transitionCounter = 0;
		} else {
			if (chars[currentChar - 48]->transitions[queueHead->stateID]->next == NULL) transitionCounter = 1;
			else transitionCounter = 2;
		}

		/*if (transitionCounter == 1) {
			// Se è uno stato pozzo, siamo già in U per quel ramo.
			if (transitionCursor->inChar == transitionCursor->outChar &&
				transitionCursor->move == MOVE_STAY &&
				queueHead->stateID == transitionCursor->endState) {

				mt_status = 2;

				queueTemp = queueHead;
				queueHead = queueHead->next;

				if (queueTemp->tape->left != NULL)
					free(queueTemp->tape->left);
				free(queueTemp->tape->right);
				free(queueTemp->tape);
				free(queueTemp);

				queueLength--;
				continue;
			}
		}*/

		while (transitionCounter > 0 && transitionCursor != NULL) {
			if (acceptingState[transitionCursor->endState] == 1 && (queueHead->moves + 1) <= maxSteps) {
				mt_status = 1;

				while (queueHead != NULL) {
					queueTemp = queueHead;
					queueHead = queueHead->next;

					if (queueTemp->tape->left != NULL)
						free(queueTemp->tape->left);
					free(queueTemp->tape->right);
					free(queueTemp->tape);
					free(queueTemp);
				}

				queueLength = 0;
				to_exit = 1;
				break;
			}

			if (transitionCounter > 1) {
				queueHead->tape->reference_counter = 0;

				queue->next = malloc(sizeof(configuration));
				queue = queue->next;

				queue->stateID = transitionCursor->endState;
				queue->index = queueHead->index;
				queue->moves = queueHead->moves + 1;

				queue->tape = malloc(sizeof(tapeInfo));
				queue->tape->reference_counter = 0;
				queue->tape->leftCounter = queueHead->tape->leftCounter;
				queue->tape->rightCounter = queueHead->tape->rightCounter;
				queue->tape->leftMaxSize = queueHead->tape->leftMaxSize;
				queue->tape->rightMaxSize = queueHead->tape->rightMaxSize;

				if (queueHead->tape->left == NULL) {
					queue->tape->left = NULL;
				} else {
					queue->tape->left = malloc(sizeof(char) * queue->tape->leftMaxSize);
					memcpy(queue->tape->left, queueHead->tape->left, sizeof(char) * queue->tape->leftMaxSize);
				}

				queue->tape->right = malloc(sizeof(char) * queue->tape->rightMaxSize);
				memcpy(queue->tape->right, queueHead->tape->right, sizeof(char) * queue->tape->rightMaxSize);

			} else if (transitionCounter == 1) {
				queueHead->tape->reference_counter = 1;
				
				queue->next = queueHead;
				queue = queue->next;

				queue->stateID = transitionCursor->endState;
				queue->index = queueHead->index;
				queue->moves = queueHead->moves + 1;
				queueTemp = queueHead->next;
			}

			if (transitionCursor->inChar != transitionCursor->outChar) {
				if (queue->index >= 0) {
					queue->tape->right[queue->index] = transitionCursor->outChar;
				} else {
					queue->tape->left[-queue->index - 1] = transitionCursor->outChar;
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
		if (to_exit == 1)
			continue;

		if (transitionCounter == 0 || transitionCounter > 1) {
			queueTemp = queueHead;
			queueHead = queueHead->next;
			if (queueTemp->tape->left != NULL)
				free(queueTemp->tape->left);
			free(queueTemp->tape->right);
			free(queueTemp->tape);
			free(queueTemp);
		} else {
			queueHead = queueTemp;
		}


		queueLength--;
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

	unsigned int maxState,
				 maxSteps,
				 parsing_state,
				 j;

	statusInfo *chars[76];
	for (i = 0; i < 76; i++)
		chars[i] = NULL;

	maxState = 0;
	int *acceptingState = NULL;

	parsing_step = 0;
	parsing_index = 0;
	for (i = 0; i < INPUT_BUFFER; i++) 
		parsing_line[i] = (char) 0;

	while ((parsing_ch = getchar()) != EOF) {
		if (parsing_ch != '\n') {
			if (parsing_ch == '\r') continue;
			if (parsing_ch == ' ' && parsing_index != 0 && parsing_line[parsing_index-1] == ' ') continue;
			parsing_line[parsing_index] = parsing_ch;
			parsing_index++;
		} else {
			if ((parsing_step == 0) && (strcmp(parsing_line, "tr") == 0)) {
				parsing_step = 1;
			} else if (parsing_step == 1) {
				transition *node = malloc(sizeof(transition));
 				node->next = NULL;
				if (sscanf(parsing_line, "%u%*c%c%*c%c%*c%c%*c%u", &(node->startState),
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

					if (node->startState > maxState) maxState = node->startState;
					if (node->endState > maxState) maxState = node->endState;

					if (chars[node->inChar-48] == NULL) {
						statusInfo *state = malloc(sizeof(statusInfo));

						state->size = STATES_HASHMAP;

						if (node->startState > state->size-1 || node->endState > state->size-1) {
							if (node->startState > node->endState) {
								state->size = node->startState + 10;
							} else {
								state->size = node->endState + 10;
							}
						}

						state->transitions = malloc(state->size * sizeof(transition*));
						for (j = 0; j < state->size; j++)
							state->transitions[j] = NULL;

						state->transitions[node->startState] = node;
						chars[node->inChar-48] = state;
					} else {
						if (node->startState > chars[node->inChar-48]->size-1 || node->endState > chars[node->inChar-48]->size-1) {
							unsigned int previous_size = chars[node->inChar-48]->size;
							if (node->startState > node->endState) {
								chars[node->inChar-48]->size = node->startState + 10;
							} else {
								chars[node->inChar-48]->size = node->endState + 10;
							}

							chars[node->inChar-48]->transitions = realloc(chars[node->inChar-48]->transitions, chars[node->inChar-48]->size * sizeof(transition*));
							for (j = previous_size; j < chars[node->inChar-48]->size; j++) {
								chars[node->inChar-48]->transitions[j] = NULL;
							}
						}

						if (chars[node->inChar-48]->transitions[node->startState] == NULL) {
							chars[node->inChar-48]->transitions[node->startState] = node;
						} else {
							node->next = chars[node->inChar-48]->transitions[node->startState];
							chars[node->inChar-48]->transitions[node->startState] = node;
						}
					}
				} else if (strcmp(parsing_line, "acc") == 0) {
					free(node);
					parsing_step = 2;

					acceptingState = malloc(sizeof(int) * (maxState + 1));
					for (j = 0; j < maxState + 1; j++)
						acceptingState[j] = 0;
				}
			} else if (parsing_step == 2) {
				if (sscanf(parsing_line, "%u", &parsing_state) == 1) {
				
					acceptingState[parsing_state] = 1;
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

	
	while(simulate(chars, maxSteps, acceptingState) != 1);

	free(acceptingState);
	transition *transitionTemp, *transitionCursor;
	for (i = 0; i < 76; i++) {
		if (chars[i] == NULL) continue;
		for (k = 0; k < chars[i]->size; k++) {
			if (chars[i]->transitions[k] == NULL) continue;
			transitionCursor = chars[i]->transitions[k];
			while (transitionCursor != NULL) {
				transitionTemp = transitionCursor;
				transitionCursor = transitionCursor->next;
				free(transitionTemp);
			}
		}
		free(chars[i]->transitions);
		free(chars[i]);
	}

	return 0;
}