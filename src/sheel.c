#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>

#define READ_BUFF 1024
#define TOK_BUFF 64
#define TOK_DELIM " \t\r\n\a"

int sheel_ls(char **args) {
		DIR *folder;
		struct dirent *files;
		
		if (args[1] == NULL) {
				folder = opendir(".");

				if (folder == NULL) {
						perror("cannot access the directory\n");
						return 0;
				}

				while ( (files = readdir(folder)) ) {
						if (files->d_type == DT_DIR) {
								if (strncmp(files->d_name, ".", strlen(".")) == 0) {
										continue;
								} else {
										printf("\033[1;31m%s\033[0m", files->d_name);
										printf("\t");
								}
						} else {
								printf("%s", files->d_name);
								printf("\t");
						}
				}
		} else {
				folder = opendir(args[1]);

				if (folder == NULL) {
						perror("cannot access the directory\n");
						return 0;
				}

				while ( (files = readdir(folder)) ) {
						if (files->d_type == DT_DIR) {
								if (strncmp(files->d_name, ".", strlen(".")) == 0) {
										continue;
								} else {
										printf("\033[1;31m%s\033[0m", files->d_name);
										printf("\t");
								}
						} else {
								printf("%s", files->d_name);
								printf("\t");
						}
				}
		}

		printf("\n");
		closedir(folder);
		return 1;
}

int sheel_exit() {
		return 0;
}

int sheel_cd(char **args) {
		if (args[1] == NULL) {
				fprintf(stderr, "needs a argument\n");
				return 0;
		}

		if (chdir(args[1]) != 0) {
				perror("sheel");
		}

		return 1;
}

int sheel_launch(char **args) {
		pid_t pid, wpid;
		int status;

		pid = fork();

		if (pid == 0) {
				if (execvp(args[0], args) == -1) {
						perror("sheel");
				}
				exit(-1);
		} else if (pid < 0) {
				perror("sheel");
		} else {
				do {
						wpid = waitpid(pid, &status, WUNTRACED);
				} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		}

		return 1;
}

char *buitin_str[] = {
		"cd",
		"ls",
		"exit"
};

int (*builtin_met[])(char **) = {
		&sheel_cd,
		&sheel_ls,
		&sheel_exit
};

int builtin_str_len() {
		return sizeof(buitin_str)/sizeof(char *);
}

int sheel_execute(char **args) {
		int i;

		if (args[0] == NULL) {
				return i;
		}

		for (i = 0; i < builtin_str_len(); i++) {
				if (strcmp(args[0], buitin_str[i]) == 0) {
						return (*builtin_met[i])(args);
				}
		}

		return sheel_launch(args);
}

char ** sheel_tokens(char *line) {
		int bufsize = TOK_BUFF;
		char **tokens = malloc(sizeof(char *) * bufsize);
		int position = 0;
		char *token;

		token = strtok(line, TOK_DELIM);
		while(token != NULL) {
				tokens[position] = token;
				position++;

				if (position >= bufsize) {
						bufsize += TOK_BUFF;
						tokens = realloc(tokens, bufsize * sizeof(char *));

						if (!tokens) {
								fprintf(stderr, "allocation error\n");
								exit(-1);
						}
				}

				token = strtok(NULL, TOK_DELIM);
		}

		tokens[position] = NULL;
		return tokens;
}

char * sheel_read_line() {
		int bufsize = READ_BUFF;
		char *buffer = malloc(sizeof(char) * bufsize);
		char c;
		int position = 0;

		if (!buffer) {
				fprintf(stderr, "allocation failed\n");
				exit(-1);
		}

		while(1) {
				c = getchar();

				if (c == EOF || c == '\n') {
						buffer[position] = '\0';
						return buffer;
				} else {
						buffer[position] = c;
				}

				position++;

				if (position >= bufsize) {
						bufsize += READ_BUFF;
						buffer = realloc(buffer, bufsize * sizeof(char));

						if (!buffer) {
								fprintf(stderr, "allocation failed\n");
								exit(-1);
						}
				}
		}

		return NULL;
}

void sheel_loop(void) {
		char *line;
		char **args;
		int status;

		do {
				printf("> ");
				line = sheel_read_line();
				args = sheel_tokens(line);
				status = sheel_execute(args);

				free(line);
				free(args);
		} while (status);
}

int main(int argc, char **argv) {
		sheel_loop();

		return 0;
}
