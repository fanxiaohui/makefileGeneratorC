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


/*Returns a struct of filesOfDirectory, for current dir */
filesOfDirectory getCFilesInDirectory(const char* dirpath){
  DIR *d;
  struct dirent *dir;
  long numberOfFiles = 0;
  size_t defaultSizeAllocatedForArrayOfFiles = sizeof(fileInformations) * DEFAULT_NUMBER_FILES_IN_DIR;
  filesOfDirectory files = { NULL , numberOfFiles, dirpath};
  memset(&files.arrayOfFiles, '\0', sizeof(files.arrayOfFiles));
  files.arrayOfFiles = malloc(defaultSizeAllocatedForArrayOfFiles);
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
        files.arrayOfFiles[numberOfFiles].name = (char *)currFile;

        files.arrayOfFiles[numberOfFiles].nameWithoutExtension = malloc(lengthOfFileName - 2);

        memcpy(files.arrayOfFiles[numberOfFiles].nameWithoutExtension , files.arrayOfFiles[numberOfFiles].name, lengthOfFileName - 2);
        files.arrayOfFiles[numberOfFiles].date = getDateFile(currFile);

        numberOfFiles++;
        if(numberOfFiles % DEFAULT_NUMBER_FILES_IN_DIR == 0){
          files.arrayOfFiles = realloc(files.arrayOfFiles,
            (sizeof(fileInformations) * numberOfFiles)
            + defaultSizeAllocatedForArrayOfFiles);
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

  char* headerFileName = malloc(lengthOfFileName);
  memset(headerFileName, 0x0, lengthOfFileName);
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
    if(read(fd, &logDate, sizeof(logDate)) == -1) { printf("Err reading log date %s\n", filePath); exit(errno);}
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
    int fileNeedsToBeRemake = 1;
    if(f_flag == FALSE) fileNeedsToBeRemake = (hasItBeenModified(filesInCurrDir.arrayOfFiles[i]));

    if(fileNeedsToBeRemake){
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
