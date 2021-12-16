#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dtw.h"

message_t msg;

int main(){

	int test[SIZE][SIZE];
	int pattern[SIZE][SIZE];
	int result, j;

	memphis_receive(&msg, recognizer);

	puts("Task P1 INIT\n");

	__builtin_memcpy(test, msg.payload, sizeof(test));

	memphis_real_time(DEADLINE, DEADLINE, EXEC_TIME);

	for(j=0; j<PATTERN_PER_TASK; j++){

		memset(msg.payload,0, sizeof(int)*msg.length);

		memphis_receive(&msg, bank);

		//puts("Task P1 received pattern from bank\n");

		__builtin_memcpy(pattern, msg.payload, sizeof(pattern));

		result = dynamicTimeWarping(test, pattern);
		printf("R = %d\n", result);

		msg.length = 1;

		msg.payload[0] = result;

		memphis_send(&msg, recognizer);
	}

	printf("Task P1 FINISHED AT %d\n", memphis_get_tick());

	return 0;
}
