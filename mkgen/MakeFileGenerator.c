#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "makeheaders.h"

#include <sys/types.h>
#include <fcntl.h>

#define ANSI_COLOR_GREEN  "\x1B[32m" //Green color output
#define ANSI_COLOR_RESET  "\x1B[0m"  //Normal color output
#define ANSI_COLOR_RED  "\x1B[31m" //Red color output
#define ANSI_COLOR_YELLOW  "\x1B[33m" //yellow color output

#define TRUE 1
#define FALSE 0
#define DEFAULT_NUMBER_FILES_IN_DIR 10

#define FOLDER_CONFIG_NAME ".mkgenlogs"
#define PROJECT_DIRECTORY "."
#define MAX_FOLDER_PATH_SIZE 2048

#define BUFF_SIZE 2048
int quiet = TRUE;
int h_flag = FALSE;
int f_flag = FALSE;
#define DEBUG
/*Flags*/

#ifdef DEBUG
#define DEBUG_PRINT if(quiet == FALSE) printf
#else
#define DEBUG_PRINT(...)
#endif


//TODO when checking if file is a .c, check if the .c is at the end of the name
//ex : c.cfg shouldn't be considered as .c file, whereas file.c should

typedef struct fileInformations{
  char* name;
  time_t date;
  char* nameWithoutExtension;
}fileInformations;

typedef struct filesOfDirectory{
  fileInformations* arrayOfFiles;
  unsigned long numberOfFiles;
  const char* directoryPath;
}filesOfDirectory;

time_t getDateFile(const char* file){
  struct stat file_stat;
  stat(file, &file_stat);
  return file_stat.st_mtime;
}

void saveDate(fileInformations file){
  char folderPath[2048] = FOLDER_CONFIG_NAME;
  char* filePath = strcat(strcat(folderPath, "/"), file.nameWithoutExtension);
  int fd = open(filePath, O_CREAT|O_TRUNC|O_WRONLY, 0600);
  if(fd == -1){ printf("Error opening %s to save date\n", filePath); exit(errno);}
  time_t date = file.date;
  if(write(fd, &date, sizeof(date)) == -1){
    printf("Error saving date into %s\n", filePath);
    exit(errno);
  }
  close(fd);
}



void* safeMemoryAllocation(long cellSize, long numberOfCells){
  void *ptr = calloc(numberOfCells, cellSize);
  if(ptr == NULL){
    printf("Array couldn't be allocated in memory, exiting...\n");
    exit(errno);
  } else {
    //printf("Allocation successful\n");
  }
  return ptr;
}

// remove_ext: removes the "extension" from a file spec.
//   mystr is the string to process.
//   dot is the extension separator.
//   sep is the path separator (0 means to ignore).
// Returns an allocated string identical to the original but
//   with the extension removed. It must be freed when you're
//   finished with it.
// If you pass in NULL or the new string can't be allocated,
//   it returns NULL.

char *remove_ext (char* mystr) {
  char dot = '.';
  char sep = '/';
  char *retstr, *lastdot, *lastsep;
  // Error checks and allocate string.
  if (mystr == NULL)
      return NULL;
  if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
      return NULL;
    // Make a copy and find the relevant characters.
  strcpy (retstr, mystr);
  lastdot = strrchr (retstr, dot);
  lastsep = (sep == 0) ? NULL : strrchr (retstr, sep);
    // If it has an extension separator.
  if (lastdot != NULL) {
      // and it's before the extenstion separator.
      if (lastsep != NULL) {
        if (lastsep < lastdot) {
            // then remove it.
              *lastdot = '\0';
        }
      } else {
        // Has extension separator with no path separator.
          *lastdot = '\0';
      }
  }
    // Return the modified string.
  return retstr;
}


/*Returns a struct of filesOfDirectory, for current dir */
filesOfDirectory getCFilesInDirectory(const char* dirpath){
  DIR *d;
  struct dirent *dir;
  long numberOfFiles = 0;
  filesOfDirectory files = { NULL , numberOfFiles, dirpath };
  files.arrayOfFiles = safeMemoryAllocation(sizeof(fileInformations), DEFAULT_NUMBER_FILES_IN_DIR);
  if(files.arrayOfFiles == NULL){
    printf("nope\n");
    exit(1);
  }
  d = opendir(dirpath);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      char * currFile = dir->d_name;
      long lengthOfFileName = strlen(currFile);
      char* tst = strstr(currFile, ".c");
      int hasDotCExtension = (tst != NULL && strcmp(tst, ".c\n"));
      int hasValidFileLength = (lengthOfFileName >= 3);
      int isACFile = (hasValidFileLength && hasDotCExtension);
      if(isACFile){
        files.arrayOfFiles[numberOfFiles].name = currFile;
        files.arrayOfFiles[numberOfFiles].nameWithoutExtension =
          safeMemoryAllocation(lengthOfFileName - 2, 1);
        files.arrayOfFiles[numberOfFiles].nameWithoutExtension = remove_ext(files.arrayOfFiles[numberOfFiles].name);
        if(files.arrayOfFiles[numberOfFiles].nameWithoutExtension == NULL){ printf("Error removing ext of %s\n", currFile); exit(errno);}

        files.arrayOfFiles[numberOfFiles].date = getDateFile(currFile);
        long defaultSizeAllocatedForArrayOfFiles = sizeof(fileInformations) * DEFAULT_NUMBER_FILES_IN_DIR;
        numberOfFiles++;
        if(numberOfFiles % DEFAULT_NUMBER_FILES_IN_DIR == 0){
          files.arrayOfFiles = realloc(files.arrayOfFiles,
            (sizeof(fileInformations) * numberOfFiles)
            + defaultSizeAllocatedForArrayOfFiles);
          if(files.arrayOfFiles == NULL){
            perror("Realloc array of files failed");
            exit(errno);
          }
        }
      }
    }
    closedir(d);
  }
  files.numberOfFiles = numberOfFiles;

  return files;
}
/*In : base file name without extension*/
void deleteHeaderFromFile(const char* filePath){
  long lengthOfFileName = strlen(filePath);
  char* headerFileName = calloc(1,lengthOfFileName + 2);
  memcpy(headerFileName , filePath, lengthOfFileName + 2);
  strcat(headerFileName, ".h");
  unlink(headerFileName);
  DEBUG_PRINT("Unlinked header file %s\n", headerFileName);
}

void generateH(const char* filePath){
  long lengthOfFileName = strlen(filePath);

  char* headerFileName = safeMemoryAllocation(lengthOfFileName, 1);
  strncpy(headerFileName, filePath, lengthOfFileName);
  DEBUG_PRINT("Generating .h from %s\n", headerFileName);
  if(createHFileFromC(headerFileName) == -1){
    DEBUG_PRINT(ANSI_COLOR_RED"Error during header's generation\n"ANSI_COLOR_RESET);
  } else {
    DEBUG_PRINT(ANSI_COLOR_GREEN"Header has been successfuly generated\n"ANSI_COLOR_RESET);
  }
}

/*Returns boolean if file exists or not*/
int isFileExisting(const char * filePath){
  if(access(filePath, F_OK) != -1) {
    return TRUE;
  } else {
    return FALSE;
  }
}


/*Returns 1 if files date doens't match, 0 instead.*/
int hasItBeenModified(const fileInformations fileInfo){
  char folderPath[2048] = FOLDER_CONFIG_NAME;
  struct stat sb;
  int folderDoesntExists = (stat(folderPath, &sb) == -1);
  if(folderDoesntExists) {
    mkdir(folderPath, 0700); //Creating dir
  }
  char* filePath = strcat(strcat(folderPath, "/"), fileInfo.nameWithoutExtension);
  if(isFileExisting(filePath)){
    int fd = open(filePath, O_RDONLY);
    if(fd == -1) { perror("opening log file"); exit(errno);}
    time_t logDate;
    if(read(fd, &logDate, sizeof(logDate)) == -1) {
      printf("Err reading log date %s. File name without ext : %s\n", filePath, fileInfo.nameWithoutExtension);
      exit(errno);
    }
    DEBUG_PRINT("Associated log file name = %s\nAssociated log file date = %ld\n", filePath, logDate);

    if(logDate!= fileInfo.date){
      return TRUE;
    }
  } else {
    return TRUE;
  }
  return FALSE;
}

static void showHelp(const char *argv0){
  fprintf(stdout,"Usage: %s [options]...\n"
    "Options:\n"
    "  -f          (force)   Force all header files te be re-generated\n"
    "  -h          (help)    Display help\n"
    "  -v          (verbose) Write status information to the screen.\n",
    argv0
  );
}

int main(int argc, char const *argv[]) {

  for(int i = 1; i < argc; i++){
    if( argv[i][0] == '-' ){
      switch( argv[i][1] ){
        case 'v' : quiet = FALSE; break;
        case 'f' : f_flag = TRUE; break;
        case 'h': h_flag = TRUE; showHelp(argv[0]); return 1;
        case 'H': h_flag = TRUE; showHelp(argv[0]); return 1;
        default: showHelp(argv[0]); return 1;
      }
    }
  }
  if(f_flag == TRUE) DEBUG_PRINT("\nForcing re-generation\n");
  filesOfDirectory filesInCurrDir = getCFilesInDirectory(PROJECT_DIRECTORY);
  printf("Number of C files in your directory = %ld\n", filesInCurrDir.numberOfFiles);


  for(int i = 0; i < filesInCurrDir.numberOfFiles ; i++){

    DEBUG_PRINT("\n============= %s =============\n", filesInCurrDir.arrayOfFiles[i].name);
    DEBUG_PRINT("date = %ld\n", filesInCurrDir.arrayOfFiles[i].date);
    DEBUG_PRINT("File name without extension = %s\n", filesInCurrDir.arrayOfFiles[i].nameWithoutExtension);
    //int fileNeedsToBeRemake = 1;

    //if(f_flag == FALSE) fileNeedsToBeRemake = (hasItBeenModified(filesInCurrDir.arrayOfFiles[i]));

    if(TRUE/*f_flag == TRUE || fileNeedsToBeRemake*/){
      if(f_flag == FALSE) {
        DEBUG_PRINT(ANSI_COLOR_YELLOW"File has been modified, it is going to be remake.\n"ANSI_COLOR_RESET);
      }
      else DEBUG_PRINT(ANSI_COLOR_YELLOW"Forcing re-generation.\n"ANSI_COLOR_RESET);
      deleteHeaderFromFile(filesInCurrDir.arrayOfFiles[i].nameWithoutExtension);
      generateH(filesInCurrDir.arrayOfFiles[i].name);
      saveDate(filesInCurrDir.arrayOfFiles[i]);
    } else {
      DEBUG_PRINT(ANSI_COLOR_GREEN"File hasn't been modified.\n"ANSI_COLOR_RESET);
    }
  }
  putchar('\n');
  printf(ANSI_COLOR_GREEN"\t====Done ! The makefile is ready !====\n"ANSI_COLOR_RESET);
  /*if(system("make") == -1){
    perror("Couldn't spawn shell to make");
    exit(errno);
  }*/
  return 0;
}
