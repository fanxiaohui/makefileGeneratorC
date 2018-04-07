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

#define KGRN  "\x1B[32m" //Green color output
#define KNRM  "\x1B[0m"  //Normal color output
#define KRED  "\x1B[31m" //Red color output

#define TRUE 1
#define FALSE 0
#define DEFAULT_NUMBER_FILES_IN_DIR 10

#define FOLDER_CONFIG_NAME ".mkgenlogs"
#define PROJECT_DIRECTORY "."
#define MAX_FOLDER_PATH_SIZE 2048

#define BUFF_SIZE 2048

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT printf
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
  char* headerFileName = malloc(lengthOfFileName + 2);
  memcpy(headerFileName , filePath, lengthOfFileName + 2);
  strcat(headerFileName, ".h");
  unlink(headerFileName);
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


int main(int argc, char const *argv[]) {
  filesOfDirectory filesInCurrDir = getCFilesInDirectory(PROJECT_DIRECTORY);
  //printf("name %s\n", filesInCurrDir.arrayOfFiles[0].name);
  printf("Number of C files in your directory = %ld\n", filesInCurrDir.numberOfFiles);
  //printMD5(filesInCurrDir.arrayOfFiles[0].MD5Sum);

  for(int i = 0; i < filesInCurrDir.numberOfFiles ; i++){
    DEBUG_PRINT("\n============= %s =============\n", filesInCurrDir.arrayOfFiles[i].name);
    DEBUG_PRINT("date = %ld\n", filesInCurrDir.arrayOfFiles[i].date);

    int fileNeedsToBeRemake = (hasItBeenModified(filesInCurrDir.arrayOfFiles[i]));
    if(fileNeedsToBeRemake){
      DEBUG_PRINT(KRED"File has been modified, it is going to be remake.\n"KNRM);
      deleteHeaderFromFile(filesInCurrDir.arrayOfFiles[i].nameWithoutExtension);
      createHFileFromC(filesInCurrDir.arrayOfFiles[i].name);
      saveDate(filesInCurrDir.arrayOfFiles[i]);
    } else {
      DEBUG_PRINT(KGRN"File hasn't been modified.\n"KNRM);
    }
  }
  putchar('\n');
  printf(KGRN"\t====Done ! The makefile is ready !====\n"KNRM);
  /*if(system("make") == -1){
    perror("Couldn't spawn shell to make");
    exit(errno);
  }*/
  return 0;
}
