#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {MOVE_RIGHT, MOVE_STAY, MOVE_LEFT} moveType;
typedef enum {true, false} bool;

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
}

int main(int argc, char const *argv[])
{
	int step;
	int statesSize;
	int lastStatesSize;

	int index;
	char ch;
	char move;
	char line[11];
	
	int i;
	
	statesSize = 10;
	lastStatesSize= 10;
	transition** states = (transition**) malloc(statesSize * sizeof(transition*));
	for (i = 0; i < statesSize; i++) 
		states[i] = NULL;

	step = 0;
	index = 0;
	for (i = 0; i < 11; i++) 
		line[i] = (char) 0;

	while ((ch = getc(stdin)) != EOF) {
		if (ch != '\n') {
			line[index] = ch;
			index++;
		} else {
			if ((step == 0) && (strcmp(line, "tr") == 0)) {
				step += 1;
			} else if (step == 1) {
				transition* node = (transition*) malloc(sizeof(transition));
 				node->next = NULL;
				if (sscanf(line, "%d%*c%c%*c%c%*c%c%*c%d", &(node->startState),
												   &(node->inChar),
												   &(node->outChar),
												   &move,
												   &(node->endState)) == 5) {
					if (move == 'R')
						node->move = MOVE_RIGHT;
					else if (move == 'S')
						node->move = MOVE_STAY;
					else if (move == 'L')
						node->move = MOVE_LEFT;

					printf(">> %d %c %c %d %d\n", node->startState, node->inChar, node->outChar, node->move, node->endState);
					
					// Ingrandisco l'array quanto necessario
					while ((node->startState > (statesSize - 1)) || (node->endState > (statesSize - 1))) {
						statesSize *= 2;
					}
					
					if (lastStatesSize != statesSize) {
						states = (transition**) realloc(states, statesSize * sizeof(transition*));
						for (i = lastStatesSize; i < statesSize; i++)
							states[i] = NULL;

						lastStatesSize = statesSize;
					}

					// Aggiungo lo stato alla array linked-list
					if (states[node->startState] != NULL) {
						node->next = states[node->startState];
						states[node->startState] = node;
					} else
						states[node->startState] = node;


				} else if (strcmp(line, "acc") == 0) {
					step = 2;
					break;
				}
			}
			index = 0;
			for (i = 0; i < 11; i++) 
				line[i] = (char) 0;
		}
	}

	for (i = 0; i < statesSize; i++) {
		if (states[i] != NULL)
			printf("%d -> %d\n", states[i]->startState, states[i]->endState);
	}

	printf("oko\n");
	return 0;
}