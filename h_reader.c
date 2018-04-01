#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define INCLUDE_SIZE 8
#define TO_READ 255

/* Pas du tout fini
 * J'continuerai soon */

struct hfile {
  char* name;
  struct hfile* next;
};

struct cfile {
  char* name;
  struct hfile* first;
};

struct cfile* findLibraries(char* cname){
  struct cfile* file;
  file -> name = cname;
  file -> first = NULL;
  int fd_src, read_bytes = -1, pos = 0;
  char* buf, verified_list;
  char current_char = '\0';

  fd_src = open(cname, O_RDONLY, 0666);

  if(fd_src == -1){
    perror("Problème open");
    return NULL;
  }

  buf = (char*) malloc(TO_READ);
  memset(&buf, '\0', sizeof(buf));

  /* On commence par lire un gros bloc
   * qui sera traité par la suite.
   * Celui-ci est censé contenir tous les include d'un fichier */
  read_bytes = read(fd_src, &buf, TO_READ);

  if(read_bytes == 0)
    return NULL;

  if(read_bytes == -1){
    perror("Problème read");
    return NULL;
  }

  verified_list = (char*) malloc(read_bytes);
  memset(&verified_list, '\0', sizeof(verified_list));

  /* Ici on va trier les .h du reste dans
   * le tableau buf */
  while(pos < read_bytes){
    pos += INCLUDE_SIZE;

    /* Place le curseur au début
     * de la déclaration du fichier
     * à inclure*/
    while(current_char != '\"'){
      read_bytes = read(fd_src, &current_char, 1);

      if(read_bytes == 0)
        break;

      if(read_bytes == -1){
        perror("Problème read");
        return file;
      }

      pos++;
    }

    /*Saute les premières guillemets
     * de la déclaration */
    read(fd_src, &current_char, 1);
    pos++;
    //&verified_list + pos = current_char;
  }

  return file;
}
