#ifndef executar_h
#define executar_h

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "historico.h"

void sigkillall_handler(int signum);
int separate(char* execute[10], char* command);
int exec_command(char* command);
int executar(char* command, int timeMax, int fdlidxw, int fdlogw);

#endif