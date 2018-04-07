#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>



#include <sys/types.h>
#include <fcntl.h>

#define KGRN  "\x1B[32m" //Green color output
#define KNRM  "\x1B[0m"  //Normal color output
#define KRED  "\x1B[31m" //Red color output

#define TRUE 1
#define FALSE 0
#define DEFAULT_NUMBER_FILES_IN_DIR 10

#define FOLDER_CONFIG_NAME ".mkfileGen"
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


void createHeaders(char folderPath[], char* name){
  memset(folderPath, 0x0, BUFF_SIZE);
  strcat(folderPath, "./scr.sh ");
  strcat(folderPath, name);
  printf("Making headers of %s\n", folderPath);
  if(system(folderPath) == -1){
    perror("Couldn't spawn shell to create header files");
    exit(errno);
  }
}

int main(int argc, char const *argv[]) {

  filesOfDirectory filesInCurrDir = getCFilesInDirectory(PROJECT_DIRECTORY);
  //printf("name %s\n", filesInCurrDir.arrayOfFiles[0].name);
  printf("Number of C files in your directory = %ld\n", filesInCurrDir.numberOfFiles);
  //printMD5(filesInCurrDir.arrayOfFiles[0].MD5Sum);
  char folderPath[BUFF_SIZE] ;

  for(int i = 0; i < filesInCurrDir.numberOfFiles ; i++){
    DEBUG_PRINT("\n============= %s =============\n", filesInCurrDir.arrayOfFiles[i].name);
    DEBUG_PRINT("date = %ld\n", filesInCurrDir.arrayOfFiles[i].date);

    int fileNeedsToBeRemake = (hasItBeenModified(filesInCurrDir.arrayOfFiles[i]));
    if(fileNeedsToBeRemake){
      DEBUG_PRINT(KRED"File has been modified, it is going to be remake.\n"KNRM);
      createHeaders(folderPath, filesInCurrDir.arrayOfFiles[i].name);
      saveDate(filesInCurrDir.arrayOfFiles[i]);
    } else {
      DEBUG_PRINT(KGRN"File hasn't been modified.\n"KNRM);
    }
  }
  putchar('\n');
  if(system("make") == -1){
    perror("Couldn't spawn shell to make");
    exit(errno);
  }
  return 0;
}


/*Returns boolean to says if you need to remake*/
/*int checkMD5Sum(char* fileName){
  int needToRemake = FALSE;
  char folderPath[2048] = FOLDER_CONFIG_NAME;
  struct stat sb;
  int folderDoesntExists = (stat(folderPath, &sb) == -1);
  if(folderDoesntExists) {
    mkdir(folderPath, 0700); //Creating dir
    needToRemake = TRUE;
  }
  int fd;
  char* filePath = strcat(strcat(folderPath, "/"), fileName);
  if(isFileExisting(filePath)){
    fd = open(filePath, O_RDONLY, 0655);
    if(fd == -1){
      perror("Couldn't open file");
      printf("%s\nexiting...\n", filePath);
      exit(1);
    }
    char cachedMD5Sum[MD5_DIGEST_LENGTH] = {0};
    int MD5FileCharCount = 0;
    MD5FileCharCount = read(fd, cachedMD5Sum, MD5_DIGEST_LENGTH);
    if(MD5FileCharCount == -1){
      perror("error reading a C file");
      printf("File path is %s\n", filePath);
      exit(errno);
    }
    else if(MD5FileCharCount != MD5_DIGEST_LENGTH){
      printf("%s isn't an MD5 log\n", filePath);
      needToRemake = TRUE;
      close(fd);
    }
    else {
      printf("Checking MD5 for file %s and %s \tFile's MD5 is = %s \t", fileName, filePath, cachedMD5Sum);
      if(!strncmp(cachedMD5Sum, (char*)getMD5OfFile(fileName), MD5_DIGEST_LENGTH)){
        needToRemake = TRUE;
        printf("MD5SUMS are differents ! Need to remake file %s\n", filePath);
      } else {
        printf("No need to remake file %s\n", filePath);
      }
      close(fd);
    }
  } else {
    fd = open(folderPath, O_CREAT|O_TRUNC, 0666);
    needToRemake = TRUE;
    printf("Need to create file %s\n", filePath);
    close(fd);
  }
  printf("needs to be remake for %s is %d\n", fileName, needToRemake);

  if(!needToRemake) return FALSE;

  return TRUE; //Functiion returns if the MD5 cmp is correct
}*/

/*int saveMD5ForLog(fileInformations* fileInfo){
  int fd = open(fileInfo->name, O_CREAT|O_TRUNC|O_RDWR, 0655);
  if(fd == -1){
    printf("Couldn't open logs for %s\n", fileInfo->name);
    return -1;
  }
  if(write(fd, fileInfoMD5Sum, sizeof(fileInfo->MD5Sum)) == -1){
    printf("Couldn't save logs for %s\n", fileInfo);
    return -1;
  }
  return 0;
}*/



/*unsigned char* getMD5OfFile(const char* filepath){
  unsigned char* c = (unsigned char*) malloc(MD5_DIGEST_LENGTH);
  printf("\n\n\t\tCALCULATING MD5 SUM FOR FILE %s\n\t\t", filepath);
  int i;
  FILE *inFile = fopen (filepath, "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];
  if (inFile == NULL) {
      printf ("%s can't be opened to check MD5.\n", filepath);
      exit(1);
  }
  MD5_Init (&mdContext);
  while ((bytes = fread (data, 1, 1024, inFile)) != 0)
      MD5_Update (&mdContext, data, bytes);
  MD5_Final (c,&mdContext);
  for(i = 0; i < MD5_DIGEST_LENGTH; i++){
    printf("%02x", c[i]);
  }
  printf (" %s\n\n", filepath);
  fclose (inFile);
  return c;
}*/

/*
void printMD5(unsigned char* MD5){
  for(int i = 0; i < MD5_DIGEST_LENGTH; i++){
    printf("%02x", MD5[i]);
  }
  putchar('\n');
}
*/
